
/* screen.c                                (c) Markus Hoffmann  2007-2010
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

/* These functions are based on the 
  Hello World Example
  Copyright (C) 2004 TomTom BV. All Rights Reserved.  */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/ioctl.h>
#include <sysexits.h>
#include "screen.h"

#ifndef NATIVE
#define FB_DEVICENAME "/dev/fb"
#else
#define FB_DEVICENAME "/dev/fb0"
#endif

int fbfd = -1;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
#if 0
char *fbbackp = 0;
#endif


void Fb_Open() {
  fbfd=open(FB_DEVICENAME, O_RDWR);
  if (!fbfd) {
    printf("ERROR: could not open framebufferdevice %s.\n",FB_DEVICENAME);
    exit(EX_OSFILE);
  }
#if DEBUG
  printf("Framebuffer device now opened.\n");
#endif
  /* Get fixed screen information  */
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("ERROR: Could not get fixed screen information.\n");
    exit(EX_IOERR);
  }
  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("ERROR: Could not get variable screen information.\n");
    exit(EX_IOERR);
  }

  printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

  // Figure out the size of the screen in bytes
  screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

  // Map the device to memory
  fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if((int)fbp==-1) {
    printf("ERROR: Could not map framebuffer device to memory.\n");
    exit(EX_IOERR);
  }
  
#if 0  
  fbbackp = (char*)malloc(screensize);
#endif
  /* Now set the padding to zero, otherwise it can happen that nothing is visible
    if the pading was set to the second page...*/

  vinfo.xoffset=0;
  vinfo.yoffset=0;  /* bzw. vinfo.yres fuer die 2te page */
  ioctl(fbfd,FBIOPAN_DISPLAY,&vinfo);

  printf("Framebuffer device is ready.\n");
}

void Fb_Close() {
  if(fbfd > 0) {
    munmap(fbp, screensize);
    close(fbfd);
  }
#if 0
  if(fbbackp) free(fbbackp);
#endif
  fbfd = -1;
}
#if 0
void FbRender_Clear(int y, int h, unsigned short color) {
  if (y<0|| y+h>ScreenHeight) return;
  unsigned short *ptr  = (unsigned short*)(fbbackp+y*((vinfo.xres*vinfo.bits_per_pixel)/8));
  unsigned short *endp = ptr + h*Scanline/sizeof(short);
  while (ptr < endp) *ptr++=color;
}
#endif
void Fb_Clear(int y, int h, unsigned short color) {
  if (y<0|| y+h>ScreenHeight) return;
  unsigned short *ptr  = (unsigned short*)(fbp+y*Scanline);
  unsigned short *endp = ptr + h*Scanline/sizeof(short);
  while (ptr < endp) *ptr++=color;
}

#if 0
inline void FbRender_PutPixel(int x, int y, unsigned short color) {
  unsigned short *ptr  = (unsigned short*)(fbbackp+x*2+y*Scanline);
  *ptr = color;
}
#endif
inline void Fb_PutPixel(int x, int y, unsigned short color) {
  unsigned short *ptr  = (unsigned short*)(fbp+x*2+y*Scanline);
  *ptr = color;
}

#if 0
inline void FbRender_Flush() {
  memcpy(fbp,fbbackp,screensize);
}

inline void FbRender_Scroll(int target_y, int source_y, int height) {
  memmove(fbbackp+target_y*Scanline,fbbackp+source_y*Scanline,height*Scanline);
}
#endif
inline void Fb_Scroll(int target_y, int source_y, int height) {
  memmove(fbp+target_y*Scanline,fbp+source_y*Scanline,height*Scanline);
}


unsigned short global_color=YELLOW;
unsigned short global_bcolor=BLACK;

void set_color(unsigned short color) {
  global_color=color;
}
void set_bcolor(unsigned short color) {
  global_bcolor=color;
}

void plot(int x, int y) {
  Fb_PutPixel(x,y,global_color);
}

unsigned short point(int x, int y) {
  unsigned short *ptr  = (unsigned short*)(fbp+x*2+y*Scanline);
  return(*ptr);
}

void DrawHorizontalLine(int X, int Y, int width, unsigned short color) {
  register int w = width;  // in pixels
#define iClipTop 0
#define iClipBottom ScreenHeight
#define iClipMin 0
#define iClipMax ScreenWidth

  if (Y<iClipTop) return;
  if (Y>=iClipBottom) return;

  if (iClipMin-X>0)      // clip left margin
    { w-=(iClipMin-X); X=iClipMin; }
  if (w>iClipMax-X)    // clip right margin
    w=iClipMax-X;

  // put the pixels...  
  {
    while ( w-- > 0 )
      Fb_PutPixel(X++,Y,color);
  }
}


/* Bresenham's line drawing algorithm */

void DrawLine(int x0, int y0, int x1, int y1,unsigned short color) {
  int dy = y1 - y0;
  int dx = x1 - x0;
  int stepx, stepy;

  if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
  if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
  dy <<= 1;						     // dy is now 2*dy
  dx <<= 1;						     // dx is now 2*dx

  Fb_PutPixel(x0, y0,color);
  if (dx > dy) {
      int fraction = dy - (dx >> 1);			     // same as 2*dy - dx
      while (x0 != x1) {
	  if (fraction >= 0) {
	      y0 += stepy;
	      fraction -= dx;				     // same as fraction -= 2*dx
	  }
	  x0 += stepx;
	  fraction += dy;				     // same as fraction -= 2*dy
	  Fb_PutPixel(x0, y0,color);
      }
  } else {
      int fraction = dx - (dy >> 1);
      while (y0 != y1) {
	  if (fraction >= 0) {
	      x0 += stepx;
	      fraction -= dy;
	  }
	  y0 += stepy;
	  fraction += dx;
	  Fb_PutPixel(x0, y0,color);
    }
  }
}



void line(int x1,int y1,int x2,int y2) {
  if(y1==y2) DrawHorizontalLine(x1,y1,x2-x1,global_color);
  else DrawLine(x1,y1,x2,y2,global_color);
}


void box(int x1,int y1,int x2,int y2) {
  DrawHorizontalLine(x1,y1,x2-x1,global_color);
  DrawHorizontalLine(x1,y2,x2-x1,global_color);
  DrawLine(x1,y1,x1,y2,global_color);
  DrawLine(x2,y1,x2,y2,global_color);
}



unsigned long FixSqrt(unsigned long x) {
  unsigned long r, nr, m;

  r = 0;
  m = 0x40000000;
  do {
    nr = r + m;
    if (nr <= x) {
      x -= nr;
      r = nr + m;
    }
    r >>= 1;
    m >>= 2;
  } while(m!=0);
  if(x>r) r++;
  return r;
}

void FillCircle (int cx, int cy, int aRad, unsigned short color) {
  int y;
  for (y=cy-aRad; y<=cy+aRad; ++y) {      
    register unsigned long tmp;
    tmp = aRad*aRad - (y - cy)*(y-cy);
    tmp = FixSqrt(tmp);
    DrawHorizontalLine( cx-tmp, y, tmp<<1, color);
  }  
}
void pbox(int x1, int y1, int x2, int y2) {
  FillBox(x1,y1,x2-x1,y2-y1,global_color);
}
void FillBox (int x, int y, int w, int h, unsigned short color) {
  int i;
  for (i=y; i<=y+h; i++) {
    DrawHorizontalLine(x, i, w, color);
  }  
}


void PutLinePoint(int x, int y, unsigned short color, int width) {  
  if (width>1) {
    FillCircle(x,y,width,color);
  } else {
    Fb_PutPixel(x, y, color);
  }
}

#if 0
void FbRender_inverse(int x, int y,int w,int h){

  if(x<0||y<0||w<=0||h<=0|| x+w>ScreenWidth|| y+h>ScreenHeight) return;

  register unsigned short *ptr  = (unsigned short*)(fbbackp+y*Scanline);
  ptr+=x;
  register unsigned short *endp  = ptr+h*ScreenWidth;
  register int i;
  while (ptr<endp) {
    for(i=0;i<w;i++) *ptr++=~*ptr;
    ptr+=ScreenWidth-w;
  }
}
#endif

void Fb_inverse(int x, int y,int w,int h){

  if(x<0||y<0||w<=0||h<=0|| x+w>ScreenWidth|| y+h>ScreenHeight) return;

  register unsigned short *ptr  = (unsigned short*)(fbp+y*Scanline);
  ptr+=x;
  register unsigned short *endp  = ptr+h*ScreenWidth;
  register int i;
  while (ptr<endp) {
    for(i=0;i<w;i++) *ptr++=~*ptr;
    ptr+=ScreenWidth-w;
  }
}
unsigned short int mean_color(unsigned short int a, unsigned short int b) {
  return(
  (((((a>>11)&0x1f)+((b>>11)&0x1f))>>2)<<11)|
  (((((a>>5)&0x3f)+((b>>5)&0x3f))>>2)<<5)|
  (((a&0x1f)+(b&0x1f))>>2));
}

void copyarea(int x,int y,int w, int h, int tx,int ty) {
  if(x<0||y<0||w<=0||h<=0|| x+w>ScreenWidth|| y+h>ScreenHeight||
  tx<0||ty<0||tx+w>ScreenWidth|| ty+h>ScreenHeight) return;
  register unsigned short *ptr1  = (unsigned short*)(fbp+y*Scanline);
  register unsigned short *ptr2  = (unsigned short*)(fbp+ty*Scanline);
  ptr1+=x;
  ptr2+=tx;

  register int i;
  if(y>ty) {
    for(i=0;i<h;i++) {
      memmove(ptr2,ptr1,w*sizeof(short));
      ptr1+=ScreenWidth;
      ptr2+=ScreenWidth;
    }
  } else {
    for(i=h-1;i>=0;i--) {
      memmove(&ptr2[i*ScreenWidth],&ptr1[i*ScreenWidth],w*sizeof(short));
    }
  }
}
