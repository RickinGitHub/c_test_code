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
unsigned char uartcmd[9] = {0x01, 0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
unsigned char powerval[] = {0x97,...,0xFA};

void set_state(int type, int val)
{
	printf("set_state, type:%d, val:%d\n", type, val);
	switch (type) {
		case STATE_MICS:
			state_mics = val;
			break;
		case STATE_PHONE:
			write(uart_fd, "1", 2);
			state_phone = val;
			break;
		case STATE_PAIR:
			write(uart_fd, "4", 2);
			stat_pair = val;
			break; 
		case STATE_LINK:
			write(uart_fd, "89", 4);
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

int system_wakeup(void)
{
	printf("-------->system_wakeup\n");

	return 0;

}

int link_pair(void)
{
	printf("-------->link_pair\n");

	return 0;

}

int system_sleep(void)
{
	printf("-------->system_sleep\n");

	return 0;
}

int query_cmd(int val)
{
	printf("-------->query_cmd\n");

	return 0;

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


void *events_capture(void *argv)
{
	int  event_type = 0;
	int  len;
	char buf[512]  = {0};
	int  keys_fd;
	struct input_event t;

	/* key */
	keys_fd=open("/dev/input/event0", O_RDONLY);
	if(keys_fd <= 0)
	{
		printf("open device error!\n");
		exit(1);
	}

	/* uart */
	ttySX_init(&uart_fd, "/dev/ttyS4", 19200);
	write(uart_fd, "Uart Connect Successfully!!!", 32);
		
	while(1) {
		while ( (len = read(uart_fd, buf, 512)) > 0) {
			printf("\nLen %d\n",len);
			buf[len+1]='\0';
			printf("RX:\n%s",buf);
			int cmd = atoi(buf);
			printf("cmd:%d\n", cmd);
			switch(cmd) {
				case UART_CMD_PHONE:
					printf("UART_CMD_PHONE\n");
					set_state(STATE_PHONE, 0);
					break;
				case UART_CMD_MUTE:
					printf("UART_CMD_MUTE\n");
					set_state(STATE_MICS, 0);
					break;
				case UART_CMD_WAKEUP:
					printf("UART_CMD_WAKEUP\n");
					system_wakeup();
					break;
				case UART_CMD_PAIR:
					printf("UART_CMD_PAIR\n");
					link_pair();
					break;
				case UART_CMD_SLEEP:
					printf("UART_CMD_SLEEP\n");
					system_sleep();
					break;
				case UART_CMD_QUERY:
					printf("UART_CMD_QUERY\n");
					query_cmd(2);
					break;
				case UART_CMD_PAIR_SUCCESS:
					printf("UART_CMD_PAIR_SUCCESS\n");
					set_state(STATE_LINK, 1);
					break;
				case UART_CMD_DISCONNECT:
					printf("UART_CMD_DISCONNECT\n");
					set_state(STATE_LINK, 0);
					break;
				case UART_CMD_CONNECT:
					printf("UART_CMD_CONNECT\n");
					set_state(STATE_LINK, 1);
					break;
			}	
		}
		
		while(read(keys_fd, &t, sizeof(t)) == sizeof(t)) {
			if(t.type==EV_KEY && t.value==1) {
				printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
				switch(t.code) {
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
					uart_fd
					printf("------>KEY_PHONE\n");
					break;
				case KEY_MICMUTE:
					set_state(STATE_MICS, 0);
					uart_fd
					printf("------>KEY_MICMUTE\n");
					break;
				default:
					break;
				}	
			}
		}
	}
	close(uart_fd);
	close(keys_fd);
}


