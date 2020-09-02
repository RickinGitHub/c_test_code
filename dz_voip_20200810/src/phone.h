/*************************************************************************** 
  > File Name: phone.h
  > Des      : 话机主函数头文件
  > Author   : Rick
  > Date     : 2020/08/03
 ***************************************************************************/ 
#ifndef __PHONE__
#define __PHONE__

#define ENABLE			1
#define DISABLE			0

#define RMT_CALL_IN_REQ			0x01 //远程呼入请求
#define RMT_HANDUP_SYNC		    0x02 //远程挂断同步
#define MSR_ANSWER_SYNC			0x03 //主机接听同步
#define MSR_HANDUP_SYNC			0x04 //主机挂断同步
#define SLV_ANSWER_SYNC			0x05 //话机接听同步
#define SLV_HANDUP_SYNC			0x06 //话机挂断同步

#define TCP_CTRL_PORT	10120
#define TCP_SEND_PORT	10121
#define TCP_RECV_PORT	10122


typedef enum {
	PHONE_STATE_NORMAL    = 0,
	PHONE_STATE_WAIT      = 1,
	PHONE_STATE_ACTIVATED = 2
}phone_state_t;

extern int volume;
extern int micsta;
extern phone_state_t phone_state;
extern pthread_mutex_t mutex_phone_stat;

#endif


