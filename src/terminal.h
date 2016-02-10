/* terminal.h                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

void g_out(char a);
void g_outs(char *t);
void cursor_onoff(int onoff,int x,int y);
void textscreen_redraw(int x,int y,int w,int h);

#define LineLen (ScreenWidth/CharWidth)
#define AnzLine (ScreenHeight/CharHeight)

typedef struct {
        unsigned char c;
	unsigned short color;
	unsigned short bcolor;
	unsigned char flags;
} CINFO;


#define AT_NORMAL       0
#define AT_KEYLOCK      4
#define AT_INSERT     0x10
#define AT_SOFTSCROLL 0x20

#define AT_LINEWRAP  (1<<7)
#define AT_KEYREPEAT (1<<8)
#define AT_MOUSE     (1<<10)
#define AT_AUTOLF    (1<<20)
#define AT_CURSORON   (1<<25)

#define AT_DEFAULT (AT_LINEWRAP|AT_CURSORON)


extern CINFO textscreen[];
extern int terminal_fd;
extern int col,lin;
extern int attributes;
