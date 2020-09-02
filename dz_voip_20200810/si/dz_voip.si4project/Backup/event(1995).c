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

#include "control.h"
#include "voip.h"
#include "p2p_connect.h"
#include "tcp_server.h"

void voice_call_answer(void);
void voice_call_handup(void);

void ctrl_events_handle(void) 
{
	int cmd;
	unsigned char recvbuff[MSG_MAX] = {0};
	unsigned char seddbuff[MSG_MAX] = {0};

	while(sizeof(recvbuff) != 0) {
		if(tcp_info.server_fd > 0){
			Recv_Internet_Msg(&tcp_info, recvbuff, MSG_MAX, 0);
			cmd = atoi(recvbuff);
			printf("recfbuff:%s,cmd:%d\n",recvbuff,cmd);			

			if(cmd == RMT_CALL_IN_REQ) {
				if(phone_state == PHONE_STATE_NORMAL) {
					set_phone_state(PHONE_STATE_WAIT);

				}				
				printf("RMT_CALL_IN_REQ\n");
			} else if(cmd == RMT_HANDUP_SYNC) {
				set_phone_state(PHONE_STATE_NORMAL);
				printf("RMT_HANDUP_SYNC\n");
				Send_Internet_Msg(&tcp_info,"RMT_HANDUP_SYNC",30);

			} else if(cmd == MSR_ANSWER_SYNC) {
				set_phone_state(PHONE_STATE_ACTIVATED);
				printf("MSR_ANSWER_SYNC\n");
				Send_Internet_Msg(&tcp_info,"MSR_ANSWER_SYNC",30);

			} else if(cmd == MSR_HANDUP_SYNC) {
				if(phone_state == PHONE_STATE_ACTIVATED) {
					set_phone_state(PHONE_STATE_NORMAL);
					printf("MSR_HANDUP_SYNC\n");
				}
				Send_Internet_Msg(&tcp_info,"MSR_HANDUP_SYNC",30);

			} else if(cmd == SLV_ANSWER_SYNC) {
				if(phone_state == PHONE_STATE_WAIT) {
					set_phone_state(PHONE_STATE_ACTIVATED);
					itoa(PHONE_STATE_ACTIVATED,seddbuff);
					Send_Internet_Msg(&tcp_info,seddbuff,strlen(seddbuff));
				}
				printf("SLV_ANSWER_SYNC\n");

			} else if(cmd == SLV_HANDUP_SYNC) {
				printf("SLV_HANDUP_SYNC\n");
				Send_Internet_Msg(&tcp_info,"SLV_HANDUP_SYNC",30);
			}else {
				printf("Error, no such case!!!\n");
			}

			memset(recvbuff, 0 ,MSG_MAX);
			memset(seddbuff, 0 ,MSG_MAX);
		}
	}
}
void voice_call_answer(void)
{
	static snd_pcm_t *handlec,*handlep;
	pthread_t thread_audio_send,thread_audio_recv;

	if(phone_state == PHONE_STATE_WAIT ) {
		set_phone_state(PHONE_STATE_ACTIVATED);
	}

	//pcm音频参数设置
	dz_set_params(&handlec, CAPTURE, DEF_CAPTURE_RATE);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");
	dz_set_params(&handlep, PLAYBACK, DEF_PLAYBACK_RATE);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");			

	//创建语音发送、接收线程
	pthread_create(&thread_audio_send, NULL, &audio_send_handle, NULL);
	pthread_detach(thread_audio_send);
	pthread_create(&thread_audio_recv, NULL, &audio_recv_handle, NULL);
	pthread_detach(thread_audio_recv);
	
	printf("audio handle has estabished!\n");
}

void voice_call_handup(void)
{
	set_phone_state(PHONE_STATE_NORMAL);
	snd_pcm_drain(pcm_playback_handle.handle);
	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_playback_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
}


