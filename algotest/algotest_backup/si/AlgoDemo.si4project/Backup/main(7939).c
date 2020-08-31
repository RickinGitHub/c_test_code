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
#include "duilite_algo.h"
#include "dz_voip.h"

int led_state  = 0;
int amp_enable = 0;

int key_vol_down    = 0;
int key_vol_up      = 0;
int key_mic_enable  = 0;
int key_phone_state = 0;

int main(int argc, char *argv[])
{
	static snd_pcm_t *handlec,*handlep;
	// output beamforming audio
	
	dz_set_params(&handlec, CAPTURE, DEF_CAPTURE_RATE);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");

	dz_set_params(&handlep, PLAYBACK, DEF_PLAYBACK_RATE);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");

#ifdef ALGORITHM	
	output_bf_audio = fopen("./bf.pcm", "wb");
	if (!output_bf_audio) {
		printf("ERROR: open %s error!", "./bf.pcm");
	}

	duilite_init(output_bf_audio);
#endif
	
	int server_status = 0;
	while(1) {
		if(server_status == 0) {
		pthread_t thread_audio_send,thread_audio_recv;
			
		pthread_create(&thread_audio_send, NULL, &audio_send_handle, NULL);
		pthread_detach(thread_audio_send);
		pthread_create(&thread_audio_recv, NULL, &audio_recv_handle, NULL);
		pthread_detach(thread_audio_recv);
		printf("audio handle has estabished!\n");	

		server_status = 1;
		}
	}
		   
	snd_pcm_drain(pcm_playback_handle.handle);
	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_playback_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);

#ifdef ALGORITHM
	duilite_exit();
	fclose(output_bf_audio);
#endif
	
	return 0;
}


