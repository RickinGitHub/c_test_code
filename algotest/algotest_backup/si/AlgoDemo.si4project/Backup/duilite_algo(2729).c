#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "duilite_algo.h"
#include "dz_voip.h"

char *auth_cfg = "{\"auth_dev_addr\": 2}";  // I2C addr
struct duilite_fespa *fespa;

/*
 * 唤醒回调，唤醒时触发，返回json 字符串，字段释义同2.6 节语音唤醒结果：
 *	{
 		"wakeupWord": "xxx",
 		"major": 1,
 		"status": 1,
 		"confidence": 0.465926
 *	}
 */
int wakeup_callback(void *user_data, int type, char *msg, int len) 
{
    printf("%.*s\n", len, msg);
    return 0;
}

/*
 * 在唤醒回调之后，返回唤醒角度信息，为json字符串，格式如下：
 *		{"doa": 95}
 */
int doa_callback(void *user_data, int type, char *msg, int len) 
{
    printf("%.*s\n", len, msg);
    return 0;
}

/*
	音频回调，实时输出经过信号处理后的音频，通常用于识别；
	当唤醒时，会在唤醒结果与角度回调之后，返回一个json字符串：
		{"wakeup_type": 1}
	或
		{"wakeup_type": 2}
	其中1 表示，此唤醒词在env中配置major=1，需要回滚音频，在收到该json串后，回滚音频开始;返回；2表示不需要回滚。
*/
int beamforming_callback(void *user_data, int type, char *msg, int len) 
{
	//FILE* output = (FILE*)user_data;
	
	if (type == DUILITE_MSG_TYPE_JSON) {
		printf("-------->,DUILITE_MSG_TYPE_JSON%.*s", len, msg);
	} else if (type == DUILITE_MSG_TYPE_BINARY) {
		printf("-------->DUILITE_MSG_TYPE_BINARY,%d\n", len);
		if(len > 0) {
			printf("-------->memcpy\n");
			memcpy(send_buf, msg, len);
		    fwrite(msg, 1, len, output_bf_audio);
		}
	}
    return 0;

}

/*
 * 包含回声消除（echo），声源定位（doa），波束成形（beamforming）和语音唤醒（wakeup）等功能模块
 *
 */
int duilite_init(FILE** output)
{
	char *cfg = 
            "{ \
                \"aecBinPath\":\"../res/fesp/AEC_ch8-2-ch4_2ref_emd_20200811_v2.50.0.10.bin\",    \
                \"wakeupBinPath\":\"../res/fesp/wakeup_aifar_comm_20180104.bin\", \
                \"beamformingBinPath\":\"../res/fesp/UCA_asr_ch4-2-ch4_70mm_20200616_v2.0.0.40_wkpMTModeOff.bin\",    \
                \"env\":\"words=ni hao xiao le;thresh=0.13\",  \
                \"rollBack\":1200     \
            }";
    int r = duilite_library_load(auth_cfg);//加载库函数，完成初始化与授权，接口阻塞，调用一次即可
    printf("duilite_library_load :%d\n", r);
    
    fespa = duilite_fespa_new(cfg);//初始化fespa引擎，并在后台开启wakeup，echo和beamforming等计算线程
    assert(fespa != NULL);

    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_WAKEUP, wakeup_callback, NULL);

    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_DOA, doa_callback, NULL);

    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_BEAMFORMING, beamforming_callback, NULL);

}

void duilite_exit(void)
{
	duilite_fespa_delete(fespa);//销毁fespa引擎
    duilite_library_release();//释放库资源
}

void duilite_feed(struct duilite_fespa *fespa, char *data, int len)
{			 
	duilite_fespa_feed(fespa, data, len);//输入音频数据,格式为16k采样率，有符号16bit编码
	usleep(32000);
}



