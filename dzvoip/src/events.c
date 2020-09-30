/*************************************************************************** 
  > File Name: events.c
  > Des      : Capture events of keys and cmd of uart
  > Author   : Rick
  > Date     : 2020/09/11
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "events.h"
#include "ttySX.h"

int state_mics  = 0;
int state_phone = 0;
int stat_pair   = 0;
int state_link  = 0;
int volume = 0;
int volarg[8]   = {0,27,45,61,72,83,91,100}; //0%,14%,28%,43%,57%,71%,86%,100%
char cmd[64]    = {0};
int uart_fd;
int keys_fd;

unsigned char uartcmd[11] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B};
unsigned char power = 0xFF;

void uart_tx(char *cmd);


void uart_tx(char *cmd)
{
	printf("-------->uart_tx:0x%02x\n", *cmd);
	ttySX_write(uart_fd, cmd, 1);
}

void uart_rx(char *buf)
{
	printf("-------->uart_rx\n");
}


void set_state(int type, int val)
{
	printf("set_state, type:%d, val:%d\n", type, val);
	switch (type) {
		case STATE_MICS:
			if(state_mics  == ENABLE) {	
				state_mics = DISABLE;
			} else {
				state_mics = ENABLE;
			}
			break;
		case STATE_PHONE:
			if(state_phone == TRUE) {
				state_phone == FALSE;
			} else {
				state_phone == TRUE;
			}
			break;
		case STATE_PAIR:
			stat_pair = val;
			break; 
		case STATE_LINK:
			state_link = val;
			break;			
	}
}

int get_state(int type)
{
	printf("get_state, type:%d, \n", type);
	switch (type) {
		case STATE_MICS:
			return state_mics;
		case STATE_PHONE:
			return state_phone;
		case STATE_PAIR:
			return stat_pair;
		case STATE_LINK:
			return state_link;			
	}
	return -1;
}

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


int get_volume(void)
{
	printf("volume:%d\n", volume);
	return volume;
}


/*******************************************
 *
 * bref : Power managements
 *
 *******************************************/
void system_suspend(void)
{
	int fd;
	
	if( fd=open("/sys/power/state", O_RDWR)) {
		printf("---->system suspend\n");
		write(fd, "mem", 3);
	}
}

void system_wakeup()
{


}

/********************************************
 *
 * bref : Power lights and interactive lights
 *
 ********************************************/

void lights_of_power()
{


}

void lights_of_interaction()
{


}

/******************************************
 *
 * bref : Status of neccessay modules
 *
 ******************************************/
void init_state(void)
{
	state_mics  = 0;
    state_phone = 0;
    stat_pair   = 0;
    state_link  = 0;
    volume      = 74;
	power       = 0xFA;
}

int uart_events_init(void)
{
	ttySX_init(&uart_fd);
}

void uart_events_handle(void)
{
	if(!get_state(STATE_MICS))
		printf("MICS MUTED!!!\n");
	if(!get_state(STATE_PHONE))
		printf("VoIP IS'T ESTABLISHED!!!\n");
	if(!get_state(STATE_PAIR))
		printf("2.4G IS'T PAIRED!!!\n");
	if(!get_state(STATE_LINK))
		printf("2.4G IS'T CONNECTED!!!\n");
}

void *uart_events_capture(void *arg)
{
	char buf[8] = {0};
	int  len    = 0;

	/* Query link state */
	uart_tx(uartcmd +5);
	
	while(1) {
		while ( (len = ttySX_read(uart_fd, buf, 8)) > 0) {
			printf("Len %d\n",len);
			for(int i = 0; i < len; i++)
				printf("RX: 0x%02x ", buf[i]);
			#if 1
			switch(buf[0]) {
				case UART_CMD_PAIR_SUCCESS:
					set_state(STATE_LINK, 1);
					break;
				case UART_CMD_DISCONNECT:
					set_state(STATE_LINK, 0);
					break;
				case UART_CMD_CONNECT:
					set_state(STATE_LINK, 1);
					break;
			}	
			#endif
		}
		
		uart_events_handle();
		
	}
}


int key_events_init(void)
{
	keys_fd = open("/dev/input/event0", O_RDONLY | O_NDELAY);
	if(keys_fd <= 0)
	{
		printf("open event0 error!\n");
		exit(1);
	}
}

void key_events_handle()
{
	printf("---->key_events_handle\n");

}

void key_events_capture(void)
{
	struct input_event event;
	
	while(read(keys_fd, &event, sizeof(event)) == sizeof(event)) {
		if(event.type==EV_KEY && event.value==1) {
			printf("key %d %s\n", event.code, (event.value) ? "Pressed" : "Released");
			switch(event.code) {
			case KEY_VOLUMEDOWN:
				volume_down();
				printf("------>KEY_VOLUMEDOWN\n");
				break;
			case KEY_VOLUMEUP:
				volume_up();
				printf("------>KEY_VOLUMEUP\n");
				break;
			case KEY_PHONE:
				set_state(STATE_PHONE, 0);
				printf("------>KEY_PHONE\n");
				uart_tx(uartcmd);
				break;
			case KEY_MICMUTE:
				set_state(STATE_MICS, 0);
				uart_tx(uartcmd + 1);
				printf("------>KEY_MICMUTE\n");
				break;
			default:
				break;
			}	
		}
	}
}

