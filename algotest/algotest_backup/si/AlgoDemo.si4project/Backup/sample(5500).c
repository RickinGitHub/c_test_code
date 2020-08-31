#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "../include/duilite.h"

char *auth_cfg = "{\"auth_dev_addr\": 2}";  // I2C addr

static int wakeup_callback(void *user_data, int type, char *msg, int len) {
    printf("%.*s\n", len, msg);
    return 0;
}

static int doa_callback(void *user_data, int type, char *msg, int len) {
    printf("%.*s\n", len, msg);
    return 0;
}

static int beamforming_callback(void *user_data, int type, char *msg, int len) {
    if(type == DUILITE_MSG_TYPE_JSON) {
        printf("%.*s\n", len, msg);
    }
    return 0;
}

int main(int argc, char **argv)
{
    char *cfg = 
            "{ \
                \"aecBinPath\":\"./res/fesp/AEC_ch6-2-ch4_1ref_emd_20200508_v2.50.0.10.bin\",    \
                \"wakeupBinPath\":\"./res/fesp/wakeup_aifar_comm_20180104.bin\", \
                \"beamformingBinPath\":\"./res/fesp/UCA_asr_ch4-2-ch4_70mm_20200616_v2.0.0.40_wkpMTModeOff.bin\",    \
                \"env\":\"words=ni hao xiao le;thresh=0.13\",  \
                \"rollBack\":1200     \
            }";
    int r = duilite_library_load(auth_cfg);
    printf("duilite_library_load :%d\n", r);
    
    struct duilite_fespa *fespa = duilite_fespa_new(cfg);
    assert(fespa != NULL);
    
    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_WAKEUP, wakeup_callback, NULL);
    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_DOA, doa_callback, NULL);
    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_BEAMFORMING, beamforming_callback, NULL);
    FILE *audio = fopen(argv[1], "r");
    assert(audio != NULL);
    
    fseek(audio, 44, SEEK_SET);
    char buf[3200* 8];
    int len;
    
    while(1)
    {
        len = fread(buf, 1, sizeof(buf), audio);
        if(len == 0)
            break;
        
        duilite_fespa_feed(fespa, buf, len);
        usleep(100000);
    }
    
    fclose(audio);
    duilite_fespa_delete(fespa);
    
    duilite_library_release();
    
    return 0;
}
