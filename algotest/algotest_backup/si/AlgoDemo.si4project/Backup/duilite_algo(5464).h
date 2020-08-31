/*************************************************************************** 
  > File Name: duilite_algo.h
  > Des      : none
  > Author   : Rick
  > Date     : 2020/08/19
 ***************************************************************************/
#ifndef __DUILITE_ALGO__
#define __DUILITE_ALGO__

#include "../include/duilite.h"

#define ALGORITHM

extern struct duilite_fespa *fespa;

extern int  wakeup_callback(void *user_data, int type, char *msg, int len);
extern int  doa_callback(void *user_data, int type, char *msg, int len);
extern int  beamforming_callback(void *user_data, int type, char *msg, int len);
extern int  duilite_init(FILE* output);
extern void duilite_exit(void);
extern void duilite_feed(struct duilite_fespa *fespa, char *data, int len);

#endif

