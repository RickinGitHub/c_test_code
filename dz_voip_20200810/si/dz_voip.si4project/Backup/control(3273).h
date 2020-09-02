/*************************************************************************** 
  > File Name: control.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#ifndef __CTRL_STREAM__
#define __CTRL_STREAM__

#define RMT_CALL_IN_REQ			0x0101
#define RMT_HANDUP_REQ_SYNC		0x0102
#define MSR_CALL_IN_REQ_TRANS	0x0103
#define MSR_ANSWER_SYNC			0x0104
#define MSR_HANDUP_SYNC			0x0105
#define SLV_ANSWER_SYNC			0x0106
#define SLV_HANDUP_SYNC			0x0107
#define MSR_CALL_OUT_REQ_SYNC	0x0108
#define RMT_ANSWER_SYNC			0x0109

#define MSG_MAX 				128
typedef struct {
	char head[12];
	unsigned char func;
	unsigned char opt;
	int len[2];
	char buff[MSG_MAX];
} MsgHead,*pMsgHead;

MsgHead msg_head;

void *control_stream_handle(void *arg);

#endif
