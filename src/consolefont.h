/* consolefont.h                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

extern int bigfont;

#define CharWidth816 8
#define CharWidth57 5
#define CharHeight816 16
#define CharHeight57 (7+1)

extern unsigned char *fontlist57[];
extern unsigned char *fontlist816[];
extern int fontnr;


extern const unsigned char asciiTable[];
extern const unsigned char spat_a816[];
extern unsigned char ext_font816[];
extern unsigned char ibm_like816[];

#define FL_NORMAL        0
#define FL_BOLD          1
#define FL_DIM         (1<<1)
#define FL_ITALIC      (1<<2)
#define FL_UNDERLINE   (1<<3)
#define FL_BLINK       (1<<4)
#define FL_BLINKRAPID  (1<<5)
#define FL_REVERSE     (1<<6)
#define FL_CONCREAL    (1<<7)
#define FL_CROSSED     (1<<8)
#define FL_HIDDEN      (1<<9)
#define FL_DBLUNDERLINE (1<<10)
#define FL_FRAMED       (1<<11)
#define FL_TRANSPARENT  (1<<12)




void Fb_BlitCharacter(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character,int flags,int);
void Fb_BlitCharacter816(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character,int flags,int);
void Fb_BlitCharacter57(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character,int flags,int);
void Fb_BlitText(int x, int y, unsigned short aColor, unsigned short aBackColor, char *string);
void Fb_BlitText816(int x, int y, unsigned short aColor, unsigned short aBackColor, char *str);
void Fb_BlitText57(int x, int y, unsigned short aColor, unsigned short aBackColor, char *str);

void mtext(int x, int y, char *t);
