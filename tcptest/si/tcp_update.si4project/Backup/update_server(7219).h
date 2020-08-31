#ifndef __UPDATE_SERVER_
#define __UPDATE_SERVER_

#define UPDATE_TCP_PORT 10090
#define USER_TCP_MAXSEG 256

#define msleep(x) usleep(x*1000)

typedef struct _TCP_PARAM
{
    int server_fd;
    int client_fd;
    int port;
    int server_flag;
}TCP_PARAM;

extern volatile  int connect_state;
TCP_PARAM tcp_info;

extern int  Send_Internet_Msg(TCP_PARAM *net_info,unsigned char *buf,int len);
extern int Recv_Internet_Msg(TCP_PARAM *net_info,unsigned char *buf,int len, int flag);
extern int Update_Server_Thread(void);

#endif
