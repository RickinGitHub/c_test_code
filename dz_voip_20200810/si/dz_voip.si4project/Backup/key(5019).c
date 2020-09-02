/*************************************************************************** 
  > File Name: key.c 
  > Des      : 按键检测线程，主要话机端音量加、音量减、禁麦、挂断/接听4个按键检测
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#include <stdio.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "key.h"
#include "events.h"
#include "tcp_server.h"

#define DEV_PATH "/dev/input/event0"

int volume_value = 0;
int mic_mute     = DISABLE;
phone_state_t phone_state = PHONE_STATE_NORMAL;


void volume_down(void)
{
	if(volume_value == 0) {
		printf("amplifier muted!!!\n");
	}else {
		volume_value--;
	}
}
void volume_up(void)
{
	if(volume_value == 7) {
		printf("peak value\n");
	}else {
		volume_value++;
	}
}
int  get_volme_val(void){
	return volume_value;
}
void set_phone_state(int state)
{
	phone_state = state;
}
int  get_phone_state(void)
{
	return phone_state;
}
void set_mic_enable(int enable)
{
	mic_mute = enable;
}
int  get_mic_state(void)
{
	return mic_mute;
}
void dz_key_events(void)
{
	printf("---->dz_key_events\n");

	int fd;
	struct input_event key_event;
	
	if( (fd=open(DEV_PATH, O_RDONLY)) <= 0)
	{
		printf("open /dev/input/event0 device error!\n");
		return;
	}

	if(read(fd, &key_event, sizeof(key_event)) == sizeof(key_event)) {
		if( (key_event.type==EV_KEY) && (key_event.value==1))
		{
		    unsigned char sendbuff[MSG_MAX] = {0};
			printf("key %d, %s\n", key_event.code, (key_event.value) ? "Pressed" : "Released");
			switch(key_event.code) {
				case KEY_VOLUMEDOWN:
					volume_down();
					printf("volume:%d\n",get_volme_val());
					break;
				case KEY_VOLUMEUP:
					volume_up();
					printf("volume:%d\n",get_volme_val());
					break;
				case KEY_PHONE:	
					if((tcp_ctrl_events.server_fd > 0) && (phone_state == PHONE_STATE_WAIT)) {
						voice_call_answer();
						
						set_phone_state(PHONE_STATE_ACTIVATED);
					    //itoa(PHONE_STATE_ACTIVATED,sendbuff,4);
					    //server_send_msg(&tcp_ctrl_events,sendbuff,strlen(sendbuff));
						printf("phone:%d\n",get_phone_state());
					}else if((tcp_ctrl_events.server_fd > 0) && (phone_state == PHONE_STATE_ACTIVATED)){

						set_phone_state(PHONE_STATE_NORMAL);
						//itoa(PHONE_STATE_ACTIVATED,sendbuff, 4);
						//server_send_msg(&tcp_ctrl_events,sendbuff,strlen(sendbuff));
						printf("phone:%d\n",get_phone_state());
					}else {
						printf("phone state:%d\n",get_phone_state());
					}
					break;
				case KEY_MICMUTE:
					if(get_mic_state()) {
						set_mic_enable(DISABLE);
					} else {
						set_mic_enable(ENABLE);
					}
					printf("mic:%d\n",get_mic_state());
					break;
				default:
					//
					break;
			}
		}
	}
	close(fd);
}

