/*************************************************************************** 
  > File Name: main.c 
  > Des      : 主函数，包括I2C初始化(功放音量控制)、LED初始化、P2P连接初始化和回连, 
  >            创建按键检测线程、控制流线程、server初始化和检测client请求线程、语音
  >            通话接收/发送线程
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/tcp.h>   
#include <netinet/in.h>   
#include <netdb.h>
#include <pthread.h>

#include "events.h"
#include "p2p_connect.h"
#include "voip.h"
#include "key.h"
#include "tcp_server.h"

#define AUDIO

int state_led  = 0;
int enable_amp = 0;

pthread_mutex_t mutex_phone_stat;

void led_init(void);
void amp_init(void);
void p2p_init(void);

void audio_params_init(void);
void audio_params_exit(void);

int main(int argc, char *argv[])
{
	int ctrl_srv_stat = 0;
	int audio_sndsrv_stat = 0;
	int audio_recsrv_stat = 0;
	int key_events_stat   = 0;
	/**
	 *1. LED开机状态显示
	 */
	//led_init();
	
	/**
	 * 2.功放初始化
	 * 初始化使能(在boot阶段关闭功放，防止音爆)、音量大小等参数	
	 */
	//amp_init();
	
	/**
	 * 3.P2P 
	 * 初始化p2p相关参数，将p2p设置为GC模式，请求连接GO
	 * 如果连接成功，设置连接状态标志
	 */
	//p2p_init();

	/* 4.按键初始化、按键检测线程
	 * 初始化各个按键初始状态，开启按键检测线程
	 *
	 */
	//key_init();
	/**
	 * 5.打开pcm设备，设置音频相关参数，为后面语音通话准备
	 *
	 */

	pthread_mutex_init(&mutex_phone_stat, NULL);
	//pthread_mutex_lock(&mutex_phone_stat);
	 
	audio_params_init();

	p2p_connect_state = 1;
	while(1) {
		if(p2p_connect_state) {
			if(!ctrl_srv_stat) {
				pthread_t thrd_ctrl_srv;
				pthread_create(&thrd_ctrl_srv, NULL, &tcp_server_thread, NULL);
				pthread_detach(thrd_ctrl_srv);
				ctrl_srv_stat = 1;
			}
			if(tcp_ctrl_events.client_fd > 0) {
				ctrl_events_handle();
			}

			if(!audio_sndsrv_stat) {
				pthread_t thrd_audio_sndsrv;
				pthread_create(&thrd_audio_sndsrv, NULL, &audio_send_handle, NULL);
				pthread_detach(thrd_audio_sndsrv);
				audio_sndsrv_stat = 1;
			}

			if(!audio_recsrv_stat) {
				pthread_t thrd_audio_recvsrv;
				pthread_create(&thrd_audio_recvsrv, NULL, &audio_recv_handle, NULL);
				pthread_detach(thrd_audio_recvsrv);
				audio_recsrv_stat = 1;

			}

			if(!key_events_stat) {
				pthread_t thrd_key_events;
				pthread_create(&thrd_key_events, NULL, &dz_key_events, NULL);
				pthread_detach(thrd_key_events);
				key_events_stat = 1;
			}
		}
	}

	audio_params_exit();
	
	return 0;
}

#if 0
void led_init(void)
{
#if 0
	/* 电源指示灯 */
	//读gpio
	//写gpio
#endif
}

void amp_init(void)
{
	/* 系统初始化阶段，对功放芯片不供电，当系统初始化完成后，再通过i2c将其使能*/
	
	// 打开i2c设备节点
	// i2c读
	// i2c写
}

void key_init(void)
{
#if 0
	struct iio_channel *chan;
	int value,ret;
	
	/* 1.初始化各个按键状态 */
	chan = iio_channel_get(&pdev->dev, NULL);
	if(iio_read_channel_raw(chan, &value) > 0) {
		printf("key_val:%d\n",value);

	}
#endif
}

int p2p_connect(void)
{
	system("wpa_cli -D");
}

void p2p_init(void)
{
#if 1
	int ret;
	/* 
	   1.初始化p2p, 设置为gc模式，不断请求连
	   2.检查是否连接成功 
	   3. 如果连接成功，则设置连接状态
	 */
	ret = p2p_connect();
	if(ret) {
		p2p_connect_state = 1;
	} else {
		p2p_connect_state = 0;
	}
	
	printf("p2p connected!\n");
#endif
}
#endif

void audio_params_init(void)
{
	static snd_pcm_t *handlec;
	static snd_pcm_t *handlep;
	dz_set_params(&handlec, CAPTURE, DEF_CAPTURE_RATE);
	pcm_capture_handle = handlec;
	dz_set_params(&handlep, PLAYBACK, DEF_PLAYBACK_RATE);
	pcm_playback_handle = handlep;
}

void audio_params_exit(void)
{
	snd_pcm_drain(pcm_playback_handle);
	snd_pcm_drain(pcm_capture_handle);
	snd_pcm_close(pcm_playback_handle);
	snd_pcm_close(pcm_capture_handle);
}

