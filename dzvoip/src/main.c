/*************************************************************************** 
  > File Name: main.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/09/12
****************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include "dz_voip.h"
#include "events.h"

int main(int argc, char *argv[])
{
	int link_status   = 0;
	unsigned char cmd = 0x04;

	/* initialize flags */
	init_state();
	/* key */
	key_events_init();
	/* uart */
	uart_events_init();
	/* 2.4G pair request */
	//uart_tx(&cmd);
	
	while(1) {

		//key_events_capture();
		
		if(link_status == 0) {
			pthread_t thread_uplink,thread_downlink, thread_uart_events;
				
			pthread_create(&thread_uplink, NULL, &voice_uplink, NULL);
			pthread_detach(thread_uplink);
			pthread_create(&thread_downlink, NULL, &voice_downlink, NULL);
			pthread_detach(thread_downlink);
			pthread_create(&thread_uart_events, NULL, &uart_events_capture, NULL);
			pthread_detach(thread_uart_events);
			printf("audio handle has estabished!\n");	

			link_status = 1;
		}
		usleep(320000);
	}
	
	return 0;
}

