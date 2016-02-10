/* terminal.c                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <linux/fb.h>
#include "terminal.h"
#include "screen.h"
#include "consolefont.h"

int terminal_fd=-1;   /* File descriptor for connection to shell */

CINFO textscreen[80*160];

int col=0,lin=0;



void cursor_onoff(int onoff,int x,int y) {
  static int cursor=0;
  if(cursor!=onoff) Fb_inverse(x,y,CharWidth,CharHeight);
  cursor=onoff;
}
void textscreen_scroll(int target,int source,int num) {
  int i,j;
  if(target<source) {
    for(i=0;i<num;i++) {
      for(j=0;j<LineLen;j++) textscreen[(target+i)*LineLen+j]=
      textscreen[(source+i)*LineLen+j];
    }
  } else if(target>source) {
    for(i=num-1;i>=0;i--) {
      for(j=0;j<LineLen;j++) textscreen[(target+i)*LineLen+j]=textscreen[(source+i)*LineLen+j];
    }
  }
}
void textscreen_clear(int line,int num){
  int i,j;
  for(i=0;i<num;i++) {
    for(j=0;j<LineLen;j++) textscreen[(line+i)*LineLen+j].c=0;
  }
}
void textscreen_redraw(int x, int y, int w,int h){
  int i,j;
  for(i=y;i<y+h;i++) {
    for(j=x;j<x+w;j++) {
      if(textscreen[i*LineLen+j].c) Fb_BlitCharacter(j*CharWidth,i*CharHeight,
      textscreen[i*LineLen+j].color,textscreen[i*LineLen+j].bcolor, 
      textscreen[i*LineLen+j].c,textscreen[i*LineLen+j].flags);
      else Fb_BlitCharacter(j*CharWidth,i*CharHeight,
      textscreen[i*LineLen+j].color,textscreen[i*LineLen+j].bcolor,' ',0);
    }
  }
}


void textscreen_lineclear(int lin,int col,int num){
  int j;
    for(j=col;j<col+num;j++) textscreen[lin*LineLen+j].c=0;
}



/* Terminalroutines, (c) Markus Hoffmann, all rights reserved */
void g_out(char a) {
  static int chw=CharWidth,chh=CharHeight;
  static int escflag=0;
  static int numbers[16];
  static int anznumbers;
  static int tcolor=WHITE,tbcolor=BLACK,flags=0;
  static int scroll_region_1=0;
  static int scroll_region_2=30;
  static int cursor_saved_x=0;
  static int cursor_saved_y=0;
  static int cursor_saved_flags=0;
  static number;
  int bbb;
  if(escflag==1) {
    if(a=='c') {   /* Terminal reset */
      escflag=0;
      cursor_onoff(0,col*chw,lin*chh);
      col=lin=0;
      textscreen_clear(0,AnzLine);
      scroll_region_2=AnzLine;
      scroll_region_1=0;
      Fb_Clear(0,ScreenHeight,tbcolor);
      cursor_onoff(1,col*chw,lin*chh);
      tcolor=WHITE;tbcolor=BLACK;flags=0;
    } else if(a=='7') {/* Save cursor-position and attributes */
      cursor_saved_x=col;cursor_saved_y=lin;cursor_saved_flags=flags;
      escflag=0;
    } else if(a=='8') {/* Set cursor an save-pos. and attributes */
      cursor_onoff(0,col*chw,lin*chh);
      col=cursor_saved_x;
      lin=cursor_saved_y;
      flags=cursor_saved_flags;
      cursor_onoff(1,col*chw,lin*chh);
      escflag=0;
    } else if(a=='E') {/* Next Row (CR LF) */
      cursor_onoff(0,col*chw,lin*chh);
      lin++;col=0;  
      if((lin+1)*chh>ScreenHeight){
        lin--;
	textscreen_scroll(0,1,lin);
	textscreen_clear(lin,1);
	
        Fb_Scroll(0, chh,lin*chh);  
        Fb_Clear(lin*chh,chh,tbcolor);
      } 
      cursor_onoff(1,col*chw,lin*chh);
      escflag=0;
    } else if(a=='M') {/*  Cursor up - at top of region, scroll down */
      cursor_onoff(0,col*chw,lin*chh);
      if(lin) lin--;
      else {
       	textscreen_scroll(1,0,AnzLine-1);
	textscreen_clear(0,1);
        Fb_Scroll(chh,0,(AnzLine-1)*chh);  
        Fb_Clear(0,chh,tbcolor);
      }
      cursor_onoff(1,col*chw,lin*chh);
      escflag=0;  
    } else if(a=='D') {/* Cursor down - at bottom of region, scroll up */
       cursor_onoff(0,col*chw,lin*chh);
      lin++;  
      if((lin+1)*chh>ScreenHeight){
        lin--;
	textscreen_scroll(0,1,lin);
	textscreen_clear(lin,1);
	
        Fb_Scroll(0, chh,lin*chh);  
        Fb_Clear(lin*chh,chh,tbcolor);
      } 
      cursor_onoff(1,col*chw,lin*chh);
      escflag=0;
    } else if(a=='[') {/* more parameters follow */
      escflag=2;
      number=anznumbers=0;
    } else {
      int ocolor=tcolor;
        Fb_BlitText(0,0,RED,BLACK,"TTconsole-terminal-emulation: Received ESC-");
        Fb_BlitCharacter(0,10,RED,BLACK,a,0);
      tcolor=RED;
      escflag=0;
      g_out(a);
      tcolor=ocolor;
    }
  } else if(escflag==2) {
    if(a==';') {
      if(anznumbers<15) numbers[anznumbers++]=number;
      number=0;
    } else if(a>='0' && a<='9') {
      number=number*10+(a-'0');
    } else {
      escflag=0;
      if(anznumbers<15)  numbers[anznumbers++]=number;
      if(a=='m') {
	if(anznumbers==1 && numbers[0]==0) {flags=0;tcolor=WHITE;tbcolor=BLACK;}
	else {
	  int i,f;
	  for(i=0;i<=anznumbers;i++) {
	    f=numbers[i];
	    if(f<10) flags|=(1<<f);
	    else if(f>20 && f<30) flags&=~(1<<(f-20));
	    else if(f==30) tcolor=WHITE;
	    else if(f==31) tcolor=RED;
	    else if(f==32) tcolor=GREEN;
	    else if(f==33) tcolor=YELLOW;
	    else if(f==34) tcolor=BLUE;
	    else if(f==35) tcolor=MAGENTA;
	    else if(f==36) tcolor=LIGHTBLUE;
	    else if(f==37) tcolor=BLACK;
	    else if(f==40) tbcolor=WHITE;
	    else if(f==41) tbcolor=RED;
	    else if(f==42) tbcolor=GREEN;
	    else if(f==43) tbcolor=YELLOW;
	    else if(f==44) tbcolor=BLUE;
	    else if(f==45) tbcolor=MAGENTA;
	    else if(f==46) tbcolor=LIGHTBLUE;
	    else if(f==47) tbcolor=BLACK;
	  }
	}
	
      } else if(a=='h') {
        
      } else if(a=='l') {
      
      } else if(a=='A') { /* cursor up pn times - stop at top */
        cursor_onoff(0,col*chw,lin*chh);
	lin=max(0,lin-max(numbers[0],1));
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='B') { /* cursor down pn times - stop at bottom */
        cursor_onoff(0,col*chw,lin*chh);
	lin=min(ScreenHeight/CharHeight-1,lin+max(numbers[0],1));
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='C') { /* cursor right pn times - stop at far right */
        cursor_onoff(0,col*chw,lin*chh);
	col=min(ScreenWidth/CharWidth-1,col+max(numbers[0],1));
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='D') { /* cursor left pn times - stop at far left */
        cursor_onoff(0,col*chw,lin*chh);
	col=max(0,col-max(numbers[0],1));
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='P') { /* delete pn characters  */
        int i=min(max(numbers[0],1),ScreenWidth/CharWidth);
        cursor_onoff(0,col*chw,lin*chh);
	/* Textscreen move_chars */
        memmove(&textscreen[lin*LineLen+col],&textscreen[lin*LineLen+col+i],(LineLen-(col+i))*sizeof(CINFO));
	copyarea((col+i)*chw,lin*chh,ScreenWidth-(col+i)*chw,chh,col*chw,lin*chh);
	textscreen_lineclear(lin,LineLen-i,i);
	FillBox((LineLen-i)*chw,lin*chh,chw*i,chh,tbcolor);
	cursor_onoff(1,col*chw,lin*chh);	
      } else if(a=='L') { /* insert pn characters(?)  */
        int i=min(max(numbers[0],1),ScreenWidth/CharWidth);
        cursor_onoff(0,col*chw,lin*chh);
	/* Textscreen move_chars */
        memmove(&textscreen[lin*LineLen+col+i],&textscreen[lin*LineLen+col],(LineLen-(col+i))*sizeof(CINFO));
	copyarea(col*chw,lin*chh,ScreenWidth-(col+i)*chw,chh,(col+i)*chw,lin*chh);
	textscreen_lineclear(lin,col,i);
	FillBox(col*chw,lin*chh,chw*i,chh,tbcolor);
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='M') {                               /* delete pn lines  */
        int i=min(max(numbers[0],1),ScreenHeight/CharHeight-lin);
        cursor_onoff(0,col*chw,lin*chh);
	textscreen_scroll(lin,lin+i,ScreenHeight/CharHeight-lin-i);
	Fb_Scroll(lin*chh,(lin+i)*chh,(ScreenHeight/CharHeight-lin-i)*chh);
	Fb_Clear((ScreenHeight/CharHeight-1)*chh,chh,tbcolor);
	textscreen_clear(ScreenHeight/CharHeight-1,1);
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='s') { /* Speichern der Cursorposition */
        cursor_saved_x=col;
        cursor_saved_y=lin;	
      } else if(a=='u') { /* Cursor auf gespeicherte Position setzen */
        cursor_onoff(0,col*chw,lin*chh);
        col=cursor_saved_x;
        lin=cursor_saved_y;
	cursor_onoff(1,col*chw,lin*chh);		
      } else if(a=='r') { /* set scroll region */
	  int i;
	  for(i=0;i<=anznumbers;i++) {
	    if(i==0) scroll_region_1=numbers[i];
	    if(i==1) scroll_region_2=numbers[i];
	  }
      } else if(a=='H' ||a=='f' ) {
        cursor_onoff(0,col*chw,lin*chh);
	if(anznumbers==1 && numbers[0]==0) {col=lin=0;}
	else {
	  int i;
	  for(i=0;i<=anznumbers;i++) {
	    if(i==0) lin=max(0,numbers[i]-1);
	    if(i==1) col=max(0,numbers[i]-1);
	  }
	}
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='J') {
        cursor_onoff(0,col*chw,lin*chh);
        if(numbers[0]==0) {
	  textscreen_clear(lin+1,AnzLine-lin-1);
	  Fb_Clear((lin+1)*chh,ScreenHeight-(lin+1)*chh,tbcolor);	
	} else if(numbers[0]==1) {
	  textscreen_clear(0,lin);
	  Fb_Clear(0,lin*chh,tbcolor);	
	} else if(numbers[0]==2) {
	  textscreen_clear(0,ScreenHeight/CharHeight);
	  Fb_Clear(0,ScreenHeight,tbcolor);
	}
	cursor_onoff(1,col*chw,lin*chh);
      } else if(a=='K') {
        cursor_onoff(0,col*chw,lin*chh);
        if(numbers[0]==0) {
	  textscreen_lineclear(lin,col,ScreenWidth/CharWidth-col);
	  FillBox(col*chw,lin*chh,ScreenWidth-chw*col,chh,tbcolor);	
	} else if(numbers[0]==1) {
	  textscreen_lineclear(lin,0,col);
	  FillBox(0,lin*chh,chw*col,chh,tbcolor);	
	} else if(numbers[0]==2) {
	  textscreen_lineclear(lin,0,ScreenWidth/CharWidth);
	  Fb_Clear(lin*chh,(lin+1)*chh,tbcolor);
	}
	cursor_onoff(1,col*chw,lin*chh);
      } else {
        int ocolor=tcolor;
        Fb_BlitText(0,0,RED,BLACK,"TTconsole-terminal-emulation: Received ESC-[-x-");
        Fb_BlitCharacter(0,10,RED,BLACK,a,0);
        tcolor=MAGENTA;
        g_out(a);
        tcolor=ocolor;  
      }
    }
  } else {
    cursor_onoff(0,col*chw,lin*chh);
    switch(a) {
    case 0: break;
    case 7: printf("\007");break;
    case 8: if(col) col--; break;
    case 9: col=min(((col+8)>>3)<<3,ScreenWidth/CharWidth-1); break;
    case 10: 
    case 11:
    case 12: 
      lin++;col=0;
      if((lin+1)*chh>ScreenHeight){
        lin--;
	textscreen_scroll(0,1,lin);
	textscreen_clear(lin,1);
	
        Fb_Scroll(0, chh,lin*chh);  
        Fb_Clear(lin*chh,chh,tbcolor);
      } 
      break;
    case 13: col=0; break;
    case 27: escflag=1;break; 
    case 127: break; 
    default:
      Fb_BlitCharacter(col*chw,lin*chh,tcolor,tbcolor, a,flags);
      
      textscreen[lin*LineLen+col].c=a;
      textscreen[lin*LineLen+col].color=tcolor;
      textscreen[lin*LineLen+col].bcolor=tbcolor;
      textscreen[lin*LineLen+col].flags=flags;
      col++;
      if(col*chw>=ScreenWidth) {
        col=0; lin++;
        if((lin+1)*chh>ScreenHeight) {
          lin--;
  	  textscreen_scroll(0,1,lin);
	  textscreen_clear(lin,1);
          Fb_Scroll(0, chh,lin*chh);  
          Fb_Clear(lin*chh,chh,tbcolor);
        } 
      }
    } 
    cursor_onoff(1,col*chw,lin*chh);
  }
}

void g_outs(char *t){
  int i;
  if(t && strlen(t)) {
    for(i=0;i<strlen(t);i++) g_out(t[i]);
  }
}
