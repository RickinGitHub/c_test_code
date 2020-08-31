/************************************************************************* 
  > File Name: dz_voip_client.c 
  > Des      : duplex audio
  > Author   : Rick
  > Date     : 2020/05/26, modified on 07/14
 ************************************************************************/ 
 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/timeb.h>
#include<alsa/asoundlib.h>
#include<pthread.h>
#include "dz_voip.h"

#define TCP_SEND_PORT	10122
#define TCP_RECV_PORT	10121

PcmHandle pcm_capture_handle, pcm_playback_handle;
pthread_mutex_t mutex_audio;
//static snd_pcm_uframes_t frames   = 32;
//static unsigned int channels;

snd_pcm_uframes_t dz_set_params(snd_pcm_t **handle, int direct, int rate);
void *client_recv(void *arg);
void *client_send(void *arg);

int main(int argc, char** argv)
{
	pthread_t thread_id_recv, thread_id_send;
	pthread_mutex_init(&mutex_audio, NULL);

	if(argc != 2) {
		printf("Usage: %s <server ip>",argv[0]);
		return 0;
	}

	int client_status = 0;
	while(1) {
		if(!client_status) {
			//
			pthread_create(&thread_id_recv, NULL, &client_recv, &argv[1]);  //创建接收线程
			pthread_detach(thread_id_recv);

			//给
			pthread_create(&thread_id_send, NULL, &client_send, &argv[1]);  //创建发送线程
			pthread_detach(thread_id_send); // 线程分离，结束时自动回收资源

			client_status = 1;
		}
		usleep(320000);
	}

	snd_pcm_drain(pcm_playback_handle.handle);
	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_playback_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
	pthread_mutex_destroy(&mutex_audio);
	
    exit(0);
}

snd_pcm_uframes_t dz_set_params(snd_pcm_t **handle, int direct, int rate)
{
	int rc;
	int dir=0;
	snd_pcm_t* phandle;
	snd_pcm_hw_params_t* params;
	snd_pcm_stream_t stream;
	snd_pcm_format_t format   = DEFAULT_FORMAT;
	snd_pcm_uframes_t frames;
	static unsigned int channels;

	if(direct == PLAYBACK) {
		stream = SND_PCM_STREAM_PLAYBACK;
		channels =DEF_PLAYBACK_CHANALS;
		printf("SND_PCM_STREAM_PLAYBACK\n");
	}
	if(direct == CAPTURE) {
		stream = SND_PCM_STREAM_CAPTURE;
		channels = DEF_CAPTURE_CHANALS;
		printf("SND_PCM_STREAM_CAPTURE\n");
	}
	
	//stream = direct == PLAYBACK? SND_PCM_STREAM_PLAYBACK : SND_PCM_STREAM_CAPTURE;
	
	/* 1.设置硬件参数，将参数写入devices*/
	rc=snd_pcm_open(&phandle, "default", stream, 0);
	if(rc<0)
	{
	  perror("\nopen PCM device failed:");
	  exit(1);
	}
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params); //分配params结构体
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_alloca:");
	  exit(1);
	}

	/* Fill it in with default values. */
	rc=snd_pcm_hw_params_any(phandle, params);//初始化params
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
	rc=snd_pcm_hw_params_set_rate_near(phandle, params, &rate, &dir); 
	if(rc<0)
	{
	  perror("\nsnd_pcm_hw_params_set_rate_near:");
	  exit(1);
	}
	/* Set period size to 32 frames. */
	//snd_pcm_hw_params_set_period_size_near(phandle,params, &frames, &dir);
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
	//size = frames * 4; /* 2 bytes/sample, 2 channels */
	//printf("set hwparams finished, size:%d,frames:%d\n", size, frames);

	/* 将句柄指针传出去 */
	*handle = phandle;

	printf("--------[ Hardware Parameters Info. ]----------\n");
	printf("stream:%d\n",(int)stream);
	printf("frames:%d\n",(int)frames);
	printf("format:%d\n",(int)format);
	printf("chanel:%d\n",(int)channels);
	printf("rate  :%d\n",(int)rate);
	printf("-----------------------------------------------\n");

	return frames;

}

void *client_recv(void *arg)
{
    int recv_len = 0, rc;
	int size;
    char *recv_buf = NULL;   // 接收缓冲区
    char *server_ip = *(char **)arg;
    FILE *fp = NULL;
    int ret;
	static snd_pcm_t *handlep;
	static snd_pcm_uframes_t frames;
    //static snd_pcm_t *handle;

	printf("-------->SND_PCM_STREAM_PLAYBACK\nclient receive, IP:%s\n",server_ip);
#if 1
    int sock_recv;
    struct sockaddr_in servaddr;

    if ((sock_recv = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(TCP_RECV_PORT);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        printf("inet_pton error\n");
        exit(0);
    }
    
    if (connect(sock_recv, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }
    printf("connect success\n");
#endif

	frames = dz_set_params(&handlep, PLAYBACK, DEFAULT_RATE);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");

	size = frames * DEF_PLAYBACK_CHANALS * 2; /* 2 bytes/sample */
	recv_buf = (char *)malloc(size);
	memset(recv_buf, 0, size);

	fp = fopen("clien_recived_dump", "a+");
	if (fp ==NULL) {
		printf("open clien_recived.raw failed\n");
	}

	printf("------>dz_audio_playback\n");	
	while(1) {
		if((recv_len = recv(sock_recv, recv_buf, size, 0)) > 0)
    	{
			printf("client received from server:[%d]\n", recv_len);			
			pthread_mutex_lock(&mutex_audio);
			/* write data to DMA cache */
			rc = snd_pcm_writei(pcm_playback_handle.handle, recv_buf, frames);
			if (rc == -EPIPE) {
				/* EPIPE means overrun */
				fprintf(stderr, "overrun occurred\n");
				snd_pcm_prepare(pcm_playback_handle.handle);
			} else if (rc < 0) {
				fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
			} else if (rc != (int)frames) {
				fprintf(stderr, "short read, read %d frames\n", rc);
			}
			
			ret = fwrite((char *)recv_buf, 1, recv_len, fp);
			if(ret) {
				printf("fwrite:%d\n", ret);	
			}
			pthread_mutex_unlock(&mutex_audio);
			
			memset(recv_buf, 0, size);
		}
	}
	
    printf("client closed!\n");
	free(recv_buf);
	close(sock_recv);
	fclose(fp);

	return  NULL;
}

void *client_send(void *arg)
{
    int send_len = 0;
	int size;
	int rc;
    char *send_buf = NULL;    // 发送缓冲区
	char *server_ip = *(char **)arg;
	static snd_pcm_uframes_t frames;
		static snd_pcm_t *handlec;

	//static snd_pcm_t *handle;
    
	printf("-------->SND_PCM_STREAM_CAPTURE\nclient send, IP:%s\n",server_ip);

#if 1
		int sock_send;
		struct sockaddr_in servaddr;
	
		if ((sock_send = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
			exit(0);
		}
	
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(TCP_SEND_PORT);
		if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
			printf("inet_pton error\n");
			exit(0);
		}
		if (connect(sock_send, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
			printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
			exit(0);
		}
		printf("connect success\n");
		//*******************************************************************
#endif

	frames = dz_set_params(&handlec, CAPTURE, DEFAULT_RATE);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");

	size = frames * DEF_CAPTURE_CHANALS * 2; /* 2 bytes/sample */
	send_buf = (char *)malloc(size);
	memset(send_buf, 0, size);
	
	
    // 发送数据
    printf("send_buff size:%d frames:%d, ready to send...\n",size,frames);
    while (1)
    {
		pthread_mutex_lock(&mutex_audio);
		/* read data from DMA cache */
	    rc = snd_pcm_readi(pcm_capture_handle.handle, send_buf, frames);
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
		
		/* send data out */		
		if (send(sock_send, send_buf, size, 0) < 0) {
           printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
           exit(0);
        }
		printf("client send:%d\n", size);
		memset(send_buf, 0, size);
    }
    
    printf("client closed!\n");

	free(send_buf);
	close(sock_send);
	
    return  NULL;

}

