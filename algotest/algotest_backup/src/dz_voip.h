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
#define DEFAULT_RATE 	16000
#define DEF_CAPTURE_RATE 	16000
#define DEF_PLAYBACK_RATE 	48000
#define DEF_CAPTURE_CHANALS 	6
#define DEF_PLAYBACK_CHANALS 	2

enum {
	PLAYBACK = 0,
	CAPTURE
};

typedef struct {
	char name[20];
	snd_pcm_t *handle;
} PcmHandle;

extern pthread_mutex_t mutex_audio,mutex_audio_aec;

void *audio_send_handle(void *arg);
void *audio_recv_handle(void *arg);

#endif
