#ifndef __TTYSX__
#define __TTYSX__

extern int ttySX_read(int fd, void *buf, int count);
extern int ttySX_write(int fd, const void *buf, int count);
extern int ttySX_init(int *fd, char *dev, int speed);

#endif
