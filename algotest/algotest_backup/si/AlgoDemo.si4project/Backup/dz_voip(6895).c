/*************************************************************************** 
  > File Name: dz_voip.c 
  > Des      : 语音通话线程，包括语音发送线程、语音接收线程
  >            同时包括语音采样率、通道、位深等音频参数
  > Author   : Rick
  > Date     : 2020/07/10
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
#include "dz_voip.h"
#include "duilite_algo.h"

#define TCP_SEND_PORT		10121
#define TCP_RECV_PORT		10122

PcmHandle pcm_capture_handle, pcm_playback_handle;
unsigned char *send_buf=NULL;
unsigned char *recv_buf=NULL;
FILE* output_bf_audio = NULL;


pthread_mutex_t mutex_audio,mutex_audio_aec;

void *server_send(void *arg);
void *server_recv(void *arg);

void *audio_send_handle(void *arg)
{
	printf("---->audio_send_handle\n");
	int server_fd  = 0;
    int client_fd  = 0;
    int err_log = 0;
	int flag = 1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    unsigned short port = TCP_SEND_PORT;    // 监听端口
    pthread_t thread_id_recv, thread_id_send;
    
    pthread_mutex_init(&mutex_audio, NULL);
	pthread_mutex_init(&mutex_audio_aec, NULL);
    
    printf("TCP server send,  port: %d\n", port);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket error");
        exit(-1);
    }
	if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&flag,sizeof(flag)) == -1)
    {
    	printf("%s : %s\n",__func__,strerror(errno));
    }
    
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    err_log = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (err_log != 0)
    {
        perror("bind");
        close(server_fd);      
        exit(-1);
    }
    
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
        char cli_ip[INET_ADDRSTRLEN] = "";
        socklen_t cliaddr_len = sizeof(client_addr);
                 
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &cliaddr_len);                              
       
        if (client_fd > 0)
        {
            pthread_create(&thread_id_send, NULL, &server_send, &client_fd);
            pthread_detach(thread_id_send);
        } else {
            perror("accept error this time!\n");
			close(client_fd);
            continue;
        }
    }
    
    close(server_fd);
	pthread_mutex_destroy(&mutex_audio);
	pthread_mutex_destroy(&mutex_audio_aec);

}

void *audio_recv_handle(void *arg)
{

	printf("---->audio_recv_handle\n");
	
	int server_fd  = 0;
    int client_fd  = 0;
    int err_log = 0;
	int flag = 1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    unsigned short port = TCP_RECV_PORT;
    pthread_t thread_id_recv, thread_id_send;
    pthread_mutex_init(&mutex_audio, NULL);
    
    printf("TCP server reveive, port: %d\n", port);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket error");
        exit(-1);
    }

	 if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&flag,sizeof(flag)) == -1)//in binding, allow local addr reusing
     {
     	printf("%s : %s\n",__func__,strerror(errno));
     }
    
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    err_log = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (err_log != 0)
    {
        perror("bind");
        close(server_fd);      
        exit(-1);
    }
    
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
        char cli_ip[INET_ADDRSTRLEN] = "";
        socklen_t cliaddr_len = sizeof(client_addr);
                 
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &cliaddr_len);                              		

		inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);
        printf("client ip=%s,port=%d\n", cli_ip,(int)ntohs(client_addr.sin_port));
        
        if (client_fd > 0)
        {
            pthread_create(&thread_id_recv, NULL, &server_recv, &client_fd);
            pthread_detach(thread_id_recv);

        } else {
			perror("accept this time");
			close(client_fd);
            continue;
    	}
    }
    
    close(server_fd);
}

void dz_set_params(snd_pcm_t **handle, int direct, int rate)
{
	int rc;
	int dir=0;
	int sample_rate = DEFAULT_RATE;
	snd_pcm_t* phandle;
	snd_pcm_hw_params_t* params;

	if(direct == PLAYBACK) {
		stream = SND_PCM_STREAM_PLAYBACK;
		//sample_rate = DEF_PLAYBACK_RATE;
		printf("------>stream:%d,SND_PCM_STREAM_PLAYBACK......\n",stream);
	}
	if(direct == CAPTURE) {
		stream = SND_PCM_STREAM_CAPTURE;
		//sample_rate = DEF_CAPTURE_RATE;
		printf("------>stream:%d,SND_PCM_STREAM_CAPTURE......\n",stream);
	}
	
	rc=snd_pcm_open(&phandle, "default", stream, 0);
	if(rc<0)
	{
	  perror("\nopen PCM device failed:");
	  exit(1);
	}
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_alloca:");
	  exit(1);
	}
	/* Fill it in with default values. */
	rc=snd_pcm_hw_params_any(phandle, params);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_any:");
	  exit(1);
	}
	/* Interleaved mode */
	rc=snd_pcm_hw_params_set_access(phandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(rc<0)
	{
	  perror("\nsed_pcm_hw_set_access:");
	  exit(1);

	}
	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(phandle, params, format);

	/* Two channels (stereo) */
	rc=snd_pcm_hw_params_set_channels(phandle, params, channels);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_set_channels:");
	  exit(1);
	}
	/* 44100 bits/second sampling rate (CD quality) */
	rc=snd_pcm_hw_params_set_rate_near(phandle, params, &sample_rate, &dir); 
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_set_rate_near:");
	  exit(1);
	}
	/* Set period size to 32 frames. */
	snd_pcm_hw_params_set_period_size_near(phandle,params, &frames, &dir);
	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(phandle, params);
	if(rc<0)
	{
		perror("\nsnd_pcm_hw_params: ");
		exit(1);
	}
	
	/* Use a buffer large enough to hold one period */
	rc=snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_get_period_size\n");
	  exit(1);
	}
	
	//把句柄传出去
	*handle = phandle;

	printf("--------[ Hardware Parameters Info. ]----------\n");
	printf("stream:%d\n",(int)stream);
	printf("frames:%d\n",(int)frames);
	printf("format:%d\n",(int)format);
	printf("chanel:%d\n",(int)channels);
	printf("rate  :%d\n",(int)rate);
	printf("-----------------------------------------------\n");

}


void *server_recv(void *arg)
{
    int recv_len = 0;
	int size;
	int rc;
    //char *recv_buf = NULL;
    int client_fd = *(int *)arg;
	//static snd_pcm_t *handle;

	printf("------>client_recv, SND_PCM_STREAM_PLAYBACK\n");
	
	/* set hardware parameters*/
	//dz_set_params(&handle, PLAYBACK, DEF_PLAYBACK_RATE);

	size = frames * channels * 2; /* 2 bytes/sample, 2 channels */
	recv_buf = (char *)malloc(size);
	memset(recv_buf, 0, size);
	
    printf("size:%d frames:%d, ready to receive...\n",size,frames);
	while(1) {
		while ((recv_len = recv(client_fd, recv_buf, size, 0)) > 0) {
			//printf("server_received:[%d]\n", recv_len);
			pthread_mutex_lock(&mutex_audio);
			rc = snd_pcm_writei(pcm_playback_handle.handle, recv_buf, frames);
			if (rc == -EPIPE) {
				/* EPIPE means underrun */
				fprintf(stderr, "underrun occurred\n");
				snd_pcm_prepare(pcm_playback_handle.handle);
			} else if (rc < 0) {
				fprintf(stderr,
						"error from writei: %s\n",
						snd_strerror(rc));
			} else if (rc != (int)frames) {
				fprintf(stderr,
					 "short write, write %d frames\n", rc);
			}
			pthread_mutex_unlock(&mutex_audio);
	
			/* keep data pure */
			memset(recv_buf, 0, size);
		}
    
    }
    
	free(recv_buf);
    close(client_fd);
    
    return  NULL;
}

void *server_send(void *arg)
{
    int send_len = 0;
	int size;
	int rc;
    //char *send_buf = NULL;
	char *send_buf_raw = NULL;
    int client_fd     = *(int *)arg;
	//static snd_pcm_t *handle;

	printf("------>server_send, SND_PCM_STREAM_CAPTURE\n");
	/* set hardware parameters*/
	//dz_set_params(&handle, CAPTURE, DEF_CAPTURE_RATE);

	size = frames * channels * 2; /* 2 bytes/sample, 2 channels */
	send_buf_raw = (char *)malloc(size);
	memset(send_buf_raw, 0, size);
	send_buf = (char *)malloc(size);
	memset(send_buf, 0, size);
	
    printf("size:%d frames:%d, ready to send...\n",size,frames);
    while (1)
    {
		pthread_mutex_lock(&mutex_audio);
	    rc = snd_pcm_readi(pcm_capture_handle.handle, send_buf_raw, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(pcm_capture_handle.handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
		pthread_mutex_unlock(&mutex_audio);
		
#ifdef ALGORITHM
		duilite_feed(fespa, send_buf_raw, size);
		//pthread_mutex_lock(&mutex_audio_aec);
		//fread(send_buf, 1, size, output_bf_audio);
		//pthread_mutex_unlock(&mutex_audio_aec);
#endif

		/* send data out */
		if (send(client_fd, send_buf_raw, size, 0) < 0) {
           printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
           exit(0);
        }
		printf("server send:[%d]\n", size);
		
		/* keep data pure */
		memset(send_buf, 0, size);
    }
    
	free(send_buf);
	free(send_buf_raw);
    close(client_fd);
    
    return  NULL;

}


