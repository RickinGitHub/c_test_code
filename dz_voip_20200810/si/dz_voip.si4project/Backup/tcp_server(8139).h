/*************************************************************************** 
  > File Name: tcp_server.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#ifndef __TCP_SERVER__
#define __TCP_SERVER__

#define TCP_CTRL_PORT	10120
#define TCP_SEND_PORT	10121
#define TCP_RECV_PORT	10122

#define MSG_MAX          256

#define msleep(x) usleep(x*1000)

typedef struct _TCP_PARAM
{
    int server_fd;
    int client_fd;
    int port;
    int server_flag;
}TCP_PARAM;

//volatile  int connect_status;
TCP_PARAM tcp_ctrl_events, tcp_audio_send, tcp_audio_recv;

int server_send_msg(TCP_PARAM *net_info,unsigned char *buf,int len);
int server_recv_msg(TCP_PARAM *net_info,unsigned char *buf,int len, int flag);
extern void *tcp_server_thread(void *arg);

#endif
