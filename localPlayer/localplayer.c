#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <alsa/asoundlib.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

#define DEFAULT_FORMAT	SND_PCM_FORMAT_S16_LE
#define DEFAULT_CHANALS 6
#define DEFAULT_RATE 	16000
#define DEFAULT_PORT	8000

#define DEF_CAPTURE_RATE 	16000
#define DEF_PLAYBACK_RATE 	48000

enum {
	PLAYBACK = 0,
	CAPTURE
};

typedef struct {
	char name[20];
	snd_pcm_t *handle;
} PcmHandle;

static snd_pcm_stream_t stream    = SND_PCM_STREAM_PLAYBACK;
static snd_pcm_format_t format    = DEFAULT_FORMAT;
static snd_pcm_uframes_t frames   = 32;
static unsigned int channels	  = DEFAULT_CHANALS;
static unsigned int rate  		  = DEFAULT_RATE;
static unsigned int direct		  = PLAYBACK;

PcmHandle pcm_capture_handle, pcm_playback_handle;

void dz_set_params(snd_pcm_t **handle, int direct, int rate);

#if 0
int main(int argc, char *argv[])
{
	FILE *readi = NULL;
	int rc,size;
	unsigned char * send_buf = NULL;
	static snd_pcm_t *handlec,*handlep;

	dz_set_params(&handlec, CAPTURE, DEFAULT_RATE);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");

	size = frames * channels * 2; /* 2 bytes/sample, 2 channels */
	send_buf = (char *)malloc(size);
	memset(send_buf, 0, size);

	readi = fopen("readi_dump.wav", "a+");
	if (readi ==NULL) {
		printf("open readi_dump.pcm failed\n");
	}else {
		printf("open readi ok\n");
	}

	while(1){
		//pthread_mutex_lock(&mutex_audio);
	    rc = snd_pcm_readi(pcm_capture_handle.handle, send_buf, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(pcm_capture_handle.handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
		//pthread_mutex_unlock(&mutex_audio);
		fwrite(send_buf, 1, size, readi);
	}

	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
	fclose(readi);

	return 0;
}
#endif

int main(int argc, char *argv[])
{
	FILE *fr = NULL;
	int rc,size;
	unsigned char * send_buf = NULL;
	static snd_pcm_t *handlep;
	channels = 6;

	if(argc <= 1) {
		printf("Usage: local_player xxx.wav\n");
		return 0;
	}

	dz_set_params(&handlep, PLAYBACK, DEFAULT_RATE);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");

	size = frames * channels * 2; /* 2 bytes/sample, 2 channels */
	send_buf = (char *)malloc(size);
	memset(send_buf, 0, size);

	fr = fopen(argv[1], "r");
	if (fr ==NULL) {
		printf("open file failed\n");
	}else {
		printf("open file ok\n");
	}

	while(1){
		//pthread_mutex_lock(&mutex_audio);
	    rc = snd_pcm_readi(pcm_capture_handle.handle, send_buf, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(pcm_capture_handle.handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
		//pthread_mutex_unlock(&mutex_audio);
		fwrite(send_buf, 1, size, fr);
	}

	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
	fclose(readi);

	return 0;
}



void dz_set_params(snd_pcm_t **handle, int direct, int rate)
{
	int rc;
	int dir=0;
	snd_pcm_stream_t stream;
	snd_pcm_format_t format    = DEFAULT_FORMAT;
	snd_pcm_t* phandle;
	snd_pcm_hw_params_t* params;

	if(direct == PLAYBACK) {
		stream = SND_PCM_STREAM_PLAYBACK;
		printf("------>stream:%d,SND_PCM_STREAM_PLAYBACK......\n",stream);
	}
	if(direct == CAPTURE) {
		stream   = SND_PCM_STREAM_CAPTURE;
		printf("------>stream:%d,SND_PCM_STREAM_CAPTURE......\n",stream);
	}
	
	rc=snd_pcm_open(&phandle, "mult_4_2", stream, 0);
	if(rc<0)
	{
	  perror("\nopen PCM device failed:");
	  exit(1);
	}
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_alloca:");
	  exit(1);
	}
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
	/* Set period size to 32 frames. */
	snd_pcm_hw_params_set_period_size_near(phandle,params, &frames, &dir);
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
	printf("frames:%d\n",(int)frames);
	printf("format:%d\n",(int)format);
	printf("chanel:%d\n",(int)channels);
	printf("rate  :%d\n",(int)rate);
	printf("-----------------------------------------------\n");

}
