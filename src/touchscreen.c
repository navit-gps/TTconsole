
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

#include <linux/input.h>
#include <linux/fb.h>

#include <barcelona/Barc_ts.h>

#include "screen.h"

#define TS_DEVICENAME1 "/dev/ts"
#define TS_DEVICENAME2 "/dev/input/event0"


int tsfd = -1;

int eventmode=0;
int rotatets=0;


typedef struct {
  int xMin;
  int xMax;
  int yMin;
  int yMax;
} MATRIX2;

static MATRIX2 ts_matrix;

//struct input_event {
//        struct timeval time;
//        unsigned short type;
//        unsigned short code;
//        unsigned int value;
//};



void TsScreen_Init() {
  tsfd = open(TS_DEVICENAME1, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (tsfd < 0) {
    tsfd = open(TS_DEVICENAME1, O_RDONLY | O_NOCTTY | O_NONBLOCK);
  }
  if (tsfd < 0) {
    printf("tomtom_touchscreen: ERROR could not open %s.\n",TS_DEVICENAME1);
    /* Try with different device */
    tsfd = open(TS_DEVICENAME2, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (tsfd < 0) {
      tsfd = open(TS_DEVICENAME2, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    }
    if (tsfd < 0) {
      printf("tomtom_touchscreen: ERROR could not open %s.\n",TS_DEVICENAME2);
      tsfd=-1;
    } else {
      eventmode=1;
      FILE *cal_file=fopen("/mnt/flash/sysfile/cal","rb");
      if((cal_file==NULL) || (fread(&ts_matrix.xMin, sizeof(int),4,cal_file)!=4)) {
        printf("Error reading calibration file!\n");
//        close(tsfd);
//        tsfd=-1;
      }
      fclose(cal_file);
    }
  } else ioctl(tsfd,TS_SET_RAW_OFF,NULL);
  if(tsfd!=-1) printf("Touchscreen device successfully opened.\n");
}

TS_EVENT new_event;

int TsScreen_read() {
  if(tsfd<0) return 0;
  if(eventmode) {
    struct input_event ev2;
    int retval=read(tsfd,&ev2,sizeof(struct input_event))==sizeof(struct
                input_event)?1:0;
    while(retval) {
      printf("ts_event: typ=%d code=%d value=%d \n",ev2.type,ev2.code,ev2.value);
      if(ev2.type==EV_ABS && ev2.code==ABS_X) {
        new_event.x=ev2.value;
        new_event.x=(((new_event.x-ts_matrix.xMin)*(vinfo.xres-30))/(ts_matrix.xMax-ts_matrix.xMin))+15;
//        if(new_event.x<0) new_event.x=0;
//        else 
	if (new_event.x>vinfo.xres) new_event.x=vinfo.xres;
        new_event.x=vinfo.xres-new_event.x;
printf("Xcal=%d\n",new_event.x);
          if(new_event.y!=0xffff) return 1; // -> full dataset.
      } else if (ev2.type==EV_ABS && ev2.code==ABS_Y) {
         new_event.y=ev2.value;
         new_event.y=(((new_event.y-ts_matrix.yMin)*(vinfo.yres-30))/(ts_matrix.yMax-ts_matrix.yMin))+15;
//        if(new_event.y<0) new_event.y=0;
//        else 
	if (new_event.y>vinfo.yres) new_event.y=vinfo.yres;
        new_event.y=vinfo.yres-new_event.y;
printf("Ycal=%d\n",new_event.y);
        if(new_event.x!=0xffff) return 1; // -> full dataset.
      } else if(ev2.type==EV_KEY && ev2.code==BTN_TOUCH) {
        if(ev2.value) {
          new_event.pressure=255;
          new_event.x=0xffff; // Reset...
          new_event.y=0xffff;
        } else {
          new_event.pressure=0;
          return 1; // button-release -> another full dataset.
        }
      }
      retval=read(tsfd,&ev2,sizeof(struct input_event))==sizeof(struct input_event)?1:0;
    }
    return retval;
  } else {    
    static int have_previous = 0;
    static TS_EVENT prev_event;
    int read_len;
    
    if(!have_previous) {
      read_len = read(tsfd, &prev_event, sizeof(TS_EVENT));
      if (read_len == sizeof(TS_EVENT)) have_previous=1;
    }
  // if we still don't have an event, there are no events pending, and we can just return
    if(!have_previous) return 0;
  
  // We have an event
    memcpy(&new_event, &prev_event,sizeof(TS_EVENT));
    have_previous = 0;
    return 1;
  }
}

int TsScreen_pen(int *x,int *y,int *pen) {

  if(TsScreen_read()==0) return 0;

  if(rotatets) {
   *x = new_event.y;
   *y = new_event.x;
  } else {
   *x = new_event.x;
   *y = new_event.y;
  }
  *pen = new_event.pressure;
  return 1;
}

int TsScreen_flush() {
  int count=0;
  while(TsScreen_read()) count++;
  return count;
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
