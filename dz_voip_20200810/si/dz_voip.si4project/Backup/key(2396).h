/*************************************************************************** 
  > File Name: key.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/ 
#ifndef __KEY_EVENTS__
#define __KEY_EVENTS__

#define ENABLE			1
#define DISABLE			0

typedef enum {
	PHONE_STATE_NORMAL    = 0,
	PHONE_STATE_WAIT      = 1,
	PHONE_STATE_ACTIVATED = 2
}phone_state_t;

extern phone_state_t phone_state;
extern void dz_key_events(void);
extern void set_phone_state(int state);
extern int  get_phone_state(void);
extern void set_mic_enable(int enable);
extern int  get_mic_state(void);
#endif