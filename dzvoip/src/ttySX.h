/*************************************************************************** 
  > File Name: ttySX.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/09/10
****************************************************************************/

#ifndef __TTYSX__
#define __TTYSX__

extern int ttySX_read(int fd, void *buf, int count);
extern int ttySX_write(int fd, const void *buf, int count);
extern int ttySX_init(int *fd);

#endif
