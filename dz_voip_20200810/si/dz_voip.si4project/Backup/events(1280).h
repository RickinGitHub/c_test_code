/*************************************************************************** 
  > File Name: control.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#ifndef __CTRL_STREAM__
#define __CTRL_STREAM__

#define RMT_CALL_IN_REQ			0x01 //远程呼入请求
#define RMT_HANDUP_SYNC		    0x02 //远程挂断同步
#define MSR_ANSWER_SYNC			0x03 //主机接听同步
#define MSR_HANDUP_SYNC			0x04 //主机挂断同步
#define SLV_ANSWER_SYNC			0x05 //话机接听同步
#define SLV_HANDUP_SYNC			0x06 //话机挂断同步

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

extern void ctrl_events_handle(void);
extern void voice_call_answer(void);
extern void voice_call_handup(void);

#endif
