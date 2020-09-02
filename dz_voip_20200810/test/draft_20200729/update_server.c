#include <stdio.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/tcp.h>   
#include <netinet/in.h>   
#include <netdb.h>  
#include <unistd.h>
#include <fcntl.h>

#include "update_server.h"

volatile  int connect_state;

int  Send_Internet_Msg(TCP_PARAM *net_info,unsigned char *buf,int len)
{
    int ret=-1;	
    fd_set wfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    if(net_info->client_fd > 0)
    {
        FD_ZERO(&wfds);
        FD_SET(net_info->client_fd, &wfds);
        if ((ret=select(net_info->client_fd+1,NULL,&wfds,NULL,&tv)) > 0)
        {
            ret=send(net_info->client_fd, buf, len, 0);
        }
    }
    return ret;
}

int Recv_Internet_Msg(TCP_PARAM *net_info,unsigned char *buf,int len,int flag)
{
    int ret=-1;
    fd_set tcp_rfds;
    struct timeval tv;
    while(1)    
    {
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        if(net_info->client_fd > 0)
        {
            FD_ZERO(&tcp_rfds);
            FD_SET(net_info->client_fd, &tcp_rfds);
            if (select(net_info->client_fd + 1,&tcp_rfds,NULL,NULL,&tv) > 0)
            {
                ret=recv(net_info->client_fd, buf, len, 0);
                if(ret>0)
                {
                    return ret;
                }   
                else if(ret==0)// client disconnect
                {
                    printf("the connection break.......\n");
                    close(net_info->client_fd);
                    net_info->client_fd = -1;
                    connect_state = 0;
                    msleep(30);
                    return ret;
                }
                else    
                {
                    if(errno != EINTR  || errno != EWOULDBLOCK || errno !=EAGAIN)
                    {
                        fprintf(stderr,"the connection break:%s\n",strerror(errno));
                        close(net_info->client_fd);
                        connect_state = 0;
                        net_info->client_fd = -1;
                        return 0;               //return value
                    }   
                    printf("continue........................\n");
                    msleep(300);
                    continue;
                }
            }
            else
            {
                printf("Recv_Data_From_Record tcp time out...................\n");
                if(flag)
                {
                   return -1;
                }
            }
        }
        msleep(30);
    }
}

void *Tcp_Server_Thread(void *arg)
{
    int bFlag, sin_size;
    int client_fd;
    int ret;
    struct sockaddr_in tcp_server_addr;
    struct sockaddr_in tcp_client_addr;
    TCP_PARAM *tcp_paras = (TCP_PARAM *)arg;

    tcp_paras->server_flag = 0;

    while(1)  
    {          
        if(!tcp_paras->server_flag)   
        {   
            while((tcp_paras->server_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
            {
                fprintf(stderr,"socket error:%s\n\a",strerror(errno));
                msleep(30);
            }

            bFlag = 1;
            if(setsockopt(tcp_paras->server_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag)) == -1)//in binding, allow local addr reusing
            {
                printf("%s : %s\n",__func__,strerror(errno));
            }
            bzero(&tcp_server_addr,sizeof(struct sockaddr_in));
            tcp_server_addr.sin_family=AF_INET;
            tcp_server_addr.sin_addr.s_addr=INADDR_ANY;
            tcp_server_addr.sin_port=htons(tcp_paras->port);

            while(bind(tcp_paras->server_fd,(struct sockaddr *)(&tcp_server_addr),sizeof(struct sockaddr))==-1)//judge rebinding or not
            {
                fprintf(stderr,"bind error:%s\n\a",strerror(errno));
                msleep(10);
            }
        }
        tcp_paras->server_flag = 1;

        if(listen(tcp_paras->server_fd,USER_TCP_MAXSEG)==-1)      //fail the client give error is 
        {
            fprintf(stderr,"listen error:%s\n\a",strerror(errno));
            if(tcp_paras->server_fd > 0)
                close(tcp_paras->server_fd);
            tcp_paras->server_fd = -1;
            exit(0);
        }
        sin_size=sizeof(struct sockaddr_in);
        printf("wait accept.............\n");

        if( (client_fd=accept(tcp_paras->server_fd, (struct sockaddr *)(&tcp_client_addr), (socklen_t *)&sin_size )) == -1 )
        {
            fprintf(stderr,"accept error:%s\n\a",strerror(errno));
            if(tcp_paras->server_fd > 0)  
                close(tcp_paras->server_fd);
            tcp_paras->server_fd = -1;
            exit(0);
        }
        connect_state = 1;
        
        //close the previous socket
        if(tcp_paras->client_fd > 0)
        {
            printf("close the prior client socket\n");
            close(tcp_paras->client_fd);
        }
        tcp_paras->client_fd = client_fd;

        printf("the  tcp_paras.client_fd = %d\n",tcp_paras->client_fd);
        printf("the  tcp_paras.server_fd = %d\n",tcp_paras->server_fd);
        printf("the  tcp_paras.port = %d\n",tcp_paras->port);
        printf("accept success..........\n");
        msleep(500);
    }
}


