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

#define PORT_CTRL_STREAM	10120

unsigned char *head = "DangzhiVoIP";

void cli_send(int sockfd);
void despatcher(char *buff, int len);

void *control_stream_handle(void *arg)
{

	printf("---->control_stream_handle\n");
#if 1
	int server_status = 0;
	while(1) {
		if(server_status == 0) {
			pthread_t thread_audio_send,thread_audio_recv;
				
			pthread_create(&thread_audio_send, NULL, &audio_send_handle, NULL);
			pthread_detach(thread_audio_send);
			pthread_create(&thread_audio_recv, NULL, &audio_recv_handle, NULL);
			pthread_detach(thread_audio_recv);
			printf("audio handle has estabished!\n");	

			server_status = 1;
		} else {
			//printf("audio thread have established!!!\n");
		}
	}
#endif

}


#if 0
void cli_send(int sockfd)
{
	char *buff=NULL;
	buff = (char *)malloc(128);
	memset(buff, 0, 128);

	 while ((recv_len = recv(sockfd, buff, 128, 0)) > 0) {
		printf("server received:[%d]\n", recv_len);

		despatcher(buff,128);

		/* send data out */
		if (send(sockfd, buff, 128, 0) < 0) {
   			printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
   			exit(0);
		}
		printf("server send:[%d]\n", 128);

		memset(buff, 0, 128);
	 }


}

#endif
