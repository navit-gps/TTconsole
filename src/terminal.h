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

typedef struct {
        unsigned char c;
	unsigned short color;
	unsigned short bcolor;
	unsigned char flags;
} CINFO;

extern CINFO textscreen[80][40];
extern int terminal_fd;
