/* ltext.c (c) Markus Hoffmann
   Fontdaten einlesen fuer Linienfont char font[zeile][spalte] */

/* This file is part of X11BASIC, the basic interpreter for Unix/X
 * ============================================================
 * X11BASIC is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

#include <math.h>
#include <string.h>

#define FALSE 0
#define TRUE -1
#define rad(x)   (3.141592654*x/180)

extern const unsigned char font[128][35];
extern void line(int x1,int y1,int x2, int y2);

/* Version with no floatingpoint instructions 
s in percent, t in percent, wk in degrees, return in pixels*/

int ltext(int x, int y, int s, int t, double wk, int pflg, char *tt)  {
  int obxx,bxx=0,msin,mcos;
  unsigned int a;
  int i,j,len,len2,fx,fy,charw,center,px,py,pxo,pyo;
  int xx,yy,ox,oy;
   
  len=strlen(tt);
  if(len>0) {
#if 0
    msin=sin(rad(wk));
    mcos=cos(rad(wk)); 
#else
    msin=0;
    mcos=1;
#endif
    for(i=0;i<len;i++) {
      a=tt[i];
      if(a=='ß') a=16;
      else if(a=='Ä') a=17;
      else if(a=='ä') a=18;
      else if(a=='Ö') a=19;
      else if(a=='ö') a=20;
      else if(a=='Ü') a=21;
      else if(a=='ü') a=22;
      else a&=0x7f;
      fy=bxx*msin+y;
      fx=bxx*mcos+x;
      len2=strlen(font[a]);
      
      charw=(int)font[a][0];
      
      if(pflg==FALSE) {
        center=(100-charw)/2;
        charw=100;
      } else center=0;
      if(len2>1) {
        for(j=1;j<len2;j++) {
          xx=font[a][j++];
          yy=font[a][j];


          if(xx>100) xx=xx-101+center;
          else {
            xx+=center;
            if (j>2) {
              pxo=ox*s/100*mcos-oy*t/100*msin;
              pyo=oy*t/100*mcos+ox*s/100*msin;
              px=xx*s/100*mcos-yy*t/100*msin;
              py=yy*t/100*mcos+xx*s/100*msin;
	      line(pxo+fx,pyo+fy,px+fx,py+fy); 
            }
          }
          ox=xx; oy=yy;
        }
      }
      obxx=bxx;
      bxx=bxx+(charw+30)*s/100;
    }
  }
  return(bxx);
}

int ltextlen (int s, int pflg, char *tt) {
  int a,i,len,len2,charw=100,bxx=0;
  
  len=strlen(tt);
  if(len>0) {
    for(i=0;i<len;i++) {
      a=(int)tt[i];
      if(a=='ß') a=16;
      else if(a=='Ä') a=17;
      else if(a=='ä') a=18;
      else if(a=='Ö') a=19;
      else if(a=='ö') a=20;
      else if(a=='Ü') a=21;
      else if(a=='ü') a=22;
      else a&=0x7f;
      len2=strlen(font[a]);
      if(len2>1) { 
        if(pflg) charw=(int)font[a][0];
      } 
      bxx+=(charw+30);
    }
  }
  return(bxx*s/100);
}
