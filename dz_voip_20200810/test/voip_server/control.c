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
#include "dz_voip.h"
#include "p2p_connect.h"

#define PORT_CTRL_STREAM	10120

void cli_send(int sockfd);
void despatcher(char *buff, int len);

void *control_stream_handle(void *arg)
{
#if 0
	int err_log = 0;
	int flag = 1;
	int server_fd  = 0; // 套接字
	int client_fd  = 0;
	struct sockaddr_in server_addr; 		// 服务器地址结构体
	struct sockaddr_in client_addr; 		// 用于保存客户端地址
	unsigned short port = PORT_CTRL_STREAM;	// 监听端口
	
	printf("TCP Server Started at port %d!\n", port);
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);	// 创建TCP套接字
	if (server_fd < 0)
	{
		perror("socket error");
		exit(-1);
	}
	if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&flag,sizeof(flag)) == -1)
	{
		printf("%s : %s\n",__func__,strerror(errno));
	}
	
	bzero(&server_addr, sizeof(server_addr));		// 初始化服务器地址
	server_addr.sin_family = AF_INET;
	server_addr.sin_port   = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	printf("Binding server to port %d\n", port);
	
	// 绑定
	err_log = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (err_log != 0)
	{
		perror("bind");
		close(server_fd);	   
		exit(-1);
	}
	
	// 监听，套接字变被动
	err_log = listen(server_fd, 10);
	if (err_log != 0)
	{
		perror("listen");
		close(server_fd);	   
		exit(-1);
	}
	
	printf("Waiting client...\n");
	
	while (1)
	{
		char cli_ip[INET_ADDRSTRLEN] = "";		// 用于保存客户端IP地址
		int recv_len = 0;
		socklen_t cliaddr_len = sizeof(client_addr);
				
		//获得一个已经建立的连接   
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &cliaddr_len);							  
		if (client_fd < 0)
		{
			perror("accept this time");
			continue;
		}		 
		// 打印客户端的 ip 和端口
		inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);
		printf("client ip=%s,port=%d\n", cli_ip,(int)ntohs(client_addr.sin_port));
		
		if (client_fd > 0)
		{
			void cli_send(client_fd);
		}
	}
	close(client_fd);
	close(server_fd);
#endif

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

void despatcher(char *buff, int len)
{
	int fun_code = 0;
	int opt_code = 0;
	char *msg = buff;

	fun_code = *(buff + 12);
	opt_code = *(buff + 13);

	switch(fun_code) {
		case 1:
			//
			break;
		case 2:
			//
			break;
		case 3:
			//
			break;
		default:
			printf("Error!!! No such case.\n");
			break;
	}
}

#endif
