/**************************************************************
 >	Name 	: dz_voip.h
 >  Desc.	: Dangzhi VOIP
 >  Author	: Rick
 >	Date	: 2020/07/01
 *************************************************************/
#include <sys/timeb.h>
#include <alsa/asoundlib.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define DEFAULT_FORMAT	SND_PCM_FORMAT_S16_LE
#define DEFAULT_CHANALS 
#define DEFAULT_RATE 	16000
#define DEFAULT_PORT	8000

#define DEF_CAPTURE_RATE 	48000
#define DEF_PLAYBACK_RATE 	16000

enum {
	PLAYBACK = 0,
	CAPTURE
};

typedef struct {
	char name[20];
	snd_pcm_t *handle;
} PcmHandle;
