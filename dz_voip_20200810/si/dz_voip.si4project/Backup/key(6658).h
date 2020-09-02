/*************************************************************************** 
  > File Name: key.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/ 
#ifndef __KEY_EVENTS__
#define __KEY_EVENTS__

//#define KEY_VOLUMEDOWN	114
//#define KEY_VOLUMEUP	115
//#define KEY_PHONE		169
//#define KEY_MICMUTE		248

#define ENABLE			1
#define DISABLE			0

void *thread_key_events(void *argv);
extern void set_phone_state(int state);
extern int  get_phone_state(void);
#endif