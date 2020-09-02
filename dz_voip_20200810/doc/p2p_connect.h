/*************************************************************************** 
  > File Name: p2p_connect.ch
  > Des      : TCP server线程，主要用于创建server并且不断检测是否有新的client请求连接 
  > Author   : Rick
  > Date     : 2020/07/14
 ***************************************************************************/
#ifndef __P2P_CONNECT__
#define __P2P_CONNECT__

//连接状态
extern volatile int connect_status;

void *p2p_connet_handle(void *arg);

#endif
