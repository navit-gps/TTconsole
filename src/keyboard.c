/* keyboard.c                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */
 
#define VERSION "1.07"
 
 
#if 0
char keyb1[]="[^][1][2][3][4][5][6][7][8][9][0][?][?][Bsp] [ ][ ][ ]";
char keyb2[]="[Tab][q][w][e][r][t][z][u][i][o][p][u][+][R] [ ][ ][ ]";
char keyb3[]="[Caps][a][s][d][f][g][h][j][k][l][o][a][#][]          ";
char keyb4[]="[Sh][<][y][x][c][v][b][n][m][,][.][-][Shift]    [^]   ";
char keyb5[]="[Ctr][ ][ ][              ][  ][  ][ ][Ctrl] [<][v][>]";
#endif

#include "consolefont.h"
#include "graphics.h"

#include "keyboard.h"


/* Hier jetzt den Objektbaum fuer dasw virtuelle Keyboard
   definieren */
   
  OBJECT keyboard_objects[]={
/*       next,head,tail,typ,flags,state,spec,x,y,w,h    */
/* 0*/  {-1, 1,79,G_BOX, NONE, OUTLINED, 0x00021100, 0,0,64*CharWidth,10*CharHeight+2},
/* 1*/  { 2,-1,-1,G_BUTTON, SELECTABLE|DEFAULT|EXIT,NORMAL ,(LONG)"Keyb", 58*CharWidth,0*CharHeight,5*CharWidth,2*CharHeight},
/* 2*/  { 3,-1,-1,G_BUTTON, SELECTABLE|EXIT,      NORMAL, (LONG)"QUIT", 58*CharWidth,3*CharHeight,5*CharWidth,2*CharHeight},
/* 3*/  { 4,-1,-1,G_BUTTON, SELECTABLE|EXIT,      NORMAL, (LONG)"clear",58*CharWidth,6*CharHeight,5*CharWidth,1*CharHeight},
/* 4*/  { 5,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"^",     1*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/* 5*/  { 6,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"1",     4*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/* 6*/  { 7,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"2",     7*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/* 7*/  { 8,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"3",    10*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/* 8*/  { 9,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"4",    13*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/* 9*/  {10,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"5",    16*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*10*/  {11,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"6",    19*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*11*/  {12,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"7",    22*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*12*/  {13,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"8",    25*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*13*/  {14,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"9",    28*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*14*/  {15,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"0",    31*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*15*/  {16,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"?",    34*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*16*/  {17,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"`",    37*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*17*/  {18,-1,-1,G_BUTTON, SELECTABLE|EXIT     , NORMAL, (LONG)"Bsp",  40*CharWidth,0*CharHeight,6*CharWidth,2*CharHeight},
/*18*/  {19,-1,-1,G_BUTTON, SELECTABLE|EXIT     , NORMAL, (LONG)"Tab",   1*CharWidth,2*CharHeight,4*CharWidth,2*CharHeight},
/*19*/  {20,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"q",     5*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*20*/  {21,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"w",     8*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*21*/  {22,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"e",    11*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*22*/  {23,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"r",    14*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*23*/  {24,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"t",    17*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*24*/  {25,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"z",    20*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*25*/  {26,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"u",    23*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*26*/  {27,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"i",    26*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*27*/  {28,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"o",    29*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*28*/  {29,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"p",    32*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*29*/  {30,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"\\",    35*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*30*/  {31,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"+",    38*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*31*/  {32,-1,-1,G_BUTTON, SELECTABLE,           NORMAL, (LONG)"Caps",  1*CharWidth,4*CharHeight,5*CharWidth,2*CharHeight},
/*32*/  {33,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"a",     6*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*33*/  {34,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"s",     9*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*34*/  {35,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"d",    12*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*35*/  {36,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"f",    15*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*36*/  {37,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"g",    18*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*37*/  {38,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"h",    21*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*38*/  {39,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"j",    24*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*39*/  {40,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"k",    27*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*40*/  {41,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"l",    30*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*41*/  {42,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"[",    33*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*42*/  {43,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"]",    36*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*43*/  {44,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"#",    39*CharWidth,4*CharHeight,3*CharWidth,2*CharHeight},
/*44*/  {45,-1,-1,G_BUTTON, SELECTABLE|EXIT,      NORMAL, (LONG)"Ret",  42*CharWidth,2*CharHeight,4*CharWidth,4*CharHeight},
/*45*/  {46,-1,-1,G_BUTTON, SELECTABLE,           NORMAL, (LONG)"Sh",    1*CharWidth,6*CharHeight,4*CharWidth,2*CharHeight},
/*46*/  {47,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"<",     5*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*47*/  {48,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"y",     8*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*48*/  {49,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"x",    11*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*49*/  {50,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"c",    14*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*50*/  {51,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"v",    17*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*51*/  {52,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"b",    20*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*52*/  {53,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"n",    23*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*53*/  {54,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"m",    26*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*54*/  {55,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)",",    29*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*55*/  {56,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)".",    32*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*56*/  {57,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"-",    35*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*57*/  {58,-1,-1,G_BUTTON, SELECTABLE      ,NORMAL, (LONG)"Shift",38*CharWidth,6*CharHeight,8*CharWidth,2*CharHeight},
/*58*/  {59,-1,-1,G_BUTTON, SELECTABLE,      NORMAL, (LONG)"Ctl",   1*CharWidth,8*CharHeight,4*CharWidth,2*CharHeight},
/*59*/  {60,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"ESC",   5*CharWidth,8*CharHeight,4*CharWidth,2*CharHeight},
/*60*/  {61,-1,-1,G_BUTTON, SELECTABLE,      NORMAL, (LONG)"Alt",   9*CharWidth,8*CharHeight,4*CharWidth,2*CharHeight},
/*61*/  {62,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"(c) 2008 MH",    13*CharWidth,8*CharHeight,14*CharWidth,2*CharHeight},
/*62*/  {63,-1,-1,G_BUTTON, SELECTABLE,      NORMAL, (LONG)"AltG", 27*CharWidth,8*CharHeight,5*CharWidth,2*CharHeight},
/*63*/  {64,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"paste",32*CharWidth,8*CharHeight,5*CharWidth,2*CharHeight},
/*64*/  {65,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"MENU", 37*CharWidth,8*CharHeight,5*CharWidth,2*CharHeight},
/*65*/  {66,-1,-1,G_BUTTON, SELECTABLE,      NORMAL, (LONG)"Ctl",  42*CharWidth,8*CharHeight,4*CharWidth,2*CharHeight},
/*66*/  {67,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"<",    47*CharWidth,8*CharHeight,3*CharWidth,2*CharHeight},
/*67*/  {68,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"v",    50*CharWidth,8*CharHeight,3*CharWidth,2*CharHeight},
/*68*/  {69,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)">",    53*CharWidth,8*CharHeight,3*CharWidth,2*CharHeight},
/*69*/  {70,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"^",    50*CharWidth,6*CharHeight,3*CharWidth,2*CharHeight},
/*70*/  {71,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"hide", 58*CharWidth,8*CharHeight,5*CharWidth,1*CharHeight},
/*71*/  {72,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"I",    47*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*72*/  {73,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"H",    50*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*73*/  {74,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"^",    53*CharWidth,0*CharHeight,3*CharWidth,2*CharHeight},
/*74*/  {75,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"d",    47*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*75*/  {76,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"E",    50*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*76*/  {77,-1,-1,G_BUTTON, SELECTABLE|EXIT, NORMAL, (LONG)"v",    53*CharWidth,2*CharHeight,3*CharWidth,2*CharHeight},
/*77*/  {78,-1,-1,G_STRING, NONE,            NORMAL, (LONG)"TTconsole", 47*CharWidth,4*CharHeight+4,10*CharWidth,2*CharHeight},
/*78*/  {79,-1,-1,G_STRING, NONE,            NORMAL, (LONG)VERSION, 59*CharWidth,9*CharHeight+2,5*CharWidth,1*CharHeight},
/*79*/  { 0,-1,-1,G_STRING, NONE|LASTOB,     NORMAL, (LONG)"TTconsole", 47*CharWidth,4*CharHeight+4,10*CharWidth,2*CharHeight}
  };
  int keyboard_objccount=sizeof(keyboard_objects)/sizeof(OBJECT);

