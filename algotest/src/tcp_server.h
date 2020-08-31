/*************************************************************************** 
  > File Name: tcp_server.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#ifndef __TCP_SERVER__
#define __TCP_SERVER__

#define TCP_SERVER_PORT 10090
#define USER_TCP_MAXSEG 256

#define msleep(x) usleep(x*1000)

typedef struct _TCP_PARAM
{
    int server_fd;
    int client_fd;
    int port;
    int server_flag;
}TCP_PARAM;

//volatile  int connect_status;
TCP_PARAM tcp_info;

int server_send_msg(TCP_PARAM *net_info,unsigned char *buf,int len);
int server_recv_msg(TCP_PARAM *net_info,unsigned char *buf,int len, int flag);
int server_thread(void);

#endif
