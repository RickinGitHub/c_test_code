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

enum {
	PLAYBACK = 0,
	CAPTURE
};

extern void *voice_uplink(void *arg);
extern void *voice_downlink(void *arg);

#endif
