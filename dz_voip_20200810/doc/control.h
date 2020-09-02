/*************************************************************************** 
  > File Name: control.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#ifndef __CTRL_STREAM__
#define __CTRL_STREAM__

#define MSG_MAX 4096

typedef struct {
	int  head;
	int  func;
	int  opt;
	int  len;
	char buff[MSG_MAX];
} MsgHead,*pMsgHead;

typedef enum {
	DIAL_REQUEST = 0,
	DIAL_ACCEPT  = 1,
	DIAL_STATUS  = 2,
	


}OptionCode;

MsgHead msg_head;

void *control_stream_handle(void *arg);

#endif
