/*************************************************************************** 
  > File Name: key.c 
  > Des      : 按键检测线程，主要话机端音量加、音量减、禁麦、挂断/接听4个按键检测
  > Author   : Rick
  > Date     : 2020/07/10
 ***************************************************************************/
#include <stdio.h>
#include <linux/input.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "key.h"

#define DEV_PATH "/dev/input/event0"
int volume_value = 0;
int state_mic    = DISABLE;
int state_phone  = DISABLE;

void volume_down(void);
void volume_up(void);
int  get_volme_val(void);
void set_phone_state(int state);
int  get_phone_state(void);
void set_mic_enable(int enable);
int  get_mic_state(void);

void *thread_key_events(void *argv)
{
	printf("---->thread_key_events\n");

	int fd;
	struct input_event key_event;
	
	if( (fd=open(DEV_PATH, O_RDONLY)) <= 0)
	{
		printf("open /dev/input/event2 device error!\n");
		exit(1);
	}

	while(1)
	{
		if(read(fd, &key_event, sizeof(key_event)) == sizeof(key_event)) {
			if( (key_event.type==EV_KEY) && (key_event.value==1))
			{
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
						if(get_phone_state()) {
							set_phone_state(DISABLE);
							printf("phone:%d\n",get_phone_state());
						}else {
							set_phone_state(ENABLE);
							printf("phone:%d\n",get_phone_state());
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
	}
	close(fd);
}

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

int get_volme_val(void){
	return volume_value;
}

void set_phone_state(int state)
{
	state_phone = state;
}

int get_phone_state(void)
{
	return state_phone;
}

void set_mic_enable(int enable)
{
	state_mic = enable;
}
int get_mic_state(void)
{
	return state_mic;
}

