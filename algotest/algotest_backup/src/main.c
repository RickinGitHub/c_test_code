/*************************************************************************** 
  > File Name: main.c 
  > Des      : 主函数，包括I2C初始化(功放音量控制)、LED初始化、P2P连接初始化和回连, 
  >            创建按键检测线程、控制流线程、server初始化和检测client请求线程、语音
  >            通话接收/发送线程
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/tcp.h>   
#include <netinet/in.h>   
#include <netdb.h>
#include <pthread.h>
#include "dz_voip.h"

int main(int argc, char *argv[])
{

	int server_status = 0;
	while(1) {
		if(server_status == 0) {
		pthread_t thread_audio_send,thread_audio_recv;
		
		pthread_create(&thread_audio_recv, NULL, &audio_recv_handle, NULL);
		pthread_detach(thread_audio_recv);
		pthread_create(&thread_audio_send, NULL, &audio_send_handle, NULL);
		pthread_detach(thread_audio_send);
		printf("audio handle has estabished!\n");	

		server_status = 1;
		}
		usleep(320000);
	}
		   
	return 0;
}


