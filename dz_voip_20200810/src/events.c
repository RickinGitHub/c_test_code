/*************************************************************************** 
  > File Name: control.c 
  > Des      : 控制流线程,主要用于接收主机拨号/挂断请求、音频参数，发送按键挂断/接听请求发送 
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h> 
#include <sys/select.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/tcp.h>   
#include <netinet/in.h>   
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "phone.h"
#include "events.h"
#include "voip.h"
#include "p2p_connect.h"
#include "tcp_server.h"
#include "key.h"

void voice_call_answer(void);
void voice_call_handup(void);

void ctrl_events_handle(void) 
{
	int cmd;
	unsigned char recvbuff[MSG_MAX] = {0};
	unsigned char sendbuff[MSG_MAX] = {0};

	while(sizeof(recvbuff) > 0) {
		if(tcp_ctrl_events.server_fd > 0){
			server_recv_msg(&tcp_ctrl_events, recvbuff, MSG_MAX, 0);
			cmd = atoi(recvbuff);
			printf("recfbuff:%s,cmd:%d\n",recvbuff,cmd);			

			if(cmd == RMT_CALL_IN_REQ) {
				if(phone_state == PHONE_STATE_NORMAL) {
					pthread_mutex_lock(&mutex_phone_stat);
					set_phone_state(PHONE_STATE_WAIT);
					pthread_mutex_unlock(&mutex_phone_stat);
				}				
				printf("RMT_CALL_IN_REQ,%d\n", phone_state);
			} else if(cmd == RMT_HANDUP_SYNC) {
				if(phone_state == PHONE_STATE_ACTIVATED) {
					voice_call_handup();
					pthread_mutex_lock(&mutex_phone_stat);
					set_phone_state(PHONE_STATE_NORMAL);
				    pthread_mutex_unlock(&mutex_phone_stat);
				}
				printf("RMT_HANDUP_SYNC\n");
				server_send_msg(&tcp_ctrl_events,"RMT_HANDUP_SYNC",30);

			} else if(cmd == MSR_ANSWER_SYNC) {
				if(phone_state == PHONE_STATE_NORMAL) {
					pthread_mutex_lock(&mutex_phone_stat);
					set_phone_state(PHONE_STATE_ACTIVATED);
				    pthread_mutex_unlock(&mutex_phone_stat);
				}
				printf("MSR_ANSWER_SYNC\n");
				server_send_msg(&tcp_ctrl_events,"MSR_ANSWER_SYNC",30);

			} else if(cmd == MSR_HANDUP_SYNC) {
				if(phone_state == PHONE_STATE_ACTIVATED) {
					voice_call_handup();
					pthread_mutex_lock(&mutex_phone_stat);
					set_phone_state(PHONE_STATE_NORMAL);
				    pthread_mutex_unlock(&mutex_phone_stat);
					printf("MSR_HANDUP_SYNC\n");
				}
				server_send_msg(&tcp_ctrl_events,"MSR_HANDUP_SYNC",30);
			}else {
				
			}

			memset(recvbuff, 0 ,MSG_MAX);
			memset(sendbuff, 0 ,MSG_MAX);
		}
	}
}

void voice_call_answer(void)
{
	unsigned char buf[12] = {0};
	if(phone_state == PHONE_STATE_WAIT) {
		pthread_mutex_lock(&mutex_phone_stat);
		set_phone_state(PHONE_STATE_ACTIVATED);
		pthread_mutex_unlock(&mutex_phone_stat);
		
		sprintf(buf, "%d", get_phone_state());
		server_send_msg(&tcp_ctrl_events,buf,strlen(buf));
	}
	printf("phone state:%d\n",get_phone_state());
}

void voice_call_handup(void)
{
	unsigned char buf[12] = {0};
	if(phone_state == PHONE_STATE_ACTIVATED){
		pthread_mutex_lock(&mutex_phone_stat);
		set_phone_state(PHONE_STATE_NORMAL);
		pthread_mutex_unlock(&mutex_phone_stat);
		
		sprintf(buf, "%d", get_phone_state());
		server_send_msg(&tcp_ctrl_events,buf,strlen(buf));
	}
	printf("phone state:%d\n",get_phone_state());
}

