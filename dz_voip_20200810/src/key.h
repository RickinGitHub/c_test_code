/*************************************************************************** 
  > File Name: key.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/ 
#ifndef __KEY_EVENTS__
#define __KEY_EVENTS__

extern void *dz_key_events(void *arg);
extern void set_phone_state(int state);
extern int  get_phone_state(void);
extern void set_mic_state(void);
extern int  get_mic_state(void);
#endif