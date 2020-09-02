/*************************************************************************** 
  > File Name: phone.h
  > Des      : 话机主函数头文件
  > Author   : Rick
  > Date     : 2020/08/03
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

#include "events.h"
#include "p2p_connect.h"
#include "voip.h"
#include "key.h"
#include "tcp_server.h"

#define AUDIO

int state_led  = 0;
int enable_amp = 0;

pthread_mutex_t mutex_phone_stat;

void led_init(void);
void amp_init(void);
void p2p_init(void);

void audio_params_init(void);
void audio_params_exit(void);

