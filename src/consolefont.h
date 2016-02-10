/* consolefont.h                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

  #define CharWidth    5
  #define CharHeight    (7+1)

extern const unsigned char asciiTable[];

#define FL_NORMAL        0
#define FL_BOLD          2
#define FL_DIM           4
#define FL_UNDERLINE  0x10
#define FL_BLINK      0x20
#define FL_REVERSE    0x80
#define FL_HIDDEN    0x100




void FbRender_BlitCharacter(int x, int y, unsigned short aColor, unsigned short aBackColor, char character, int flags);
void FbRender_BlitText(int x, int y, unsigned short aColor, unsigned short aBackColor, char *string);
void Fb_BlitCharacter(int x, int y, unsigned short aColor, unsigned short aBackColor, char character,int flags);
void Fb_BlitText(int x, int y, unsigned short aColor, unsigned short aBackColor, char *string);
