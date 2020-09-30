/*************************************************************************** 
  > File Name: events.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/09/11
****************************************************************************/
#ifndef __EVENTS__
#define __EVENTS__

#define UART_CMD_PHONE			0x01   // 接听&挂断 s
#define UART_CMD_MUTE			0x02   // 静音		 s
#define UART_CMD_WAKEUP			0x03   // 远程唤醒	     s
#define UART_CMD_PAIR			0x04   // 执行对配      s
#define UART_CMD_SLEEP			0x05   // 休眠        s
#define UART_CMD_QUERY			0x06   // 查询        s
#define UART_CMD_PAIR_SUCCESS	0x07   // 配对成功      r
#define UART_CMD_DISCONNECT		0x08   // 断连        r
#define UART_CMD_CONNECT		0x09   // 连接        r
#define UART_CMD_RESERVED		0x0A   // reserved: 10~30(协议限制)

#define UART_CMD_BATTERY		0x97   // range: 151~250  s

enum state_type {	
	STATE_MICS  = 0,
	STATE_PHONE = 1,
	STATE_PAIR  = 2,
	STATE_LINK  = 3,
};

extern void *events_capture(void *argv);

#endif

