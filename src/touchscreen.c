
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
#include <unistd.h>
#include <string.h>

#include <linux/input.h>
#include <linux/fb.h>

#include <barcelona/Barc_ts.h>

#include "screen.h"
#include "terminal.h"

#define TS_DEVICENAME1 "/dev/ts"

#ifndef NATIVE
#define TS_DEVICENAME2 "/dev/input/event0"
#else
//#define TS_DEVICENAME2 "/dev/mouse"
#define TS_DEVICENAME2 "/dev/input/mouse0"
#endif

#define EV_SYN			0x00

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

typedef struct {
  unsigned char but;
  signed char dx;
  signed char dy;
} MOUSE_EVENT;


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
      }
      if(cal_file!=NULL) fclose(cal_file);
    }
#ifndef NATIVE
  } else ioctl(tsfd,TS_SET_RAW_OFF,NULL);
#else
  }
#endif
#if DEBUG
  if(tsfd!=-1) printf("Touchscreen device successfully opened. Eventmode=%d.\n",eventmode);
#endif
}

static TS_EVENT new_event;

int TsScreen_read() {
  if(tsfd<0) return 0;
  if(eventmode) {
#ifndef NATIVE
    struct input_event ev2;
    int retval=read(tsfd,&ev2,sizeof(struct input_event))==sizeof(struct
                input_event)?1:0;
    if(new_event.pressure!=0xffff) {
      new_event.pressure=0xffff;  /* Reset ???*/
    }
    while(retval) {
#else
    MOUSE_EVENT ev2;
    int retval=read(tsfd,&ev2,sizeof(MOUSE_EVENT))==sizeof(
                MOUSE_EVENT)?1:0;
    while(retval || 1) {
      printf("mouse_event: but=%d dx=%d dy=%d \n",ev2.but,ev2.dx,ev2.dy);
#endif
#ifndef NATIVE
#if DEBUG
      if(ev2.type==EV_SYN) printf(" SYN ");
      else 
#endif
      if(ev2.type==EV_ABS && ev2.code==ABS_X) {
        new_event.x=ev2.value;
        new_event.x=(((new_event.x-ts_matrix.xMin)*(vinfo.xres-30))/(ts_matrix.xMax-ts_matrix.xMin))+15;
//        if(new_event.x<0) new_event.x=0;
//        else 
	if (new_event.x>vinfo.xres) new_event.x=vinfo.xres;
        new_event.x=vinfo.xres-new_event.x;
#if DEBUG
        printf("x=%d ",new_event.x);
#endif
      } else if (ev2.type==EV_ABS && ev2.code==ABS_Y) {
         new_event.y=ev2.value;
         new_event.y=(((new_event.y-ts_matrix.yMin)*(vinfo.yres-30))/(ts_matrix.yMax-ts_matrix.yMin))+15;
//        if(new_event.y<0) new_event.y=0;
//        else 
	if (new_event.y>vinfo.yres) new_event.y=vinfo.yres;
        new_event.y=vinfo.yres-new_event.y;
#if DEBUG
        printf("y=%d ",new_event.y);
#endif
      } else if(ev2.type==EV_ABS && ev2.code==ABS_PRESSURE) {
        new_event.pressure=ev2.value;
#if DEBUG
        printf("Pres=%d\n",new_event.pressure);
#endif
        if(new_event.x!=0xffff && new_event.y!=0xffff) break; // -> full dataset.
      } else if(ev2.type==EV_KEY && ev2.code==BTN_TOUCH) {
        if(ev2.value) {
          new_event.pressure=255;
        } else new_event.pressure=0;
#if DEBUG
        printf("touch=%d\n",ev2.value);
#endif       
	if(new_event.x!=0xffff && new_event.y!=0xffff) break; // -> full dataset.       
      } 
#if DEBUG
      else {
        printf("unknown input_event: typ=%d code=%d value=%d \n",ev2.type,ev2.code,ev2.value);
      }
#endif
      retval=read(tsfd,&ev2,sizeof(struct input_event))==sizeof(struct input_event)?1:0;
#else
      retval=read(tsfd,&ev2,sizeof(MOUSE_EVENT))==sizeof(MOUSE_EVENT)?1:0;
#endif
    }
#if DEBUG
    fflush(stdout);
    printf("TSREAD--> %d\n",retval);
#endif
    return retval;
  } else {    
#ifndef NATIVE
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
#endif
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
