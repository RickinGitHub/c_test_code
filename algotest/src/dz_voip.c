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
#include <sys/syscall.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/tcp.h>   
#include <netinet/in.h>   
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>
#include "../include/duilite.h"
#include "dz_voip.h"

#define ALGORITHM

//#include "duilite_algo.h"

//AEC
char *auth_cfg = "{\"auth_dev_addr\": 2}";  // I2C addr
struct duilite_fespa *fespa;

//net
int client_fd = 0;
unsigned char *send_buf=NULL;
unsigned char *recv_buf=NULL;
//audio
PcmHandle pcm_capture_handle, pcm_playback_handle;
FILE* output_bf_audio = NULL;
pthread_mutex_t mutex_audio,mutex_audio_aec;
static unsigned int channelsp	  = DEF_PLAYBACK_CHANALS;
static unsigned int channelsc	  = DEF_CAPTURE_CHANALS;

void *server_send(void *arg);
void *server_recv(void *arg);
int   send_audio(char *buf, int len);
snd_pcm_uframes_t dz_set_params(snd_pcm_t **handle, int direct, int rate);

//speech
int  wakeup_callback(void *user_data, int type, char *msg, int len);
int  doa_callback(void *user_data, int type, char *msg, int len);
int  beamforming_callback(void *user_data, int type, char *msg, int len);
int  duilite_init(void);
void duilite_exit(void);
void duilite_feed(struct duilite_fespa *fespa, char *data, int len);
int pcm_s16le_split(const char *input, int ilen, char *output);

pid_t gettid() { return syscall(SYS_gettid);}

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

#if 0
snd_pcm_uframes_t dz_set_params(snd_pcm_t **handle, int direct, int rate)
{
	int rc;
	int dir=0;
	snd_pcm_stream_t stream;
	snd_pcm_format_t format    = DEFAULT_FORMAT;
	snd_pcm_t* phandle;
	snd_pcm_hw_params_t* params;
	snd_pcm_uframes_t frames;
	
	static snd_pcm_uframes_t frames_max=0;
	static snd_pcm_uframes_t frames_min=0;
	

	if(direct == PLAYBACK) {
		stream = SND_PCM_STREAM_PLAYBACK;
		channelsp = DEF_PLAYBACK_CHANALS;
		rc=snd_pcm_open(&phandle, "default", stream, 0);
		if(rc<0)
		{
	  		perror("\nopen PCM device failed:");
	  		exit(1);
		}
		printf("------>stream:%d,SND_PCM_STREAM_PLAYBACK......\n",stream);
	}
	if(direct == CAPTURE) {
		stream   = SND_PCM_STREAM_CAPTURE;
		channelsc = DEF_CAPTURE_CHANALS;
		rc=snd_pcm_open(&phandle, "mult_4_2", stream, 0);
		if(rc<0)
		{
	  		perror("\nopen PCM device failed:");
	  		exit(1);
		}
	/* Allocate a hardware parameters object. */
		printf("------>stream:%d,SND_PCM_STREAM_CAPTURE......\n",stream);
	}
	
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
	if(direct == PLAYBACK)
		rc=snd_pcm_hw_params_set_channels(phandle, params, channelsp);
	else
		rc=snd_pcm_hw_params_set_channels(phandle, params, channelsc);
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
	snd_pcm_hw_params_get_period_size_max(params, &frames_max, &dir);
	snd_pcm_hw_params_get_period_size_min(params, &frames_min, &dir);
	
	//把句柄传出去
	*handle = phandle;

	return frames;

	printf("--------[ Hardware Parameters Info. ]----------\n");
	printf("stream:%d\n",(int)stream);
	printf("frames:%d,max:%d, min:%d\n",(int)frames,(int)frames_max,(int)frames_min);
	printf("format:%d\n",(int)format);
	printf("chanel:%d,%d\n",(int)channelsp,(int)channelsc);
	printf("rate  :%d\n",(int)rate);
	printf("-----------------------------------------------\n");

}
#endif

snd_pcm_uframes_t set_params_playback(snd_pcm_t **handle)
{
	int rc;
	int dir=0;
	snd_pcm_t* phandle;
	snd_pcm_hw_params_t* params;
	snd_pcm_stream_t stream    = SND_PCM_STREAM_PLAYBACK;
	snd_pcm_format_t format    = SND_PCM_FORMAT_S16_LE;
	unsigned int 	 rate	   = DEFAULT_RATE;
	unsigned int    chanels    = DEF_PLAYBACK_CHANALS;
	snd_pcm_uframes_t frames;
	
	//static snd_pcm_uframes_t frames_max=0;
	//static snd_pcm_uframes_t frames_min=0;
	

	rc=snd_pcm_open(&phandle, "default", stream, 0);
	if(rc<0)
	{
  		perror("\nopen PCM device failed:");
  		exit(1);
	}
	printf("------>stream:%d,SND_PCM_STREAM_PLAYBACK......\n",stream);

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
	rc=snd_pcm_hw_params_set_channels(phandle, params, chanels);
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
	
	//snd_pcm_hw_params_get_period_size_max(params, &frames_max, &dir);
	//snd_pcm_hw_params_get_period_size_min(params, &frames_min, &dir);
	
	//把句柄传出去
	*handle = phandle;

	printf("--------[ PLAYBACK, Hardware Parameters Info. ]----------\n");
	printf("stream:%d\n",(int)stream);
	printf("frames:%d,max:%d, min:%d\n",(int)frames);
	printf("format:%d\n",(int)format);
	printf("chanel:%d\n",(int)chanels);
	printf("rate  :%d\n",(int)rate);
	printf("-----------------------------------------------\n");

	return frames;

}


snd_pcm_uframes_t set_params_capture(snd_pcm_t **handle)
{
	int rc;
	int dir=0;
	snd_pcm_t* phandle;
	snd_pcm_uframes_t frames;
	snd_pcm_hw_params_t* params;
	snd_pcm_stream_t stream    = SND_PCM_STREAM_CAPTURE;
	snd_pcm_format_t format    = SND_PCM_FORMAT_S16_LE;
	unsigned int 	 rate	   = DEFAULT_RATE;
	unsigned int    chanels   = DEF_CAPTURE_CHANALS;
	
	
	//static snd_pcm_uframes_t frames_max=0;
	//static snd_pcm_uframes_t frames_min=0;
	
	rc=snd_pcm_open(&phandle, "mult_4_2", stream, 0);
	if(rc<0)
	{
  		perror("\nopen PCM device failed:");
  		exit(1);
	}

	printf("------>stream:%d,SND_PCM_STREAM_CAPTURE......\n",stream);
		
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
	rc=snd_pcm_hw_params_set_channels(phandle, params, chanels);
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
	//snd_pcm_hw_params_get_period_size_max(params, &frames_max, &dir);
	//snd_pcm_hw_params_get_period_size_min(params, &frames_min, &dir);
	
	//把句柄传出去
	*handle = phandle;

	printf("--------[ CAPTURE, Hardware Parameters Info. ]----------\n");
	printf("stream:%d\n",(int)stream);
	printf("frames:%d\n",(int)frames);
	printf("format:%d\n",(int)format);
	printf("chanel:%d\n",(int)chanels);
	printf("rate  :%d\n",(int)rate);
	printf("-----------------------------------------------\n");

	return frames;

}


void *server_recv(void *arg)
{
    int recv_len = 0;
	int size;
	int rc;
    //char *recv_buf = NULL;
    int client_fd = *(int *)arg;
	static snd_pcm_t *handlep;
	snd_pcm_uframes_t frames;
	FILE * f_rcv = NULL;

	frames = set_params_playback(&handlep);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");

	printf("------>client_recv, SND_PCM_STREAM_PLAYBACK\n");
	printf("Process ID: %d, thread ID %d\n", getpid(), gettid());
	
	size = frames * DEF_PLAYBACK_CHANALS * 2; /* 2 bytes/sample */
	recv_buf = (char *)malloc(size);
	memset(recv_buf, 0, size);

	f_rcv = fopen("srv_recv_dump", "a+");
	if (f_rcv ==NULL) {
		printf("open srv_send.raw failed\n");
	}
	
    printf("size:%d frames:%d, ready to receive...\n",size,frames);
	while(1) {
		while ((recv_len = recv(client_fd, recv_buf, size, 0)) > 0) {
			printf("server_received:[%d]\n", recv_len);
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

			//dump raw data
			fwrite(recv_buf, 1, size, f_rcv);
	
			/* keep data pure */
			memset(recv_buf, 0, size);
		}
    
    }

	snd_pcm_drain(pcm_playback_handle.handle);
	snd_pcm_close(pcm_playback_handle.handle);
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
    client_fd     = *(int *)arg;
	static snd_pcm_t *handlec;
	static snd_pcm_uframes_t frames;
	FILE* fr;
	
	frames=set_params_capture(&handlec);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");

	printf("------>server_send, SND_PCM_STREAM_CAPTURE\n");
	printf("Process ID: %d, thread ID %d\n", getpid(), gettid());
	
	size = frames * DEF_CAPTURE_CHANALS * 2; /* 2 bytes/sample, 2 channels */
	//size = 2048 ;
	send_buf_raw = (char *)malloc(size);
	memset(send_buf_raw, 0, size);
	
	//send_buf = (char *)malloc(size);
	//memset(send_buf, 0, size);

	fr = fopen("srv_send_dump", "a+");
	if (fr ==NULL) {
		printf("open srv_send.raw failed\n");
	}

	//-------- speech algorithm init --------
	duilite_init();
	//---------------------------------------
	
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

		//dump raw data
		//fwrite(send_buf_raw, 1, size, fr);
		//send_audio(send_buf_raw,size);//send out
		
		//-------- speech algorithm feed --------
		duilite_feed(fespa, send_buf_raw,size);
		//---------------------------------------

		#if 0
		/* send data out */
		if (send(client_fd, send_buf, size, 0) < 0) {
           printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
           exit(0);
        }
		printf("server send:[%d]\n", size);
		#endif
		/* keep data pure */
		//memset(send_buf, 0, size);
		memset(send_buf_raw, 0, size);
		
    }


    //-------- speech algorithm exit --------
	duilite_exit();
	//---------------------------------------
	
	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
	//free(send_buf);
	free(send_buf_raw);
	fclose(fr);
    close(client_fd);
	
    
    return  NULL;

}


int send_audio(char *buf, int len)
{
	/* send data out */
	if (send(client_fd, buf, len, 0) < 0) {
		printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
	printf("server send:[%d]\n", len);
}


int wakeup_callback(void *user_data, int type, char *msg, int len) 
{
    printf("%.*s\n", len, msg);
    return 0;
}

int doa_callback(void *user_data, int type, char *msg, int len) 
{
    printf("%.*s\n", len, msg);
    return 0;
}

int beamforming_callback(void *user_data, int type, char *msg, int len) 
{
	//FILE* output = (FILE*)user_data;
	
	if (type == DUILITE_MSG_TYPE_JSON) {
		printf("-------->,DUILITE_MSG_TYPE_JSON%.*s", len, msg);
	} else if (type == DUILITE_MSG_TYPE_BINARY) {
		printf("-------->DUILITE_MSG_TYPE_BINARY,%d\n", len);
		if(len > 0) {
			//fwrite(msg, 1, len, output_bf_audio);
			pcm_s16le_split(msg,len,send_buf);
			send_audio(send_buf,len);//send out
			fwrite(send_buf, 1, len, output_bf_audio);
			memset(send_buf, 0, 3072);
		}
	}
    return 0;
}

int duilite_init(void)
{
	char *cfg = 
            "{ \
                \"aecBinPath\":\"../res/fesp/AEC_ch6-2-ch4_2ref_emd_20200508_v2.50.0.10.bin\",    \
                \"wakeupBinPath\":\"../res/fesp/wakeup_aifar_comm_20180104.bin\", \
                \"beamformingBinPath\":\"../res/fesp/UCA_asr_ch4-2-ch4_70mm_20200616_v2.0.0.40_wkpMTModeOff.bin\",    \
                \"env\":\"words=ni hao xiao le;thresh=0.13\",  \
                \"rollBack\":1200     \
            }";
    int r = duilite_library_load(auth_cfg);//加载库函数，完成初始化与授权，接口阻塞，调用一次即可
    printf("duilite_library_load :%d\n", r);
    
    fespa = duilite_fespa_new(cfg);//初始化fespa引擎，并在后台开启wakeup，echo和beamforming等计算线程
    assert(fespa != NULL);

    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_WAKEUP, wakeup_callback, NULL);
    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_DOA, doa_callback, NULL);
    duilite_fespa_register(fespa, DUILITE_CALLBACK_FESPA_BEAMFORMING, beamforming_callback, NULL);

	output_bf_audio = fopen("callback_dump", "a+");
	if (output_bf_audio ==NULL) {
		printf("open srv_send.raw failed\n");
	}

	send_buf = (char *)malloc(3072);
	memset(send_buf, 0, 3072);

}

void duilite_exit(void)
{
	duilite_fespa_delete(fespa);//销毁fespa引擎
    duilite_library_release();//释放库资源
    fclose(output_bf_audio);
	free(send_buf);
}

void duilite_feed(struct duilite_fespa *fespa, char *data, int len)
{			 
	duilite_fespa_feed(fespa, data, len);//输入音频数据,格式为16k采样率，有符号16bit编码
	//usleep(32000);
}

int pcm_s16le_split(const char *input, int ilen, char *output)
{
	int  size = 6, i;
	char *sample = (char*)malloc(size);

	memset(sample, 0, size);
	for(i = 0; i < ilen; i+=2) {
		memcpy(sample,   input+i,   1);
		memcpy(sample+1, input+i+1, 1);
		memcpy(output + i*6+0, sample, 2);
		memcpy(output + i*6+2, sample, 2);
		memcpy(output + i*6+4, sample, 2);
		memcpy(output + i*6+6, sample, 2);
		memcpy(output + i*6+8, sample, 2);
		memcpy(output + i*6+10, sample, 2);
		memset(sample, 0, size);
	}
	free(sample);

	return ilen*6;
}




