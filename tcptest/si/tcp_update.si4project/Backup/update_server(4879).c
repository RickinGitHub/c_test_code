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

int Update_Server_Thread(void)
{     
    struct sockaddr_in tcp_server_addr;
    struct sockaddr_in tcp_client_addr;
    tcp_info.port = UPDATE_TCP_PORT;
    tcp_info.server_flag = 0;

    int bFlag, sin_size;
    int client_fd;
    int ret;

    while(1)                //server performs constantly   
    {          
        if(!tcp_info.server_flag)   
        {   
            while((tcp_info.server_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
            {
                fprintf(stderr,"socket error:%s\n\a",strerror(errno));
                msleep(30);
            }

            bFlag = 1;
            if(setsockopt(tcp_info.server_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag)) == -1)//允许bind过程中，本地地址可重复使用
            {
                printf("%s : %s\n",__func__,strerror(errno));
            }
            bzero(&tcp_server_addr,sizeof(struct sockaddr_in));
            tcp_server_addr.sin_family=AF_INET;
            tcp_server_addr.sin_addr.s_addr=INADDR_ANY;
            tcp_server_addr.sin_port=htons(tcp_info.port);

            while(bind(tcp_info.server_fd,(struct sockaddr *)(&tcp_server_addr),sizeof(struct sockaddr))==-1)//判断是否重复绑定
            {
                fprintf(stderr,"bind error:%s\n\a",strerror(errno));
                msleep(10);
            }
        }
        tcp_info.server_flag = 1;

        if(listen(tcp_info.server_fd,USER_TCP_MAXSEG)==-1)      //fail the client give error is 
        {
            fprintf(stderr,"listen error:%s\n\a",strerror(errno));
            if(tcp_info.server_fd > 0)
                close(tcp_info.server_fd);
            tcp_info.server_fd = -1;
            return -1;
        }
        sin_size=sizeof(struct sockaddr_in);
        printf("wait accept.............\n");

        if( (client_fd=accept(tcp_info.server_fd, (struct sockaddr *)(&tcp_client_addr), (socklen_t *)&sin_size )) == -1 )
        {
            fprintf(stderr,"accept error:%s\n\a",strerror(errno));
            if(tcp_info.server_fd > 0)  
                close(tcp_info.server_fd);
            tcp_info.server_fd = -1;
            return -1;
        }
        connect_state = 1;
        
        //close previous socket
        if(tcp_info.client_fd > 0)
        {
            printf("close the prior client socket\n");
            close(tcp_info.client_fd);
        }
        tcp_info.client_fd = client_fd;
        printf("the  tcp_info.client_fd = %d\n",tcp_info.client_fd);
        printf("the  tcp_info.server_fd = %d\n",tcp_info.server_fd);
        printf("accept success..........\n");
        msleep(500);
    }
    return 0;  
}


