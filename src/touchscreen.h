
/* touchscreen.h                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

void TsScreen_Init(); 
void TsScreen_Exit(); 

int TsScreen_pen(int *x,int *y,int *pen);

extern int tsfd;
