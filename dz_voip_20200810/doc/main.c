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
//#include <linux/iio/iio.h>
//#include <linux/iio/consumer.h>

#include "control.h"
#include "p2p_connect.h"
#include "dz_voip.h"
#include "key.h"

//#define #define   SYSFS_ADC_DIR   "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

int led_state  = 0;
int amp_enable = 0;

int key_vol_down    = 0;
int key_vol_up      = 0;
int key_mic_enable  = 0;
int key_phone_state = 0;

void led_init(void);
void amp_init(void);
void key_init(void);
void p2p_init(void);

int main(int argc, char *argv[])
{
	/* 1. LED */
	//led_init();
	/* 2.功放初始化 */
	//amp_init();	
	/* 3.按键初始化、按键检测线程 */
	//key_init();		
	/* 4.P2P */
	//p2p_init();

	static snd_pcm_t *handlec,*handlep;
	dz_set_params(&handlec, CAPTURE, DEF_CAPTURE_RATE);
	pcm_capture_handle.handle = handlec;
	strcpy(pcm_capture_handle.name, "pcm_capture");

	dz_set_params(&handlep, PLAYBACK, DEF_PLAYBACK_RATE);
	pcm_playback_handle.handle = handlep;
	strcpy(pcm_playback_handle.name, "pcm_playback");
	
#if 1
	connect_status = 0;
	while(1) {
		if(connect_status == 0) {
			pthread_t thread_p2p_connect,thread_control_stream;
			/* 4. server初始化，client连接请求线程*/
			pthread_create(&thread_p2p_connect, NULL, &p2p_connet_handle, NULL);
			pthread_detach(thread_p2p_connect);
		
			pthread_create(&thread_control_stream, NULL, &control_stream_handle, NULL);
			pthread_detach(thread_control_stream);
			printf("control stream has established!\n");

			connect_status = 1;

		} else {
			//printf("P2P DISCONNECTED!!!\n");
			continue;
		}
	}
#endif

#if 0		   
	while(1) {
		if(connect_status) {
			// p2p 

		} else {
			// server未初始化，创建server

		}

		/* 5.开启控制流线程 */
		if(control_stream_flag) {

		}else {


		}

		/* 6.开启语音通话 */
		if(client_fd) {
			//初始化音频采样率等参数
			//创建语音发送线程
			//创建语音接收线程

		}else {
			//关闭套接字

		}
	}
#endif
	snd_pcm_drain(pcm_playback_handle.handle);
	snd_pcm_drain(pcm_capture_handle.handle);
	snd_pcm_close(pcm_playback_handle.handle);
	snd_pcm_close(pcm_capture_handle.handle);
	
	return 0;
}

#if 0
void led_init(void)
{
#if 1
	/* 电源指示灯 */
	//读gpio
	//写gpio
#endif
}

void amp_init(void)
{
	/* 系统初始化阶段，对功放芯片不供电，当系统初始化完成后，再通过i2c将其使能*/
	
	// 打开i2c设备节点
	// i2c读
	// i2c写
}

void key_init(void)
{
#if 1
	struct iio_channel *chan;
	int value,ret;
	
	/* 1.初始化各个按键状态 */
	chan = iio_channel_get(&pdev->dev, NULL);
	if(iio_read_channel_raw(chan, &value) > 0) {
		printf("key_val:%d\n",value);

	}
#endif
}

void p2p_init(void)
{
#if 0
	int ret;
	/* 
	   1.初始化p2p, 设置为gc模式，不断请求连
	   2.检查是否连接成功 
	   3. 如果连接成功，则设置连接状态
	 */

	
	if(ret) {
		connect_status = 1;
	} else {
		connect_status = 0;
	}
	
	printf("p2p connected!\n");
#endif

}

#if 1

/**
* iio_read_channel_raw() - read from a given channel
* @chan:       The channel being queried.
* @val:        Value read back.
*
* Note raw reads from iio channels are in adc counts and hence
* scale will need to be applied if standard units required.
*/

int iio_read_channel_raw(struct iio_channel *chan,       int *val);

/**
 * struct iio_channel - everything needed for a consumer to use a channel
 * @indio_dev:      Device on which the channel exists.
 * @channel:        Full description of the channel.
 * @data:       Data about the channel used by consumer.
 */
struct iio_channel {
    struct iio_dev *indio_dev;
    const struct iio_chan_spec *channel;
    void *data;
};

/*
1、获取 AD 通道
        struct iio_channel *chan; //定义 IIO 通道结构体
        chan = iio_channel_get(&pdev->dev, NULL); //获取 IIO 通道结构体
2、读取 AD 采集到的原始数据
        int value,Vresult;
        ret = iio_read_channel_raw(chan, &value);
        调用 iio_read_channel_raw 函数读取 AD 采集的原始数据并存入value变量中。
*/

int gpiod_get_direction(struct gpio_desc *desc);
int gpiod_direction_input(struct gpio_desc *desc);
int gpiod_direction_output(struct gpio_desc *desc, int value);
int gpiod_direction_output_raw(struct gpio_desc *desc, int value);

/* Value get/set from non-sleeping context */
int gpiod_get_value(const struct gpio_desc *desc);
void gpiod_set_value(struct gpio_desc *desc, int value);
void gpiod_set_array_value(unsigned int array_size,
               struct gpio_desc **desc_array, int *value_array);
int gpiod_get_raw_value(const struct gpio_desc *desc);
void gpiod_set_raw_value(struct gpio_desc *desc, int value);
void gpiod_set_raw_array_value(unsigned int array_size,
                   struct gpio_desc **desc_array,
                   int *value_array);
#endif

#define adc1_val_voldown	89
#define adc1_val_voldown	107108
#define adc1_val_voldown	326327
#define SARA	557558

void key_event(void)
{
	int chan, val;

	chan = 1;

	iio_read_channel_raw(&chan, &val);
	if(val )

	


}
#endif

