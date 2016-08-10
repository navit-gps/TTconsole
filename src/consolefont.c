/* consolefont.c                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/fb.h>
#include "screen.h"
#include "consolefont.h"


int bigfont=0;  /* Default use the small font*/
int fontnr=0;   /* Number of Charackter set (font) */

unsigned char *fontlist57[10]={
(unsigned char *)asciiTable,(unsigned char *)asciiTable,
(unsigned char *)asciiTable,(unsigned char *)asciiTable,
(unsigned char *)asciiTable,(unsigned char *)asciiTable,
(unsigned char *)asciiTable,(unsigned char *)asciiTable,
(unsigned char *)asciiTable,(unsigned char *)asciiTable};

unsigned char *fontlist816[10]={
(unsigned char *)spat_a816,(unsigned char *)ibm_like816,
(unsigned char *)ext_font816,(unsigned char *)spat_a816,
(unsigned char *)spat_a816,(unsigned char *)spat_a816,
(unsigned char *)spat_a816,(unsigned char *)spat_a816,
(unsigned char *)spat_a816,(unsigned char *)spat_a816};

extern int bigkeys;

void Fb_BlitCharacter(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character, int flags, int fontnr){
  if(bigfont) Fb_BlitCharacter816(x,y,aColor,aBackColor,character,flags,fontnr);
  else Fb_BlitCharacter57(x,y,aColor,aBackColor,character,flags,fontnr);
}

void Fb_BlitCharacter57(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character, int flags, int fontnr){
  unsigned char data0,data1,data2,data3,data4;

  if (x<0||y<0|| x>ScreenWidth-CharWidth57 || y>ScreenHeight-CharHeight57) return;
//  if ((character < Fontmin) || (character > Fontmax)) character = '.';

  const unsigned char *aptr = &((fontlist57[fontnr])[character*5]);
  data0 = *aptr++;
  data1 = *aptr++;
  data2 = *aptr++;
  data3 = *aptr++;
  data4 = *aptr;

  register unsigned short *ptr  = (unsigned short*)(fbp+y*Scanline);
  ptr+=x;
  register unsigned short *endp  = ptr+CharHeight57*ScreenWidth;

  if(flags&FL_REVERSE) { /* reverse */
    unsigned short t=aBackColor;
    aBackColor=aColor;
    aColor=t;
  }
#if 0
  /* Sieht nicht gut aus (unlesbar) deshalb ignorieren*/
  if(flags&FL_BOLD) {/* bold */
    data0|=data1;
    data1|=data2;
    data2|=data3;
    data3|=data4;
  }
#endif  
  if(flags&FL_UNDERLINE || flags&FL_DBLUNDERLINE) {/* underline */
    data0|=0x80;
    data1|=0x80;
    data2|=0x80;
    data3|=0x80;
    data4|=0x80;
  }
  if(flags&FL_HIDDEN) {
    data0=data1=data2=data3=data4=0;
  }
  if(flags&FL_TRANSPARENT) {
    while (ptr<endp) {
      if (data0 & 1) *ptr= aColor;  ptr++; data0 >>= 1;    
      if (data1 & 1) *ptr= aColor;  ptr++; data1 >>= 1;
      if (data2 & 1) *ptr= aColor;  ptr++; data2 >>= 1;
      if (data3 & 1) *ptr= aColor;  ptr++; data3 >>= 1;
      if (data4 & 1) *ptr= aColor;  ptr++; data4 >>= 1;
      ptr+=ScreenWidth-CharWidth57;
    }
  } else {
    while (ptr<endp) {
      if (data0 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data0 >>= 1;    
      if (data1 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data1 >>= 1;
      if (data2 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data2 >>= 1;
      if (data3 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data3 >>= 1;
      if (data4 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data4 >>= 1;
      ptr+=ScreenWidth-CharWidth57;
    }
  }
}

/* Routine taken from X11-Basic */

void Fb_BlitCharacter816(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character, int flags,int fontnr){
  char charackter[CharHeight816];
  int i,d;
  if (x<0||y<0|| x>ScreenWidth-CharWidth816 || y>ScreenHeight-CharHeight816) return;
  const unsigned char *aptr = &((fontlist816[fontnr])[character*CharHeight816]);
  memcpy(charackter,aptr,CharHeight816);
  register unsigned short *ptr  = (unsigned short*)(fbp+y*Scanline);

  if(flags&FL_REVERSE) { /* reverse */
    unsigned short t=aBackColor;
    aBackColor=aColor;
    aColor=t;
  }  
  if(flags&FL_FRAMED) {/* Framed */
    charackter[0]=0xff;
    for(i=1;i<16;i++) charackter[i]|=1;
  }
  if(flags&FL_UNDERLINE) {/* underline */
    charackter[15]=0xff;
    charackter[14]=0xff;
  } else if(flags&FL_DBLUNDERLINE) {/* double underline */
    charackter[15]=0xff;
    charackter[13]=0xff;
  }
  if(flags&FL_DIM) {
    for(i=0;i<16;i++) {
      charackter[i++]&=0xaa;
      charackter[i]  &=0x55;
    }
  } else if(flags&FL_CONCREAL) {
    for(i=0;i<16;i++) {
      charackter[i++]|=0xaa;
      charackter[i]  |=0x55;
    }
  }  
  ptr+=x;
  if(flags&FL_TRANSPARENT) {/* transparent */
    for(i=0;i<16;i++) {
      d=charackter[i];
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      ptr+=ScreenWidth-CharWidth816;
    }
  } else {
    for(i=0;i<16;i++) {
      d=charackter[i];
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      ptr+=ScreenWidth-CharWidth816;
    }
  }
  if(flags&FL_BOLD) {/* bold */
    ptr  = (unsigned short*)(fbp+y*Scanline);
    ptr+=x+1;
    for(i=0;i<16;i++) {
      d=charackter[i];
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      ptr+=ScreenWidth-CharWidth816;
    }
  }
}

/* Diese Routine wird nur von den gem-Funktionen verwendet */
void mtext(int x, int y, char *t) {
  if(bigkeys) Fb_BlitText816(x,y+2,global_color, global_bcolor,t);
  else Fb_BlitText57(x,y,global_color, global_bcolor,t);
}
void mtextsmall(int x, int y, char *t) {
  Fb_BlitText57(x,y,global_color, global_bcolor,t);
}

void Fb_BlitText(int x, int y, unsigned short aColor, unsigned short aBackColor, char *str) {
  if(bigfont) Fb_BlitText816(x,y,aColor, aBackColor,str);
  else Fb_BlitText57(x,y,aColor, aBackColor,str);
}
void Fb_BlitText57(int x, int y, unsigned short aColor, unsigned short aBackColor, char *str) {
  while (*str) {
    Fb_BlitCharacter57(x, y, aColor, aBackColor, *str,0,0);
    x+=CharWidth57;
    str++;
  }
}
void Fb_BlitText816(int x, int y, unsigned short aColor, unsigned short aBackColor, char *str) {
  while (*str) {
    Fb_BlitCharacter816(x, y, aColor, aBackColor, *str,0,0);
    x+=CharWidth816;
    str++;
  }
}
