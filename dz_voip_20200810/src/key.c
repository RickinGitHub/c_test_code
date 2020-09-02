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
#include <pthread.h>

#include "key.h"
#include "events.h"
#include "tcp_server.h"
#include "phone.h"

#define DEV_PATH "/dev/input/event0"

int  volarg[8]   = {0,27,45,61,72,83,91,100}; //0%,14%,28%,43%,57%,71%,86%,100%
char cmd[64] = {0};

void volume_down(void)
{
	if(volume == 0) {
		sprintf(cmd,"amixer -M sset 'Master',0 %d &", volarg[volume]);
		printf("volume muted!!!\n");
	}else {
		sprintf(cmd,"amixer -M sset 'Master',0 %d &", volarg[--volume]);
		printf("volume peak!!!\n");
	}
	system(cmd);
	memset(cmd, 0, 64);
}
void volume_up(void)
{
	if(volume == 7) {
		sprintf(cmd,"amixer -M sset 'Master',0 %d &", volarg[volume]);
	}else {
		sprintf(cmd,"amixer -M sset 'Master',0 %d &", volarg[++volume]);
	}
	system(cmd);
	memset(cmd, 0, 64);
}
int  get_volme_value(void){
	return volume;
}
void set_phone_state(int state)
{
	phone_state = state;
}
int  get_phone_state(void)
{
	return phone_state;
}
void set_mic_state(void)
{
	int i;
	if(!micsta) {
		for(i = 0; i < 4; i++) {
			sprintf(cmd,"amixer -M sset 'ADC ALC Group %d Left',0 23 &", i);
			system(cmd);
			memset(cmd, 0, 64);
			sprintf(cmd,"amixer -M sset 'ADC ALC Group %d Right',0 23 &", i);
			system(cmd);
			memset(cmd, 0, 64);
		}
		micsta = 1;
	}else {
		for(i = 0; i < 4; i++) {
			sprintf(cmd,"amixer -M sset 'ADC ALC Group %d Left',0 0 &", i);
			system(cmd);
			memset(cmd, 0, 64);
			sprintf(cmd,"amixer -M sset 'ADC ALC Group %d Right',0 0 &", i);
			system(cmd);
			memset(cmd, 0, 64);
		}
		micsta = 0;
	}
	printf("micsta:%s\n",(micsta) ? "Wrok" : "Mute");
}
int  get_mic_state(void)
{
	return micsta;
}

void *dz_key_events(void *arg)
{
	printf("---->dz_key_events\n");

	int fd,i;
	int micsta = 0;
	struct input_event key_event;
	
	if( (fd=open(DEV_PATH, O_RDONLY)) <= 0)
	{
		printf("open /dev/input/event0 device error!\n");
		exit(-1);
	}
	
	while(1) {
		if(read(fd, &key_event, sizeof(key_event)) == sizeof(key_event)) {
			if( (key_event.type==EV_KEY) && (key_event.value==1))
			{
				unsigned char sendbuff[MSG_MAX] = {0};
				printf("key %d, %s\n", key_event.code, (key_event.value) ? "Pressed" : "Released");
				switch(key_event.code) {
					case KEY_VOLUMEDOWN:
						volume_down();
						break;
					case KEY_VOLUMEUP:
						volume_up();
						break;
					case KEY_MICMUTE:
						set_mic_state();				
						break;
					case KEY_PHONE: 
						if(tcp_ctrl_events.server_fd > 0) {
							voice_call_answer();
						}else {
							voice_call_handup();
						}
						printf("phone:%d\n",get_phone_state());
						break;
					default:
						break;
				}
			}
		}
	}
	
	close(fd);
}

