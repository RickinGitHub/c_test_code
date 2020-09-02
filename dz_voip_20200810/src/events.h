/*************************************************************************** 
  > File Name: control.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#ifndef __CTRL_STREAM__
#define __CTRL_STREAM__

/*
#define MSG_MAX 				128
typedef struct {
	char head[12];
	unsigned char func;
	unsigned char opt;
	int len[2];
	char buff[MSG_MAX];
} MsgHead,*pMsgHead;

//unsigned char *head = "DangzhiVoIP";

MsgHead msg_head;
*/

extern void ctrl_events_handle(void);
extern void voice_call_answer(void);
extern void voice_call_handup(void);

#endif
