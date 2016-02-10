
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

int chw=5,chh=8,baseline=8,depth=16;
int winbesetzt[MAXWINDOWS];
int usewindow;

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


int get_color(int r, int g, int b) {
  return((((r>>12)&0xf)<<12)|(((g>>12)&0xf)<<8)|(((b>>12)&0xf)<<4));
}
void gem_init() {
  int i;
  printf("gem_init...");
  /* Screendimensionen bestimmem */
  sbox.x=0;
  sbox.y=8;
  sbox.w=vinfo.xres-sbox.x;
  sbox.h=vinfo.yres-sbox.y;
  border=1;
  depth=vinfo.bits_per_pixel;

   for(i=0;i<16;i++)
     gem_colors[i]=get_color(gem_colordefs[i].r,gem_colordefs[i].g,gem_colordefs[i].b);
   printf("done.\n");
}

int ltext(int, int,int,int,int, int, char *);

int form_alert(int dbut,char *n) {
  return(form_alert2(dbut,n,NULL));
}
int form_alert2(int dbut,char *n, char *tval) {
  char *bzeilen[30],*bbuttons[30],*buffer[30];
  int anzzeilen=0,anzbuttons=0,anztedinfo=0,anzbuffer=0;
  int symbol,sbut=0;
  char *pos;
  char **ein=bzeilen;
  int i=0,j=strlen(n),k=0,l=0;
  TEDINFO tedinfo[32];
  OBJECT objects[64]={{-1,1,1,G_BOX, 0, OUTLINED, 0x00021100, 0,0,100,100}};
  int objccount=1;
  int x,y,w,h;
#ifdef DEBUG
  printf("**form_alert:\n");
#endif

  while(i<j) {
    if(n[i]=='[') {
      pos=&n[i+1];
      k++;l=0;
    } else if(n[i]==']') {
      n[i]=0;
      if(k>0) ein[l++]=pos;
      if(k==1) {
        symbol=atoi(pos);
	ein=bzeilen;
      } else if(k==2) {
        ein=bbuttons;anzzeilen=l;
      } else if(k==3) anzbuttons=l;
    }
    else if(n[i]=='|') {ein[l]=pos;n[i]=0;pos=n+i+1;l++;};
    i++;
  }

  if(anzbuttons) {
    /* Box-Abmessungen bestimmen */
    int textx;
    int maxc=0;

    graphics();
    gem_init();

    if(symbol) {objects[0].ob_width=objects[0].ob_height=textx=64;}
    else {objects[0].ob_width=objects[0].ob_height=textx=0;}

     /*Raender*/
    objects[0].ob_width+=chh*2;
    objects[0].ob_height=max(objects[0].ob_height+2*chh,chh*2+(anzzeilen+2)*chh);


    for(i=0;i<anzzeilen;i++) maxc=max(maxc,strlen(bzeilen[i]));
    objects[0].ob_width+=chw*maxc;

     /* Buttons  */
    maxc=0;
    for(i=0;i<anzbuttons;i++) maxc=max(maxc,strlen(bbuttons[i]));
    objects[0].ob_width=max(objects[0].ob_width,chw*(4+(maxc+1)*anzbuttons+2*(anzbuttons-1)));

    for(i=0;i<anzbuttons; i++) {
      objects[objccount].ob_x=objects[0].ob_width/2-chw*((maxc+1)*anzbuttons+2*(anzbuttons-1))/2+i*chw*(maxc+3);
      objects[objccount].ob_y=objects[0].ob_height-2*chh;
      objects[objccount].ob_width=(chw+1)*maxc;
      objects[objccount].ob_height=chh+3;
      objects[objccount].ob_spec=(LONG)bbuttons[i];
      objects[objccount].ob_head=-1;
      objects[objccount].ob_tail=-1;
      objects[objccount].ob_next=objccount+1;
      objects[objccount].ob_type=G_BUTTON;
      objects[objccount].ob_flags=SELECTABLE|EXIT;
      objects[objccount].ob_state=NORMAL;
      objccount++;
    }

    if(dbut>0 && dbut<=anzbuttons) {
      objects[dbut].ob_flags|=DEFAULT;
    }

    for(i=0;i<anzzeilen;i++) {
      objects[objccount].ob_x=textx+chh;
      objects[objccount].ob_y=(1+i)*chh;
      objects[objccount].ob_width=chw*strlen(bzeilen[i]);
      objects[objccount].ob_height=chh;
      objects[objccount].ob_spec=(LONG)bzeilen[i];
      objects[objccount].ob_head=-1;
      objects[objccount].ob_tail=-1;
      objects[objccount].ob_next=objccount+1;
      objects[objccount].ob_type=G_STRING;
      objects[objccount].ob_flags=NONE;
      objects[objccount].ob_state=NORMAL;
      objccount++;
      /* Input-Felder finden */
      if(strlen((char *)objects[objccount-1].ob_spec)) {
        for(j=0;j<strlen((char *)objects[objccount-1].ob_spec);j++) {
	  if(((char *)(objects[objccount-1].ob_spec))[j]==27) {
            ((char *)(objects[objccount-1].ob_spec))[j]=0;
	    objects[objccount].ob_x=textx+chh+chw*j+chw;
            objects[objccount].ob_y=(1+i)*chh;
            objects[objccount-1].ob_width=chw*(strlen((char *)objects[objccount-1].ob_spec));
            objects[objccount].ob_width=chw*(strlen(bzeilen[i]+j+1));
            objects[objccount].ob_height=chh;
            objects[objccount].ob_spec=(LONG)&tedinfo[anztedinfo];
            objects[objccount].ob_head=-1;
            objects[objccount].ob_tail=-1;
            objects[objccount].ob_next=objccount+1;
            objects[objccount].ob_type=G_FTEXT;
            objects[objccount].ob_flags=EDITABLE;
            objects[objccount].ob_state=NORMAL;
	    tedinfo[anztedinfo].te_ptext=(LONG)(bzeilen[i]+j+1);
	    buffer[anzbuffer]=malloc(strlen((char *)tedinfo[anztedinfo].te_ptext)+1);
	    tedinfo[anztedinfo].te_ptmplt=(LONG)(buffer[anzbuffer++]);
	    tedinfo[anztedinfo].te_pvalid=(LONG)(bzeilen[i]+j+1);
	    tedinfo[anztedinfo].te_font=FONT_IBM;
	    tedinfo[anztedinfo].te_just=TE_LEFT;
	    tedinfo[anztedinfo].te_junk1=strlen((char *)tedinfo[anztedinfo].te_ptext);
	    tedinfo[anztedinfo].te_junk2=0;
	    tedinfo[anztedinfo].te_color=0x1100;
	    tedinfo[anztedinfo].te_thickness=1;
	    tedinfo[anztedinfo].te_txtlen=strlen((char *)tedinfo[anztedinfo].te_ptext);
	    tedinfo[anztedinfo].te_tmplen=strlen((char *)tedinfo[anztedinfo].te_ptext);
	    anztedinfo++;
            objccount++;
	  /*  printf("Objcount: %d\n",objccount);*/
	  }
	}
      }
    }
    if(symbol>=1) {
      objects[objccount].ob_x=2*chw;
      objects[objccount].ob_y=chw;
      objects[objccount].ob_width=50;
      objects[objccount].ob_height=50;
      objects[objccount].ob_spec=symbol;
      objects[objccount].ob_head=-1;
      objects[objccount].ob_tail=-1;
      objects[objccount].ob_next=objccount+1;
      objects[objccount].ob_type=G_ALERTTYP;
      objects[objccount].ob_flags=NONE;
      objects[objccount].ob_state=NORMAL;
      objccount++;

    }
    objects[objccount-1].ob_next=0;
    objects[0].ob_tail=objccount-1;
    objects[objccount-1].ob_flags|=LASTOB;

      /* Objektbaum Zentrieren */
    form_center(objects, &x,&y,&w,&h);


  /* Erst den Graphic-Kontext retten  */

    form_dial(0,0,0,0,0,x,y,w,h);
    form_dial(1,0,0,0,0,x,y,w,h);
    objc_draw(objects,0,-1,0,0);
    sbut=form_do(objects);

    form_dial(3,0,0,0,0,x,y,w,h);
    form_dial(2,0,0,0,0,x,y,w,h);

    if(tval!=NULL) { /* Textfelder zusammensuchen  */

      tval[0]=0;
      for(i=0;i<objccount;i++) {

        if(objects[i].ob_flags & EDITABLE) {
	  strcat(tval,(char *)((TEDINFO *)objects[i].ob_spec)->te_ptext);
	  tval[strlen(tval)+1]=0;
	  tval[strlen(tval)]=13;
	}
      }
    }
    while(anzbuffer) {
      free(buffer[--anzbuffer]);
    }
  }
  return(sbut);
}

int objc_draw( OBJECT *tree,int start, int stop,int rootx,int rooty) {
  int idx=start;
#if DEBUG
  char buffer[100];
  int x,y,pen;
  sprintf(buffer,"**objc_draw: von %d bis %d\n",start,stop);
  g_outs(buffer);
    while (TsScreen_pen(&x,&y,&pen));
    while (!TsScreen_pen(&x,&y,&pen)) ; 
    while (TsScreen_pen(&x,&y,&pen));  
#endif
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
  int i=0;
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
void activate() {
#if 0
   Window root;
   int ox,oy,ow,oh,ob,d;
   XGCValues gc_val;
   int of;
#endif
   graphics();

#if 0
   XGetGeometry(display[usewindow],win[usewindow],&root,&ox,&oy,&ow,&oh,&ob,&d);
   XGetGCValues(display[usewindow], gc[usewindow],GCFunction , &gc_val);
   of=gc_val.function;
   gc_val.function=GXcopy;

   XChangeGC(display[usewindow], gc[usewindow],  GCFunction, &gc_val);

   XFlush(display[usewindow]);
   XCopyArea(display[usewindow],pix[usewindow],win[usewindow],gc[usewindow],0,0,ow,oh,0,0);
   handle_window(usewindow);
   gc_val.function=of;
   XChangeGC(display[usewindow], gc[usewindow],  GCFunction, &gc_val);
#endif
}


int draw_object(OBJECT *tree,int idx,int rootx,int rooty) {
  signed char randdicke=0;
  char zeichen,opaque=0;
  int fillcolor=BLACK,pattern=9;
  int textcolor=BLACK,textmode,framecolor=BLACK;
  int i,drawbg=1,drawtext=1;
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

  SetForeground(gem_colors[textcolor]);
  set_bcolor(gem_colors[WHITE]);

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
  case G_BOX:
  case G_IBOX:
    break;
  case G_BUTTON:
    text=(char *)((int)tree[idx].ob_spec);
    DrawString(obx+(obw-chw*strlen(text))/2,oby+chh-2+(obh-chh)/2,text,strlen(text));
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
#endif
    ltext(obx,oby,50,50,0,0,chr);
    if(tree[idx].ob_spec==3) ltext(obx+4,oby+12,50/6,50/2,0,0,"STOP");
#if 0
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
    DrawString(obx+bit->ib_xtext,oby+bit->ib_ytext+bit->ib_htext,(char *)*(LONG *)&bit->ib_ptext,strlen((char *)*(LONG *)&bit->ib_ptext));
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

int relobxy(OBJECT *tree,int ndx,int *x, int *y){
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

int form_dial( int fo_diflag, int x1,int y1, int w1, int h1, int x2, int y2, int w2, int h2 ) {
  static sgccount=0;
#if 0
  static GC *sgc[30];
  static Pixmap *spix[30];
  XGCValues gc_val;
  GC pgc;
  Pixmap ppix;
#endif
#ifdef DEBUG
  printf("**form_dial:\n");
#endif

  switch(fo_diflag){
  case 0:
#if 0
  /* Erst den Graphic-Kontext retten  */
    sgc[sgccount]=malloc(sizeof(GC));
    pgc=XCreateGC(display[usewindow], win[usewindow], 0, &gc_val);

    XCopyGC(display[usewindow], gc[usewindow],GCForeground|
                              GCFunction |GCLineWidth |GCLineStyle|
			      GCFont, pgc);
    memcpy(sgc[sgccount],&pgc,sizeof(GC));		

    gc_val.function=GXcopy;
    XChangeGC(display[usewindow], gc[usewindow],  GCFunction, &gc_val);

    /* Hintergrund retten  */
    ppix=XCreatePixmap(display[usewindow],win[usewindow],w2+8,h2+8,depth);
    XCopyArea(display[usewindow], pix[usewindow],ppix,gc[usewindow],x2-3,y2-3,w2+8,h2+8,0,0);
    spix[sgccount]=malloc(sizeof(Pixmap));
    memcpy(spix[sgccount],&ppix,sizeof(Pixmap));
#endif
    sgccount++;
   break;
   case 3:
   /* Hintergrund restaurieren  */
   sgccount--;
#if 0
    XCopyArea(display[usewindow], *(spix[sgccount]),pix[usewindow],gc[usewindow],0,0,w2+8,h2+8,x2-3,y2-3);
    XFreePixmap(display[usewindow],*(spix[sgccount]));
    XCopyGC(display[usewindow],*sgc[sgccount],GCForeground| GCFunction |GCLineWidth |GCLineStyle| GCFont, gc[usewindow]);
    XFreeGC(display[usewindow],*sgc[sgccount]);
    free(sgc[sgccount]);free(spix[sgccount]);
#endif
    activate();
    break;
  default:
    return(-1);
  }
}

int form_center(OBJECT *tree, int *x, int *y, int *w, int *h) {
  /* Objektbaum Zentrieren */
#ifdef DEBUG
  printf("**form_center:\n");
#endif

  tree->ob_x=sbox.x+(sbox.w-tree->ob_width)/2;
  tree->ob_y=sbox.y+(sbox.h-tree->ob_height)/2;
  *x=tree->ob_x;
  *y=tree->ob_y;
  *w=tree->ob_width;
  *h=tree->ob_height;
  return(0);
}
int form_do(OBJECT *tree) {
#if 0
  XEvent event;
  XGCValues gc_val;
#endif
  int exitf=0,bpress=0;
  int sbut,edob=-1,idx;
  int x,y,w,h;
#ifdef DEBUG
  printf("**form_do:\n");
#endif


    /* erstes editierbare Objekt finden */

  edob=finded(tree,0,0);
 /* objc_draw(tree,0,-1,0,0); */
	
  /* Cursor plazieren */
	
  if(edob>=0) {
    ((TEDINFO *)tree[edob].ob_spec)->te_junk1=strlen((char *)((TEDINFO *)tree[edob].ob_spec)->te_ptext);
    draw_edcursor(tree,edob);
  }	

  /* Auf Tasten/Maus reagieren */
  activate();
  while(exitf==0) {
#if 0
    XWindowEvent(display[usewindow], win[usewindow],KeyPressMask |KeyReleaseMask|ExposureMask |ButtonReleaseMask| ButtonPressMask, &event);
    switch (event.type) {
      char buf[4];
      XComposeStatus status;
      KeySym ks;
    /* Das Redraw-Event */
    case Expose:
      XCopyArea(display[usewindow],pix[usewindow],win[usewindow],gc[usewindow],
          event.xexpose.x,event.xexpose.y,
          event.xexpose.width,event.xexpose.height,
          event.xexpose.x,event.xexpose.y);
      break;

    /* Bei Mouse-Taste: */
    case ButtonPress:
      if(event.xbutton.button==1) {
        sbut=objc_find(tree,event.xbutton.x,event.xbutton.y);
        if(sbut!=-1) {
	if((tree[sbut].ob_flags & SELECTABLE) && !(tree[sbut].ob_state & DISABLED)) {
          if(tree[sbut].ob_flags & RBUTTON) {
            idx=rootob(tree,sbut);
            if(idx>=0) {
	      int start=tree[idx].ob_head;
	      int stop=tree[idx].ob_tail;
	      if(start>=0) {
		idx=start;
		while(1) {
		  if(tree[idx].ob_flags & RBUTTON) tree[idx].ob_state=tree[idx].ob_state & (~SELECTED);
		  if(idx==stop) break;
	          idx=tree[idx].ob_next;
		}
	      }
            }
          }
	
	    tree[sbut].ob_state^=SELECTED;
	    objc_draw(tree,0,-1,0,0);
	    if(edob>=0) draw_edcursor(tree,edob); activate();

	    if(tree[sbut].ob_flags & EXIT) {bpress=1;}
	  }
	  if(tree[sbut].ob_flags & TOUCHEXIT) {exitf=1;}
	  if(tree[sbut].ob_flags & EDITABLE) {
	    edob=sbut;
	    ((TEDINFO *)tree[edob].ob_spec)->te_junk1=strlen((char *)((TEDINFO *)tree[edob].ob_spec)->te_ptext);
	    objc_draw(tree,0,-1,0,0);
	    draw_edcursor(tree,edob);
	    activate();
	  }
	}
      } else bpress=1;
      break;
    case ButtonRelease:
      if(bpress) exitf=1;
      break;

    case KeyPress:   /* Return gedrueckt ? */
      XLookupString((XKeyEvent *)&event,buf,sizeof(buf),&ks,&status);
      if((ks & 255)==13) {                /* RETURN  */
        int idx=0;
	while(1) {
	  if(tree[idx].ob_flags & DEFAULT) {
	    tree[idx].ob_state^=SELECTED;
	    sbut=idx;
	    objc_draw(tree,0,-1,0,0); activate();
	    if(tree[idx].ob_flags & EXIT) bpress=1;
          }
	  if(tree[idx].ob_flags & LASTOB) break;
	  idx++;
	}
      } else if(edob>=0){
         int i;
         TEDINFO *ted=(TEDINFO *)(tree[edob].ob_spec);
         if(HIBYTE(ks)) {
           if((global_keycode & 255)==8) {          /* BSP */
	     if(ted->te_junk1>0) {
	       int len=strlen((char *)ted->te_ptext);
	       i=ted->te_junk1--;
	       while(i<len) ((char *)(ted->te_ptext))[i-1]=((char *)(ted->te_ptext))[i++];
	       ((char *)ted->te_ptext)[i-1]=0;
	       objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);activate();
	     }
	   } else if(ks==0xff51) { /* LEFT */
	     if(ted->te_junk1>0) ted->te_junk1--;
	     objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);activate();
	   } else if(ks==0xff53) { /* RIGHT */
	     int len=strlen((char *)ted->te_ptext);
	     if(ted->te_junk1<len && ((char *)ted->te_ptext)[ted->te_junk1]) ted->te_junk1++;
             objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);activate();
	   } else if(ks==0xff09) {          /* TAB */
	     /* Suche naechstes ED-Feld oder wieder das erste */
	     int cp=ted->te_junk1;
	     i=finded(tree,edob,1);
	     if(i<0) edob=finded(tree,0,0);
	     else edob=i;
	     ted=(TEDINFO *)(tree[edob].ob_spec);
	     ted->te_junk1=min(cp,strlen((char *)ted->te_ptext));
	     objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);activate();
	   } else if(ks==0xff52) {
	   /* Suche vorangehendes ED-Feld */
	     int cp=ted->te_junk1;
	     i=finded(tree,edob,-1);
	     if(i>=0) {edob=i;ted=(TEDINFO *)(tree[edob].ob_spec);
	     ted->te_junk1=min(cp,strlen((char *)ted->te_ptext));
	     objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);activate();}
	   } else if(ks==0xff54) { /* Page down */
	     int cp=ted->te_junk1;
	     /* Suche naechstes ED-Feld  */
	     i=finded(tree,edob,1);
	     if(i>=0) {
	       edob=i;
	       ted=(TEDINFO *)(tree[edob].ob_spec);
	       ted->te_junk1=min(cp,strlen((char *)ted->te_ptext));
	       objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);
	       activate();
	     }
	   } else if(ks==0xff1b) {   /* ESC  */
	   ((char *)ted->te_ptext)[0]=0;
	   ted->te_junk1=0;
	   objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);activate();
	   } else printf("Key: %x\n",ks);
	} else {
	  i=ted->te_txtlen-1;
	  while(i>ted->te_junk1) {((char *)ted->te_ptext)[i]=((char *)ted->te_ptext)[i-1];i--;}
	
	  if(ted->te_junk1<ted->te_txtlen) {
	    ((char *)ted->te_ptext)[ted->te_junk1]=(char)ks;
	    ted->te_junk1++;
	  }	
	  objc_draw(tree,0,-1,0,0);draw_edcursor(tree,edob);activate();
        }
      }
      break;
    case KeyRelease:
      if(bpress) exitf=1;
      break;
    }
    #endif
  }
  return(sbut);
}

int create_window2(int nummer,char *title, char* info,unsigned int x,unsigned int y,unsigned int w,unsigned int h) {

  int screen_num;              /* Ein Server kann mehrere Bildschirme haben */
  unsigned long border=4,foreground,background;
  int i,d,b;
#ifdef WINDOWS
    static class_reg=0;
#endif
  if(winbesetzt[nummer]) {
    printf("X11-Basic: Window %d already open !\n",nummer);
    return(-1);
  } else {
#if 0
  XGCValues gc_val;            /* */
  Window root;
  char *wn;
  char *in;
  XTextProperty win_name, icon_name;
  char *agv[1];


    strcpy(wname[nummer],title);
    strcpy(iname[nummer],info);
    wn=wname[nummer];
    in=iname[nummer];

  /* Verbindung zum X-Server aufnehmen. */
  if ( (display[nummer] = XOpenDisplay(display_name)) == NULL) {
    printf("Can't Connect XServer on display %s. Aborted\n",
	    XDisplayName(display_name));
    return(-1);
  }
  /* Welchen Bildschirm nehmen ? */
  screen_num = DefaultScreen(display[nummer]);

  /* Fenster Oeffnen */
  background = BlackPixel(display[nummer], DefaultScreen(display[nummer]));
  foreground = WhitePixel(display[nummer], screen_num);
  win[nummer] = XCreateSimpleWindow(display[nummer], RootWindow(display[nummer], screen_num),
			    x, y, w, h, border, foreground, background);
  XGetGeometry(display[nummer],win[nummer],&root,&x,&y,&w,&h,&b,&d);
  pix[nummer]=XCreatePixmap(display[nummer],win[nummer],w,h,d);
  fetch_icon_pixmap(nummer);

    /* Dem Window-Manager Hinweise geben, was er mit der Groesse des Fensters
     machen soll. */
  size_hints[nummer].flags = PPosition | PSize | PMinSize;
  size_hints[nummer].min_width = 32;
  size_hints[nummer].min_height = 32;
  wm_hints[nummer].flags = StateHint | InputHint | IconPixmapHint;
  wm_hints[nummer].initial_state = NormalState;
  wm_hints[nummer].input = True;
  wm_hints[nummer].icon_pixmap = icon_pixmap[nummer];
  class_hint[nummer].res_name = "X11-Basic";
  class_hint[nummer].res_class = "Graphics";

    if (!XStringListToTextProperty(&wn, 1, &win_name) ||
      !XStringListToTextProperty(&in, 1, &icon_name)) {
    printf("Couldn't set Name of Window or Icon. Aborted\n");
    return(-1);
  }

  /* Man XSetWMProperties, um zu lesen, was passiert ! */
  XSetWMProperties(display[nummer], win[nummer], &win_name, &icon_name, agv, 0,
		   &size_hints[nummer], &wm_hints[nummer], &class_hint[nummer]);

  /* Auswaehlen, welche Events man von dem Fenster bekommen moechte */
  XSelectInput(display[nummer], win[nummer],
               /*ResizeRedirectMask |*/
	       ExposureMask |
	       ButtonPressMask|
	       ButtonReleaseMask|
	       PointerMotionMask |
	       KeyPressMask|
	       KeyReleaseMask);
  gc[nummer] = XCreateGC(display[nummer], win[nummer], 0, &gc_val);

  XSetForeground(display[nummer], gc[nummer], foreground);
#endif  
    winbesetzt[nummer]=1;
  }
  return(nummer);
}
int fetch_rootwindow() {
  char *display_name = NULL;   /* NULL: Nimm Argument aus setenv DISPLAY */
  unsigned long foreground,background;
  int i,x,y,w,h,b,d;
#if 0
  XGCValues gc_val;            /* */
  Window root;


  /* Verbindung zum X-Server aufnehmen. */
  if ((display[0]=XOpenDisplay(display_name))==NULL) {
    printf("Can't Connect XServer on display %s. Aborted\n",
	    XDisplayName(display_name));
    return(-1);
  }


  win[0] = RootWindow(display[0],DefaultScreen(display[0]));
  XGetGeometry(display[0],win[0],&root,&x,&y,&w,&h,&b,&d);

  pix[0]=XCreatePixmap(display[0],win[0],w,h,d);


  /* Auswaehlen, welche Events man von dem Fenster bekommen moechte */
 /* XSelectInput(display[0], win[0],
	       ExposureMask |
	       ButtonPressMask|
	       ButtonReleaseMask|
	       PointerMotionMask |
	       KeyPressMask|
	       KeyReleaseMask); */
	
  gc[0] = XCreateGC(display[0], win[0], 0, &gc_val);
#endif
  winbesetzt[0]=1;
  return(0);
}

int create_window(char *title, char* info,unsigned int x,unsigned int y,unsigned int w,unsigned int h) {
  int nummer=0;
  while(winbesetzt[nummer] && nummer<MAXWINDOWS) nummer++;
  if(nummer>=MAXWINDOWS) {
      printf("No more windows !\n");
      return(-2);
  }
  printf("createwindow %d\n",nummer);
  return(create_window2(nummer,title,info,x,y,w,h));
}

void open_window(int nr) {
 if(winbesetzt[nr]) {
#if 0
    XEvent event;
    /* Das Fensterauf den Screen Mappen */

    XMapWindow(display[nr], win[nr]);
    XNextEvent(display[nr], &event);
    handle_event(nr,&event);
#endif
  }
}

void handle_window(int winnr) {

  if(winbesetzt[winnr]) {

#if 0
   XEvent event;

    while(XCheckWindowEvent(display[winnr],win[winnr] ,
        ExposureMask|
	ButtonPressMask|
	PointerMotionMask |
	KeyPressMask, &event)) {
       handle_event(winnr,&event);
    }
#endif
  }
}

void graphics(){
#ifdef DEBUG
  printf("graphics:\n");
#endif
  if(winbesetzt[usewindow]) {handle_window(usewindow);return;}
  else {
     if(usewindow==0) {
       fetch_rootwindow();
     } else {
       create_window2(usewindow,"X11-Basic","X11-Basic",100,10,WINDOW_DEFAULT_W,WINDOW_DEFAULT_H);
       open_window(usewindow);
     }
  }
}
