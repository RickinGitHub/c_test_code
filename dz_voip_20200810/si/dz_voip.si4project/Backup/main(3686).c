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
#include "control.h"
#include "p2p_connect.h"
#include "voip.h"
#include "key.h"

#define AUDIO

int state_led  = 0;
int enable_amp = 0;

void led_init(void);
void amp_init(void);
void key_init(void);
void p2p_init(void);

int main(int argc, char *argv[])
{
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
	pthread_t thrid_key_events;
	pthread_create(&thrid_key_events, NULL, &thread_key_events, NULL);
	pthread_detach(thrid_key_events);

	/**
	 * 5.打开pcm设备，设置音频相关参数，为后面语音通话准备
	 *
	 */
#ifdef AUDIO
	static snd_pcm_t *handlec,*handlep;
	dz_set_params(&handlec, CAPTURE, DEF_CAPTURE_RATE);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");

	dz_set_params(&handlep, PLAYBACK, DEF_PLAYBACK_RATE);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");
#endif

#ifdef AUDIO
	p2p_connect_state = 0;
	while(1) {
		if(p2p_connect_state == 0) {
			pthread_t thrid_control_stream;
			pthread_create(&thrid_control_stream, NULL, &control_stream_handle, NULL);
			pthread_detach(thrid_control_stream);
			printf("control stream has established!\n");

			p2p_connect_state = 1;

		} else {
			//printf("P2P DISCONNECTED!!!\n");
			continue;
		}
	}
#endif

#ifdef AUDIO
	snd_pcm_drain(pcm_playback_handle.handle);
	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_playback_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
#endif

	return 0;
}

#if 1
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

