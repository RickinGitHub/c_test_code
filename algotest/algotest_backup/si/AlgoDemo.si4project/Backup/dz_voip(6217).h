/*************************************************************************** 
  > File Name: dz_voip.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
****************************************************************************/
#ifndef __DZ_VOIP__
#define __DZ_VOIP__
#include <sys/timeb.h>
#include <alsa/asoundlib.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

#define TCP_SEND_PORT		10121
#define TCP_RECV_PORT		10122

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

#if 1
//static snd_pcm_stream_t stream    = SND_PCM_STREAM_PLAYBACK;
//static snd_pcm_format_t format    = DEFAULT_FORMAT;
static snd_pcm_uframes_t frames   = 32;
static unsigned int channels	  = DEFAULT_CHANALS;
//static unsigned int rate  		  = 16000;
//static unsigned int direct		  = PLAYBACK;
#endif

extern PcmHandle pcm_capture_handle, pcm_playback_handle;
extern FILE* output_bf_audio;
extern FILE* readi_buff;
extern pthread_mutex_t mutex_audio,mutex_audio_aec;
extern unsigned char *send_buf;

void  dz_set_params(snd_pcm_t **handle, int direct, int rate);
void *audio_send_handle(void *arg);
void *audio_recv_handle(void *arg);

#endif