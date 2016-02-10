
/* touchscreen.c                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */



/* Some of these functions are based on the 
  Hello World Example
  Copyright (C) 2004 TomTom BV. All Rights Reserved.  */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <barcelona/Barc_ts.h>

int tsfd = -1;

void TsScreen_Init() {
  tsfd = open("/dev/ts", O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (tsfd < 0) {
    tsfd = open("/dev/ts", O_RDONLY | O_NOCTTY | O_NONBLOCK);
  }
  if (tsfd < 0) {
    printf("tomtom_touchscreen: ERROR could not open '/dev/ts'.\n");
    tsfd = -1;
  }

  ioctl(tsfd,TS_SET_RAW_OFF,NULL);
  printf("Touchscreen device successfully opened.\n");
}
int TsScreen_pen(int *x,int *y,int *pen) {
  if(tsfd<0) return 0;
  int read_len;
  TS_EVENT new_event;
 
  static int have_previous = 0;
  static TS_EVENT prev_event;

  if (!have_previous) {
    read_len = read(tsfd, &prev_event, sizeof(TS_EVENT));
    if (read_len == sizeof(TS_EVENT)) have_previous=1;
  }
  // if we still don't have an event, there are no events pending, and we can just return
  if(!have_previous) return 0;
  
  // We have an event
  memcpy(&new_event, &prev_event,sizeof(TS_EVENT));
  have_previous = 0;

#if 0
  read_len = read(tsfd, &prev_event, sizeof(TS_EVENT));
  if(read_len==sizeof(TS_EVENT)) have_previous=1;

  while (have_previous && (prev_event.pressure != 0) == (new_event.pressure != 0)) {
    memcpy(&new_event, &prev_event,sizeof(TS_EVENT));
    have_previous = 0;
    read_len = read(tsfd, &prev_event, sizeof(TS_EVENT));
    if (read_len == sizeof(TS_EVENT)) have_previous=1;
  }
#endif  
  *x = new_event.x;
  *y = new_event.y;
  *pen = new_event.pressure;
  return 1;
}
void TsScreen_Exit() {
  if(tsfd>=0) close(tsfd);
}

int TsScreen_waitevent(int timeout) {
  fd_set active_fd_set,read_fd_set;;
  int retval;
  struct timeval tv;
  
  if(tsfd<0) return 0;
  FD_ZERO (&active_fd_set);
  FD_SET (tsfd, &active_fd_set); 

    /* Wait up to five seconds. */
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
      read_fd_set = active_fd_set;

#if DEBUG
  if(timeout) g_outs("Waiting for an event.\n");
  else g_outs("Reading an event.\n");
#endif
  retval = select (FD_SETSIZE, &read_fd_set, NULL, NULL, &tv); 
  if(retval<0) g_outs("select failed!\n");
  else if(!retval) {
    if(timeout) {
      g_outs("++timeout waiting for touchscreen event!\n");
    }
    return(0);
  } else {
    if(FD_ISSET(tsfd,&read_fd_set)) {
      return(1);
    } else g_outs("Hier ist was komisch\n");
  }
  return(0);
}
