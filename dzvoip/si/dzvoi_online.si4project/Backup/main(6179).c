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
	int server_status = 0;
	while(1) {
		if(server_status == 0) {
		pthread_t thread_uplink,thread_downlink, thread_events;
			
		pthread_create(&thread_uplink, NULL, &voice_uplink, NULL);
		pthread_detach(thread_uplink);
		pthread_create(&thread_downlink, NULL, &voice_downlink, NULL);
		pthread_detach(thread_downlink);
		pthread_create(&thread_events, NULL, &events_capture, NULL);
		pthread_detach(thread_events);
		printf("audio handle has estabished!\n");	

		server_status = 1;
		}
		usleep(320000);
	}
	
	return 0;
}

