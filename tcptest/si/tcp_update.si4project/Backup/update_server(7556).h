#ifndef __UPDATE_SERVER_
#define __UPDATE_SERVER_

#define TCP_CTRL_PORT	10120
#define TCP_SEND_PORT	10121
#define TCP_RECV_PORT	10122
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
TCP_PARAM tcp_info,;
TCP_PARAM *tcp_ctrl_events, *tcp_audio_send, *tcp_audio_recv;
//TCP_PARAM tcp_ctrl_events;

extern int  Send_Internet_Msg(TCP_PARAM *net_info,unsigned char *buf,int len);
extern int Recv_Internet_Msg(TCP_PARAM *net_info,unsigned char *buf,int len, int flag);
extern void *Tcp_Server_Thread(void *server);

#endif
