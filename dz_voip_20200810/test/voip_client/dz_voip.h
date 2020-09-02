/**************************************************************
 >	Name 	: dz_voip.h
 >  Desc.	: Dangzhi VOIP
 >  Author	: Rick
 >	Date	: 2020/07/01
 *************************************************************/
#include <unistd.h>
#include <sys/timeb.h>
#include <alsa/asoundlib.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

#define DEFAULT_FORMAT	SND_PCM_FORMAT_S16_LE
#define DEFAULT_CHANALS 2
#define DEFAULT_RATE 	48000
#define DEFAULT_PORT	8000

enum {
	PLAYBACK = 0,
	CAPTURE
};
static snd_pcm_stream_t stream  = SND_PCM_STREAM_PLAYBACK;
static snd_pcm_format_t format  = DEFAULT_FORMAT;
static snd_pcm_uframes_t frames = 32;
static unsigned int channels	= DEFAULT_CHANALS;
static unsigned int rate		= DEFAULT_RATE;
static unsigned int direct		= PLAYBACK;

//audio_buff = (char *)malloc(frames * channels * 2); /* size: 2 bytes/sample, 2 channels */

// set hardware parameters
void dz_set_params(snd_pcm_t **handle, int direct);

