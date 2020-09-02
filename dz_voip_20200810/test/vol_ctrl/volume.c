#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
#define DEV_PATH "/dev/input/event0"
 
int main(int argc, char *argv[])
{
	int keys_fd,i;
	char ret[2];
	int micsta = 1;
	int val = 4;
	struct input_event key_event;
	int vol[8] = {0,27,45,61,72,83,91,100};
	char cmd[64] = {0};

#if 1
	for(i = 0; i < 4; i++) {
		sprintf(cmd,"amixer -M sset 'ADC ALC Group %d Left',0 23 &", i);
		system(cmd);
		memset(cmd, 0, 64);
		sprintf(cmd,"amixer -M sset 'ADC ALC Group %d Right',0 23 &", i);
		system(cmd);
		memset(cmd, 0, 64);
	}
	micsta = 1;

	sprintf(cmd,"amixer -M sset 'Master',0 %d &", vol[val]);
	system(cmd);
	memset(cmd, 0, 64);	
#endif

	keys_fd=open(DEV_PATH, O_RDONLY);
	if(keys_fd <= 0) {
		printf("open /dev/input/event0 device error!\n");
		return -1;
	}
	while(1) {
		if(read(keys_fd, &key_event, sizeof(key_event)) == sizeof(key_event)) {
			if(key_event.type==EV_KEY && key_event.value==1) {
				printf("key %d %s\n", key_event.code, (key_event.value) ? "Pressed" : "Released");
				#if 1
				switch(key_event.code) {
					case KEY_VOLUMEDOWN:
						if(val == 0) {
							sprintf(cmd,"amixer -M sset 'Master',0 %d &", vol[val]);
						}else {
							sprintf(cmd,"amixer -M sset 'Master',0 %d &", vol[--val]);
						}
						system(cmd);
						memset(cmd, 0, 64);
						break;
					case KEY_VOLUMEUP:
						if(val == 7) {
							sprintf(cmd,"amixer -M sset 'Master',0 %d &", vol[val]);
						}else {
							sprintf(cmd,"amixer -M sset 'Master',0 %d &", vol[++val]);
						}
						system(cmd);
						memset(cmd, 0, 64);
						break;
					case KEY_MICMUTE:
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
						break;
					default:
						break;
				}
				#endif		
			}
		}
	}
	close(keys_fd);
	return 0;
}
