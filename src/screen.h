
/* screen.h                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */


  #define FB_DEVICE_NAME "/dev/ts"


/* Some colors (16 Bit color screen assumed) */

  #define BLACK     0x0000
  #define WHITE     0xffff
  #define RED       0xf800
  #define GREEN     0x07e0
  #define BLUE      0x001f
  #define YELLOW    0xffe0
  #define MAGENTA   0xf81f
  #define LIGHTBLUE 0x07ff
  #define GREY      0x7bef
  #define LIGHTGREY 0xadf7

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

  #define ScreenWidth  (vinfo.xres)
  #define ScreenHeight (vinfo.yres)
  #define Scanline (vinfo.xres*vinfo.bits_per_pixel/8)

void FbRender_Open();
void FbRender_Close();
void FbRender_Flush();
void FillBox (int x, int y, int w, int h, unsigned short color);

void FbRender_Clear(int,int, unsigned short);
extern struct fb_var_screeninfo vinfo;
extern struct fb_fix_screeninfo finfo;
extern long int screensize;
extern char *fbp;
extern char *fbbackp;

extern unsigned short global_color;
extern unsigned short global_bcolor;

