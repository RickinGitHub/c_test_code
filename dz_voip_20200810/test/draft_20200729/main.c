#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "update_server.h"

#define RMT_CALL_IN_REQ			0x01 //远程呼入请求
#define RMT_HANDUP_SYNC		    0x02 //远程挂断同步
#define MSR_ANSWER_SYNC			0x03 //主机接听同步
#define MSR_HANDUP_SYNC			0x04 //主机挂断同步
#define SLV_ANSWER_SYNC			0x05 //话机接听同步
#define SLV_HANDUP_SYNC			0x06 //话机挂断同步

#define MSG_MAX 				128

void ctrl_events_handle(void);
void audio_sndsrv_handle(void);
void audio_recsrv_handle(void);

int main(void)
{
	int ctrl_srv_stat = 0;
	int audio_sndsrv_stat = 0;
	int audio_recsrv_stat = 0;

	while(1) {
		if(ctrl_srv_stat == 0) {
			pthread_t thrd_ctrl_srv;
			tcp_ctrl_events.port = TCP_CTRL_PORT;

			printf("------------before thread-------------\n");
            printf("ctrl_events_handle, tcp_ctrl_events.client_fd = %d\n",tcp_ctrl_events.client_fd);
            printf("ctrl_events_handle, tcp_ctrl_events.server_fd = %d\n",tcp_ctrl_events.server_fd);
            printf("ctrl_events_handle, tcp_ctrl_events.port = %d\n",tcp_ctrl_events.port);
			
			pthread_create(&thrd_ctrl_srv, NULL, &Tcp_Server_Thread, &tcp_ctrl_events);
			pthread_detach(thrd_ctrl_srv);
			ctrl_srv_stat = 1;
		}
#if 1
		if(audio_sndsrv_stat == 0) {
			pthread_t thrd_audio_sndsrv;
			tcp_audio_send.port = TCP_SEND_PORT;
			
			pthread_create(&thrd_audio_sndsrv, NULL, &Tcp_Server_Thread, &tcp_audio_send);
			pthread_detach(thrd_audio_sndsrv);
			audio_sndsrv_stat = 1;
		}

		if(audio_recsrv_stat == 0) {
			pthread_t thrd_audio_recvsrv;
			tcp_audio_recv.port = TCP_RECV_PORT;

			pthread_create(&thrd_audio_recvsrv, NULL, &Tcp_Server_Thread, &tcp_audio_recv);
			pthread_detach(thrd_audio_recvsrv);
			audio_recsrv_stat = 1;

		}
#endif
		if(tcp_ctrl_events.client_fd > 0) {
			ctrl_events_handle();
		}
#if 1
		if(tcp_audio_send.client_fd > 0) {
			audio_sndsrv_handle();
		}

		if(tcp_audio_recv.client_fd > 0) {
			audio_recsrv_handle();
		}
#endif

	}
	
	return 0;
}

void ctrl_events_handle(void) 
{
	int cmd;
	int para_display = 0;
	unsigned char recvbuff[MSG_MAX] = {0};
    
    printf("ctrl_events_handle\n");
    if(para_display == 0) {
    	printf("------------after thread-------------\n");
    	printf("ctrl_events_handle, tcp_ctrl_events.client_fd = %d\n",tcp_ctrl_events.client_fd);
    	printf("ctrl_events_handle, tcp_ctrl_events.server_fd = %d\n",tcp_ctrl_events.server_fd);
    	printf("ctrl_events_handle, tcp_ctrl_events.port = %d\n",tcp_ctrl_events.port);
    	para_display = 1;
    }

	while(sizeof(recvbuff) != 0) {
		if(tcp_ctrl_events.server_fd > 0){
			Recv_Internet_Msg(&tcp_ctrl_events, recvbuff, MSG_MAX, 0);
			cmd = atoi(recvbuff);
			printf("recfbuff:%s,cmd:%d\n",recvbuff,cmd);			

			if(cmd == RMT_CALL_IN_REQ) {
				printf("RMT_CALL_IN_REQ\n");
				Send_Internet_Msg(&tcp_ctrl_events,"RMT_CALL_IN_REQ",30);

			} else if(cmd == RMT_HANDUP_SYNC) {
				printf("RMT_HANDUP_SYNC\n");
				Send_Internet_Msg(&tcp_ctrl_events,"RMT_HANDUP_SYNC",30);

			} else if(cmd == MSR_ANSWER_SYNC) {
				printf("MSR_ANSWER_SYNC\n");
				Send_Internet_Msg(&tcp_ctrl_events,"MSR_ANSWER_SYNC",30);

			} else if(cmd == MSR_HANDUP_SYNC) {
				printf("MSR_HANDUP_SYNC\n");
				Send_Internet_Msg(&tcp_ctrl_events,"MSR_HANDUP_SYNC",30);

			} else if(cmd == SLV_ANSWER_SYNC) {
				printf("SLV_ANSWER_SYNC\n");
				Send_Internet_Msg(&tcp_ctrl_events,"SLV_ANSWER_SYNC",30);

			} else if(cmd == SLV_HANDUP_SYNC) {
				printf("SLV_HANDUP_SYNC\n");
				Send_Internet_Msg(&tcp_ctrl_events,"SLV_HANDUP_SYNC",30);
			}else {
				printf("Error, no such case!!!\n");
			}

			memset(recvbuff, 0 ,MSG_MAX);
		}
	}
	return;
}

void audio_sndsrv_handle(void)
{	
	unsigned char recvbuff[MSG_MAX] = {0};
	unsigned char sendbuff[MSG_MAX] = {0};
	int flag = 0;
	
	if(flag == 0) {
		printf("------------audio_sndsrv_handle-------------\n");
    	printf("ctrl_events_handle, tcp_audio_send.client_fd = %d\n",tcp_audio_send.client_fd);
    	printf("ctrl_events_handle, tcp_audio_send.server_fd = %d\n",tcp_audio_send.server_fd);
    	printf("ctrl_events_handle, tcp_audio_send.port = %d\n",tcp_audio_send.port);
    	flag = 1;	
	}

	Recv_Internet_Msg(&tcp_audio_send, recvbuff, MSG_MAX, 0);
	printf("tcp_audio_send,recfbuff:%s,\n",recvbuff);
	sprintf(sendbuff, "audio_sndsrv_handle,%s", recvbuff);
	Send_Internet_Msg(&tcp_audio_send,sendbuff,MSG_MAX);
	memset(recvbuff,0,MSG_MAX);
	memset(sendbuff,0,MSG_MAX);

}


void audio_recsrv_handle(void) 
{
	unsigned char recvbuff[MSG_MAX] = {0};
	unsigned char sendbuff[MSG_MAX] = {0};
	int flag = 0;

	if(flag == 0) {
		printf("------------audio_recsrv_handle-------------\n");
    	printf("ctrl_events_handle, tcp_audio_recv.client_fd = %d\n",tcp_audio_recv.client_fd);
    	printf("ctrl_events_handle, tcp_audio_recv.server_fd = %d\n",tcp_audio_recv.server_fd);
    	printf("ctrl_events_handle, tcp_audio_recv.port = %d\n",tcp_audio_recv.port);
    	flag = 1;	
	}

	Recv_Internet_Msg(&tcp_audio_recv, recvbuff, MSG_MAX, 0);
	printf("tcp_audio_recv,recfbuff:%s,\n",recvbuff);
	sprintf(sendbuff, "audio_recvsrv_handle,%s", recvbuff);
	Send_Internet_Msg(&tcp_audio_recv,sendbuff,MSG_MAX);
	memset(recvbuff,0,MSG_MAX);
	memset(sendbuff,0,MSG_MAX);
}
