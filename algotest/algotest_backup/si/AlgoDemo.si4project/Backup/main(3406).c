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
//#include <linux/iio/iio.h>
//#include <linux/iio/consumer.h>

#include "control.h"
#include "p2p_connect.h"
#include "dz_voip.h"
#include "key.h"

//#define #define   SYSFS_ADC_DIR   "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

int led_state  = 0;
int amp_enable = 0;

int key_vol_down    = 0;
int key_vol_up      = 0;
int key_mic_enable  = 0;
int key_phone_state = 0;


int main(int argc, char *argv[])
{
	static snd_pcm_t *handlec,*handlep;
	dz_set_params(&handlec, CAPTURE, DEF_CAPTURE_RATE);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");

	dz_set_params(&handlep, PLAYBACK, DEF_PLAYBACK_RATE);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");
	
#if 1
	connect_status = 0;
	while(1) {
		if(connect_status == 0) {
			pthread_t thread_p2p_connect,thread_control_stream;
			/* 4. server初始化，client连接请求线程*/
			pthread_create(&thread_p2p_connect, NULL, &p2p_connet_handle, NULL);
			pthread_detach(thread_p2p_connect);
		
			pthread_create(&thread_control_stream, NULL, &control_stream_handle, NULL);
			pthread_detach(thread_control_stream);
			printf("control stream has established!\n");

			connect_status = 1;

		} else {
			//printf("P2P DISCONNECTED!!!\n");
			continue;
		}
	}
#endif
		   
	snd_pcm_drain(pcm_playback_handle.handle);
	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_playback_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
	
	return 0;
}
