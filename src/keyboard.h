
/* keyboard.h                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

#define VERSION "1.13"


/* Button number Definitions corresponding to Object tree */


#define BUT_KEYB 1
#define BUT_QUIT 2
#define BUT_BSP 17
#define BUT_RETURN 44
#define BUT_ESC 59
#define BUT_BLANK 61
#define BUT_CAPS 31
#define BUT_TAB 18
#define BUT_CLEAR 3
#define BUT_HIDE 70
#define BUT_CAPS 31
#define BUT_SHIFT1 45
#define BUT_SHIFT2 57
#define BUT_CTRL1 58
#define BUT_CTRL2 65
#define BUT_ALTG  62
#define BUT_PASTE 63
#define BUT_MENU  64
#define BUT_CUR_UP 69
#define BUT_CUR_DOWN 67
#define BUT_CUR_RIGHT 68
#define BUT_CUR_LEFT 66

#define BUT_INSERT 71
#define BUT_DELETE 74
#define BUT_POS1   72
#define BUT_END    75
#define BUT_PGUP   73
#define BUT_PGDOWN 76

#define BUT_F1 100
#define BUT_F2 100
#define BUT_F3 100
#define BUT_F4 100


extern OBJECT keyboard_objects[];
extern OBJECT keyboard_objects_en[];
extern int keyboard_objccount;
extern int bigkeys;


char shift_translation(char c);
char caps_translation(char c);
char altGr_translation(char c);

void keyboard_init(OBJECT *objects);
