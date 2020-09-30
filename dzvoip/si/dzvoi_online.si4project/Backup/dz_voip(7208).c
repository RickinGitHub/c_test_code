/*************************************************************************** 
  > File Name: dz_voip.c 
  > Des      : capture & playback by 2.4GHz
  > Author   : Rick
  > Date     : 2020/07/10
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/syscall.h>
#include <sys/select.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "../include/duilite.h"
#include "dz_voip.h"

#define DEBUG_TEST 1

char *auth_cfg = "{\"auth_dev_addr\": 2}";  // I2C addr
struct duilite_fespa *fespa;

unsigned char *spk_playback_buf=NULL;
snd_pcm_t *rk_capture_handle, *xy_playback_handle;
FILE* output_bf_audio = NULL;

int  wakeup_callback(void *user_data, int type, char *msg, int len);
int  doa_callback(void *user_data, int type, char *msg, int len);
int  beamforming_callback(void *user_data, int type, char *msg, int len);
int  duilite_init(void);
void duilite_exit(void);
void duilite_feed(struct duilite_fespa *fespa, char *data, int len);

void *voice_uplink(void *arg);
void *voice_downlink(void *arg);

/*
 * dz_set_params()
 * int   direct : playback or capture
 * char* devname: "default", "hw:0,0","hw:1,0", "mult_4_2" or other names
 * int rate : sample rate
 */
snd_pcm_uframes_t dz_set_params(snd_pcm_t **handle, int direct, char *devname, int rate, int channels)
{
	int rc, dir=0;
	snd_pcm_stream_t stream;
	snd_pcm_format_t format    = SND_PCM_FORMAT_S16_LE;
	snd_pcm_t* phandle;
	snd_pcm_hw_params_t* params;
	snd_pcm_uframes_t frames;

	if(direct == PLAYBACK) {
		stream = SND_PCM_STREAM_PLAYBACK;
		printf("-------->dev:%s, SND_PCM_STREAM_PLAYBACK\n",devname, stream);
	}
	if(direct == CAPTURE) {
		stream	 = SND_PCM_STREAM_CAPTURE;
		printf("-------->dev:%s, SND_PCM_STREAM_CAPTURE\n",devname, stream);

	}

	/* open pcm device */
	rc=snd_pcm_open(&phandle, devname, stream, 0);
	if(rc<0)
	{
		perror("\nopen PCM device failed:");
		exit(1);
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	rc=snd_pcm_hw_params_any(phandle, params);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_any:");
	  exit(1);
	}
	/* Interleaved mode */
	rc=snd_pcm_hw_params_set_access(phandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(rc<0)
	{
	  perror("\nsed_pcm_hw_set_access:");
	  exit(1);

	}
	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(phandle, params, format);

	/* Two channels (stereo) */
	rc=snd_pcm_hw_params_set_channels(phandle, params, channels);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_set_channels:");
	  exit(1);
	}
	/* 44100 bits/second sampling rate (CD quality) */
	rc=snd_pcm_hw_params_set_rate_near(phandle, params, &rate, &dir); 
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_set_rate_near:");
	  exit(1);
	}
	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(phandle, params);
	if(rc<0)
	{
		perror("\nsnd_pcm_hw_params: ");
		exit(1);
	}
	/* Use a buffer large enough to hold one period */
	rc=snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_get_period_size\n");
	  exit(1);
	}

	//把句柄传出去
	*handle = phandle;
	printf("--------[ Hardware Parameters Info. ]----------\n");
	printf("stream:%d\n",(int)stream);
	printf("devname:%s\n", devname);
	printf("frames:%d\n",(int)frames);
	printf("format:%d\n",(int)format);
	printf("chanel:%d\n",(int)channels);
	printf("rate  :%d\n",(int)rate);
	printf("-----------------------------------------------\n");

	return frames;
}

void *voice_downlink(void *arg)
{
	int size, rc;
	snd_pcm_t         *rk_playback_handle, *xy_capture_handle; 
	snd_pcm_uframes_t rk_playback_frames, xy_capture_frames;

	unsigned char *xy_catpure_buf=NULL;

	xy_capture_frames  = dz_set_params(&xy_capture_handle,  CAPTURE, "hw:1,0",  48000, 2);
	rk_playback_frames = dz_set_params(&rk_playback_handle, PLAYBACK,"default", 48000, 2);
	 
	printf("------>voice_downlink\n");
	
	size = xy_capture_frames* 2 * 2;
	xy_catpure_buf = (char *)malloc(size);
	memset(xy_catpure_buf, 0, size);	
	printf("xiangying capture size:%d frames:%d, rk_playback frames:%d\n",size,xy_capture_frames, rk_playback_frames);
    
	while(1) {
		rc = snd_pcm_readi(xy_capture_handle, xy_catpure_buf, xy_capture_frames); //256->frame frame=2*2 bytes
		if(rc>0)
		{
			rc = snd_pcm_writei(rk_playback_handle, xy_catpure_buf, xy_capture_frames); //256->frame frame=2*2 bytes
			if (rc == -EPIPE) {
				snd_pcm_prepare(rk_playback_handle);
			 	snd_pcm_writei(rk_playback_handle, xy_catpure_buf, xy_capture_frames);
			}
		}
		memset(xy_catpure_buf, 0, size);
    }
	snd_pcm_drain(rk_playback_handle);
	snd_pcm_close(rk_playback_handle);
	snd_pcm_drain(xy_capture_handle);
	snd_pcm_close(xy_capture_handle);
	free(xy_catpure_buf);
    
    return  NULL;
}

char *buf_48k2ch = NULL;
int bufLen_48k2ch = 512*6;
FILE* f_48k2ch_dump;

void *voice_uplink(void *arg)
{
	int size, rc;
	FILE* fr;
	char *rawbuf6Mic = NULL;
	//snd_pcm_t         *rk_capture_handle, *xy_playback_handle;
	snd_pcm_uframes_t rk_capture_frames, xy_playback_frames;	

	printf("------>voice_uplink\n");

	rk_capture_frames  = dz_set_params(&rk_capture_handle, CAPTURE, "mult_4_2", 16000, 6);
	xy_playback_frames = dz_set_params(&xy_playback_handle, PLAYBACK, "hw:1,0", 48000, 2);

	printf("xiangyin playback:frames:%d, rk capture frames:%d...\n",xy_playback_frames, rk_capture_frames);

	buf_48k2ch = (char *)malloc(bufLen_48k2ch);
	memset(buf_48k2ch, 0,bufLen_48k2ch);
	size = rk_capture_frames * 6 * 2; /* 2 bytes/sample, 2 channels */
	rawbuf6Mic = (char *)malloc(size);
	memset(rawbuf6Mic, 0, size);

#if DEBUG_TEST
	fr = fopen("6mic.raw", "a+");
	if (fr ==NULL) {
		printf("open 6mic.raw failed\n");
	}
	f_48k2ch_dump = fopen("48k2ch.raw", "a+");
	if (f_48k2ch_dump ==NULL) {
		printf("open 48k2ch.raw failed\n");
	}
#endif

	duilite_init();

    while (1)
    {
		
	    rc = snd_pcm_readi(rk_capture_handle, rawbuf6Mic, rk_capture_frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(rk_capture_handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)rk_capture_frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
		

		//dump raw data
#if DEBUG_TEST
		fwrite(rawbuf6Mic, 1, size, fr);
#endif	
		duilite_feed(fespa, rawbuf6Mic, size);
		memset(rawbuf6Mic, 0, size);
    }

	duilite_exit();
	
	snd_pcm_drain(rk_capture_handle);
	snd_pcm_close(rk_capture_handle);
	snd_pcm_drain(xy_playback_handle);
	snd_pcm_close(xy_playback_handle);

	free(rawbuf6Mic);

#if DEBUG_TEST
	fclose(fr);
#endif

    return  NULL;
}

int wakeup_callback(void *user_data, int type, char *msg, int len) 
{
    printf("%.*s\n", len, msg);
    return 0;
}

int doa_callback(void *user_data, int type, char *msg, int len) 
{
    printf("%.*s\n", len, msg);
    return 0;
}

void pcm_16k1chto48k2ch(const char *input, int len, char *output)
{
	for(int i = 0; i < len; i+=2) { 
		memcpy(output + i*6+0, input+i, 2);
		memcpy(output + i*6+2, input+i, 2);
		memcpy(output + i*6+4, input+i, 2);
		memcpy(output + i*6+6, input+i, 2);
		memcpy(output + i*6+8, input+i, 2);
		memcpy(output + i*6+10, input+i, 2);
	}
}

int beamforming_callback(void *user_data, int type, char *msg, int len) 
{
	if (type == DUILITE_MSG_TYPE_JSON) {
		printf("-------->,DUILITE_MSG_TYPE_JSON%.*s", len, msg);
	} else if (type == DUILITE_MSG_TYPE_BINARY) {
		if(len > 0) {
#if DEBUG_TEST
		    fwrite(msg, 1, len, output_bf_audio);
#endif
			if(512==len)
			{	
				memset(buf_48k2ch, 0,bufLen_48k2ch);
				pcm_16k1chto48k2ch(msg,len,buf_48k2ch); //This will make the size of data to 6 times
#if DEBUG_TEST
				fwrite(buf_48k2ch, 1, bufLen_48k2ch, f_48k2ch_dump);
#endif
			    snd_pcm_uframes_t frames = ( len / 2 ) * 3; // len = frames * channels * 2,  2 bytes/sample
				int rc2 = snd_pcm_writei(xy_playback_handle, buf_48k2ch, frames);
				if (rc2 == -EPIPE) {
					snd_pcm_prepare(xy_playback_handle);
					snd_pcm_writei(xy_playback_handle, buf_48k2ch, frames);
				}
			}
		}
	}
    return 0;
}

int duilite_init(void)
{
	char *cfg = 
            "{ \
                \"aecBinPath\":\"/userdata/res/fesp/AEC_ch6-2-ch4_2ref_emd_20200508_v2.50.0.10.bin\",    \
				\"wakeupBinPath\":\"/userdata/res/fesp/wakeup_aifar_comm_20180104.bin\", \
				\"beamformingBinPath\":\"/userdata/res/fesp/UCA_asr_ch4-2-ch4_70mm_20200616_v2.0.0.40_wkpMTModeOff.bin\",    \
				\"env\":\"words=ni hao xiao le;thresh=0.13\",  \
                \"rollBack\":1200     \
            }";
    int r = duilite_library_load(auth_cfg);
    printf("duilite_library_load :%d\n", r);
    
    fespa = duilite_fespa_new(cfg);
    assert(fespa != NULL);

    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_WAKEUP, wakeup_callback, NULL);
    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_DOA, doa_callback, NULL);
    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_BEAMFORMING, beamforming_callback, NULL);
#if DEBUG_TEST
	output_bf_audio = fopen("aispeechdump.raw", "a+");
	if (output_bf_audio ==NULL) {
		printf("open srv_send.raw failed\n");
	}
#endif
}

void duilite_exit(void)
{
	duilite_fespa_delete(fespa);
    duilite_library_release();
#if DEBUG_TEST
    fclose(output_bf_audio);
	fclose(f_48k2ch_dump);
#endif
	free(buf_48k2ch);
	
}

void duilite_feed(struct duilite_fespa *fespa, char *data, int len)
{			 
	duilite_fespa_feed(fespa, data, len);//输入音频数据,格式为16k采样率，有符号16bit编码
}

#if 0
int main(int argc, char *argv[])
{
	int server_status = 0;
	while(1) {
		if(server_status == 0) {
		pthread_t thread_uplink,thread_downlink;
			
		pthread_create(&thread_uplink, NULL, &voice_uplink, NULL);
		pthread_detach(thread_uplink);
		pthread_create(&thread_downlink, NULL, &voice_downlink, NULL);
		pthread_detach(thread_downlink);
		printf("audio handle has estabished!\n");	

		server_status = 1;
		}
		usleep(320000);
	}
	
	return 0;
}
#endif
