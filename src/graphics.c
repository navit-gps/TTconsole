
/* graphics.c                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <linux/fb.h>

#include "screen.h"
#include "graphics.h"
#undef WHITE
#undef BLACK
#undef RED
#undef GREEN
#undef BLUE
#undef MAGENTA
#undef YELLOW

#define WHITE 0
#define BLACK 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define CYAN 5
#define YELLOW 6
#define MAGENTA 7

extern int bigkeys;
#define chw (bigkeys?CharWidth816:CharWidth57)
#define chh (bigkeys?CharHeight816:CharHeight57)


const struct { unsigned short r,g,b;} gem_colordefs[]={
{65535,65535,65535},  /* WHITE */
{0,0,0},/*BLACK  */
{65535,0,0},/* RED */
{0,65535,0},/* GREEN */
{0,0,65535},/* BLUE */
{0,65535,65535},/* CYAN */
{65535,65535,0},/* YELLOW */
{65535,0,65535},/* MAGENTA */
{40000,40000,40000},/* LWHITE */
{20000,20000,20000},/* LBLACK */
{65535,32000,32000},/* LRED */
{32000,65535,32000},/* LGREEN */
{32000,32000,65535},/* LBLUE */
{32000,65535,65535},/* LCYAN  */
{65535,65535,32000},/* LYELLOW */
{65535,32000,65535},/* LMAGENTA */
};

int gem_colors[16];
int border;
ARECT sbox;

OBJECT menu_objects[4]={
/* 0*/  {-1,1,3,G_BOX, NONE, OUTLINED, 0x00021100, 0,0,20,7},
/* 1*/  {2,-1,-1,G_BUTTON, SELECTABLE|DEFAULT|EXIT,NORMAL ,(LONG)"OK", 15,4,4,2},
/* 2*/  {3,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"CANCEL",    15,6,4,2},
/* 3*/  {0,-1,-1,G_STRING,NONE|LASTOB, NORMAL, (LONG)"Settings:", 2,3,2,1}
};




int get_color(int r, int g, int b) {
  return((((r>>11)&0x1f)<<11)|(((g>>10)&0x3f)<<5)|(((b>>11)&0x1f)));
}
void gem_init() {
  int i;
  /* Screendimensionen bestimmem */
  sbox.x=0;
  sbox.y=8;
  sbox.w=vinfo.xres-sbox.x;
  sbox.h=vinfo.yres-sbox.y;
  border=1;

   for(i=0;i<16;i++)
     gem_colors[i]=get_color(gem_colordefs[i].r,gem_colordefs[i].g,gem_colordefs[i].b);
}


int objc_draw( OBJECT *tree,int start, int stop,int rootx,int rooty) {
  int idx=start;
  draw_object(tree,idx,rootx,rooty);
  if(tree[idx].ob_flags & LASTOB) return(1);
  if(idx==stop) return(1);
  if(tree[idx].ob_head!=-1) {
    if(!(tree[idx].ob_flags & HIDETREE)) {
      objc_draw(tree,tree[idx].ob_head,tree[idx].ob_tail,tree[idx].ob_x+rootx,tree[idx].ob_y+rooty);
    }
  }
  while(tree[idx].ob_next!=-1) {
    idx=tree[idx].ob_next;
    draw_object(tree,idx,rootx,rooty);
    if(tree[idx].ob_flags & LASTOB) return(1);
    if(tree[idx].ob_head!=-1) {
      if(!(tree[idx].ob_flags & HIDETREE)) objc_draw(tree,tree[idx].ob_head,tree[idx].ob_tail,tree[idx].ob_x+rootx,tree[idx].ob_y+rooty );
    }
    if(idx==stop) return(1);
  }
  return(0);
}

void  objc_add(OBJECT *tree,int p,int c) {
  if(tree[p].ob_tail<0) {
    tree[p].ob_head = c;
    tree[p].ob_tail = c;
    tree[c].ob_next = p;
  } else {
    tree[c].ob_next = p;
    tree[tree[p].ob_tail].ob_next = c;
    tree[p].ob_tail = c;
  }
}

void objc_delete(OBJECT *tree,int object) {
  int i=0;
  int prev=-1;
  int next=tree[object].ob_next;	
  if(next!=-1) {
    if(tree[next].ob_tail==object) next=-1;
  }
  while(1) {	
    if((tree[i].ob_next==object) && (tree[object].ob_tail!=i)) {
      prev=i;
      tree[i].ob_next=tree[object].ob_next;
      break;
    }
    if(tree[i].ob_flags & LASTOB) break;
    i++;
  }
  i=0;
  while(1) {	
    if(tree[i].ob_head==object) tree[i].ob_head=next;
    if(tree[i].ob_tail == object) tree[i].ob_tail=prev;
    if(tree[i].ob_flags & LASTOB) break;
    i++;
  }
}
/* *****************************  */
/*     objc_offset                  */

int objc_offset(OBJECT *tree,int object,int *x,int *y) {
  if((tree == NULL)) return(0);
  *x=*y=0;
  while(1) {
    int last;	
    *x+=tree[object].ob_x;
    *y+=tree[object].ob_y;
    if((tree[object].ob_next<0) || (object==0)) break;		
    do {
      last=object;
      object=tree[object].ob_next;
    } while(last!=tree[object].ob_tail);	
  }
  if(object==0) return 1;
  else return 0;
}
/* *****************************  */
/*     objc_find                  */

int objc_find(OBJECT *tree,int x,int y) {
  int sbut=-1;
  int idx=0;
  int stop=-1;
  int flag=0;
  int rootx=0;
  int rooty=0;
  while(1) {
    if(x>=tree[idx].ob_x+rootx && x<tree[idx].ob_x+tree[idx].ob_width+rootx &&
    y>=tree[idx].ob_y+rooty && y<tree[idx].ob_y+tree[idx].ob_height+rooty) {
      sbut=idx;
      if(tree[idx].ob_head!=-1) {
        if(!(tree[idx].ob_flags & HIDETREE)) {
          stop=tree[idx].ob_tail;
          rootx+=tree[idx].ob_x;
          rooty+=tree[idx].ob_y;
          idx=tree[idx].ob_head;
          flag=1;
        }
      }
    }
    if(flag) flag=0;
    else {
      if(tree[idx].ob_flags & LASTOB) return(sbut);
      if(idx==stop) return(sbut);
      if(tree[idx].ob_next!=-1) idx=tree[idx].ob_next;
      else return(sbut);
    }
  }
}


int draw_object(OBJECT *tree,int idx,int rootx,int rooty) {
  signed char randdicke=0;
  char zeichen,opaque=0;
  int fillcolor=BLACK,pattern=9;
  int textcolor=BLACK,textmode,framecolor=BLACK;
  int i,drawbg=1;
  int obx=tree[idx].ob_x+rootx;
  int oby=tree[idx].ob_y+rooty;
  int obw=tree[idx].ob_width;
  int obh=tree[idx].ob_height;

#ifdef DEBUG
  printf("Drawobjc: %d   head=%d  next=%d tail=%d\n",idx,tree[idx].ob_head,
  tree[idx].ob_next, tree[idx].ob_tail);
#endif
  switch(LOBYTE(tree[idx].ob_type)) {
  case G_BOX:
  case G_BOXCHAR:
    zeichen=(tree[idx].ob_spec & 0xff000000)>>24;
    randdicke=(tree[idx].ob_spec & 0xff0000)>>16;
    fillcolor=(tree[idx].ob_spec & 0xf);
    textcolor=(tree[idx].ob_spec & 0xf00)>>8;
    textmode=(tree[idx].ob_spec & 0x80)>>7;
    framecolor=(tree[idx].ob_spec & 0xf000)>>12;
    pattern=(tree[idx].ob_spec & 0x70)>>4;
    break;

  case G_IBOX:
    zeichen=(tree[idx].ob_spec & 0xff000000)>>24;
    randdicke=0;
    fillcolor=(tree[idx].ob_spec & 0xf);
    textcolor=(tree[idx].ob_spec & 0xf00)>>8;
    textmode=(tree[idx].ob_spec & 0x80)>>7;
    framecolor=(tree[idx].ob_spec & 0xf000)>>12;
    pattern=(tree[idx].ob_spec & 0x70)>>4;
    break;

  case G_TEXT:
  case G_FTEXT:
    framecolor=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>12) & 0xf;
    textcolor=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>8) & 0xf;
    opaque=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>7) & 1;
    pattern=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>4) & 7;
    fillcolor=(((TEDINFO *)((int)tree[idx].ob_spec))->te_color) & 0xf;
    randdicke=0;
    break;
  case G_STRING:
  case G_STRINGSMALL:
  case G_TITLE:
    randdicke=0;
    if(tree[idx].ob_state & SELECTED) {
       fillcolor=WHITE;
       pattern=9;
     } else {
       drawbg=0;
     }
    break;
  case G_BUTTON:
  case G_BUTTONSMALL:
    randdicke=-1;
    fillcolor=WHITE;
    pattern=9;
    break;
  case G_BOXTEXT:
  case G_FBOXTEXT:
    framecolor=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>12) & 0xf;
    textcolor=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>8) & 0xf;
    opaque=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>7) & 1;
    pattern=((((TEDINFO *)((int)tree[idx].ob_spec))->te_color)>>4) & 7;
    fillcolor=(((TEDINFO *)((int)tree[idx].ob_spec))->te_color) & 0xf;
    randdicke=((TEDINFO *)((int)tree[idx].ob_spec))->te_thickness;
    break;
  default:
    drawbg=0;
    break;
  }

  if(tree[idx].ob_state & SELECTED) {
    fillcolor=fillcolor^1;
    textcolor=textcolor^1;
  }
  if(tree[idx].ob_flags & EXIT) randdicke--;
  if(tree[idx].ob_flags & DEFAULT) randdicke--;

  if (drawbg) {
    /* Zeichnen  */
    if(tree[idx].ob_state & OUTLINED) {
      SetForeground(gem_colors[WHITE]);
      FillRectangle(obx-3,oby-3,obw+6,obh+6);
      SetForeground(gem_colors[framecolor]);
      DrawRectangle(obx-3,oby-3,obw+6,obh+6);
    }
    if(tree[idx].ob_state & SHADOWED) {
      SetForeground(gem_colors[BLACK]);
      FillRectangle(obx+obw,oby+chh/2,chw,obh);
      FillRectangle(obx+chw,oby+obh,obw,chh/2);
    }


/* Hintergrund zeichnen */

    SetForeground(gem_colors[WHITE]);
    if(!opaque) {FillRectangle(obx,oby,obw,obh);}

    if(pattern) { 
      SetForeground(gem_colors[fillcolor]);
      SetFillStyle(FillStippled);
      set_fill(pattern);
      FillRectangle(obx,oby,obw,obh);
      SetFillStyle(FillSolid);
    }
  }

/* Text zeichnen   */

  if(tree[idx].ob_state & SELECTED) {
    SetForeground(gem_colors[textcolor]);
    set_bcolor(gem_colors[BLACK]);
  } else{
    SetForeground(gem_colors[textcolor]);
    set_bcolor(gem_colors[WHITE]);
  }

  if(tree[idx].ob_state & DISABLED) {SetForeground(gem_colors[LWHITE]);}
  switch(LOBYTE(tree[idx].ob_type)) {
    char *text;
    char chr[2];
    TEDINFO *ted;
    int x,y,i;
  case G_STRING:
  case G_TITLE:
    text=(char *)((int)tree[idx].ob_spec);
    DrawString(obx,oby+chh-2,text,strlen(text));
    break;
  case G_STRINGSMALL:
    text=(char *)((int)tree[idx].ob_spec);
    DrawStringSmall(obx,oby+6,text,strlen(text));
    break;
  case G_BOX:
  case G_IBOX:
    break;
  case G_BUTTON:
    text=(char *)((int)tree[idx].ob_spec);
    DrawString(obx+(signed)(obw-chw*strlen(text))/2,oby+chh-2+(obh-chh)/2,text,strlen(text));
    break;
  case G_BUTTONSMALL:
    text=(char *)((int)tree[idx].ob_spec);
    DrawStringSmall(obx+1+(signed)(obw-CharWidth57*strlen(text))/2,oby+7+(obh-CharHeight57)/2,text,strlen(text));
    break;
  case G_BOXCHAR:
    DrawString(obx+(obw-chw)/2,oby+chh-2+(obh-chh)/2,&zeichen,1);
    break;
  case G_ALERTTYP:
    chr[0]=tree[idx].ob_spec+4;
    chr[1]=0;
    SetForeground(gem_colors[RED]);
#if 0
    XSetLineAttributes(display[usewindow], gc[usewindow], 2, 0,0,0);
    ltext(obx,oby,50,50,0,0,chr);
    if(tree[idx].ob_spec==3) ltext(obx+4,oby+12,50/6,50/2,0,0,"STOP");
    XSetLineAttributes(display[usewindow], gc[usewindow], 1, 0,0,0);
#endif
    break;
  case G_TEXT:
  case G_FTEXT:
  case G_BOXTEXT:
  case G_FBOXTEXT:
    ted=(TEDINFO *)((int)tree[idx].ob_spec);
    text=(char *)(ted->te_ptext);
#if 0
    load_GEMFONT(ted->te_font);
#endif
    if(tree[idx].ob_type==G_FTEXT || tree[idx].ob_type==G_FBOXTEXT){
      if(ted->te_junk1-ted->te_junk2>ted->te_tmplen) ted->te_junk2=ted->te_junk1-ted->te_tmplen;
      if(ted->te_junk1-ted->te_junk2<0) ted->te_junk2=ted->te_junk1;
      for(i=0;i<ted->te_tmplen;i++) {
        if(i<strlen((char *)(ted->te_ptext))) ((char *)(ted->te_ptmplt))[i]=((char *)(ted->te_ptext))[i+ted->te_junk2];
	else ((char *)(ted->te_ptmplt))[i]='_';
      }
      ((char *)ted->te_ptmplt)[ted->te_tmplen]=0;
      text=(char *)(ted->te_ptmplt);
    }
    if(ted->te_just==TE_LEFT) {
      x=obx; y=oby+chh-2+(obh-chh)/2;
    } else if(ted->te_just==TE_RIGHT) {
      x=obx+obw-chw*strlen(text); y=oby+chh-2+(obh-chh)/2;
    } else {
      x=obx+(obw-chw*strlen(text))/2; y=oby+chh-2+(obh-chh)/2;
    }
    DrawString(x,y,text,strlen(text));
    SetForeground(gem_colors[RED]);
    if(strlen((char *)(ted->te_ptext))>ted->te_tmplen+ted->te_junk2)
      DrawString(obx+obw,oby+obh,">",1);
    if(ted->te_junk2)
      DrawString(obx-chw,oby+obh,"<",1);
#if 0
    load_GEMFONT(1);
#endif
    break;
  case G_IMAGE:
    {BITBLK *bit=(BITBLK *)((int)tree[idx].ob_spec);
    unsigned int adr;

    adr=*((LONG *)&(bit->bi_pdata));
    SetForeground(gem_colors[(bit->bi_color) & 0xf]);
    SetBackground(gem_colors[WHITE]);

#if 0
    put_bitmap((char *)adr,obx,oby,bit->bi_wb*8,bit->bi_hl);
#endif
   }

    break;
  case G_ICON:
    {ICONBLK *bit=(ICONBLK *)((int)tree[idx].ob_spec);
    unsigned int adr;
    adr=*(LONG *)&bit->ib_pmask;
    SetForeground(gem_colors[WHITE]);
#if 0
    put_bitmap((char *)adr,obx,oby,bit->ib_wicon,bit->ib_hicon);
#endif
    FillRectangle(obx+bit->ib_xtext,oby+bit->ib_ytext,bit->ib_wtext,bit->ib_htext);
    adr=*(LONG *)&bit->ib_pdata;
    SetBackground(gem_colors[WHITE]);
    SetForeground(gem_colors[BLACK]);
#if 0
    put_bitmap((char *)adr,obx,oby,bit->ib_wicon,bit->ib_hicon);
    /* Icon-Text */
    load_GEMFONT(FONT_SMALL);
#endif
    DrawStringSmall(obx+bit->ib_xtext,oby+bit->ib_ytext+bit->ib_htext,(char *)*(LONG *)&bit->ib_ptext,strlen((char *)*(LONG *)&bit->ib_ptext));
    /* Icon char */
#if 0
    load_GEMFONT(1);
#endif
    }
    break;
  default:
    printf("Unbekanntes Objekt #%d\n",tree[idx].ob_type);
  }

/* Rand zeichnen */
  if(randdicke) {
    SetForeground(gem_colors[framecolor]);
    if(randdicke>0) {
      for(i=0;i<randdicke;i++) DrawRectangle(obx+i,oby+i,obw-2*i,obh-2*i);
    } else if(randdicke<0) {
      for(i=0;i>randdicke;i--) DrawRectangle(obx+i,oby+i,obw-2*i,obh-2*i);
    }   
  }
  if(tree[idx].ob_state & CROSSED) {
    SetForeground(gem_colors[RED]);
    DrawLine(obx,oby,obx+obw,oby+obh);
    DrawLine(obx+obw,oby,obx,oby+obh);
  }
  return(0);
}

int finded(OBJECT *tree,int start, int r) {
    /*  editierbare Objekt finden */
  int idx=start;
  int sbut=-1;
  if(r>0 && !(tree[idx].ob_flags & LASTOB)) idx++;
  else if(r<0 && idx>0) idx--;

  while(1) {
    if(tree[idx].ob_flags & EDITABLE) {sbut=idx;break;}
    if(tree[idx].ob_flags & LASTOB) break;
    if(r<0)idx--;
    else idx++;
    if(idx<0) break;
  }
  return(sbut);
}
int rootob(OBJECT *tree,int onr) {
  int idx=onr;
  int sbut;

  while(1) {
    sbut=idx;
    idx=tree[idx].ob_next;
    if(idx==-1) return(-1);
    if(tree[idx].ob_tail==sbut) return(idx);
  }
}

void relobxy(OBJECT *tree,int ndx,int *x, int *y){
  *x=tree[ndx].ob_x;
  *y=tree[ndx].ob_y;
  while((ndx=rootob(tree,ndx))>=0){
    *x+=tree[ndx].ob_x;
    *y+=tree[ndx].ob_y;
  }
}

void draw_edcursor(OBJECT *tree,int ndx){
  TEDINFO *ted=(TEDINFO *)(tree[ndx].ob_spec);
  int x,y;

  relobxy(tree,ndx,&x,&y);
  SetForeground(gem_colors[RED]);
  DrawLine(x+chw*(ted->te_junk1-ted->te_junk2),y,x+chw*(ted->te_junk1-ted->te_junk2),
  y+chh+4);
  SetForeground(gem_colors[BLACK]);
}

int form_center(OBJECT *tree, int *x, int *y, int *w, int *h) {
  tree->ob_x=sbox.x+(sbox.w-tree->ob_width)/2;
  tree->ob_y=sbox.y+(sbox.h-tree->ob_height)/2;
  *x=tree->ob_x;
  *y=tree->ob_y;
  *w=tree->ob_width;
  *h=tree->ob_height;
  return(0);
}


