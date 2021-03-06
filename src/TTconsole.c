
/* TTconsole.c                            (c) Markus Hoffmann  2007-2011
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

#include <stdlib.h>
#include <stdio.h>

#include <sysexits.h>


#include <unistd.h>
//#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <sys/socket.h>

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <termios.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pty.h>

#include "screen.h"
#include "consolefont.h"
#include "terminal.h"
#include "touchscreen.h"
#include "graphics.h"
#include "keyboard.h"
  
struct winsize win={25,80,320,240};
char *cutbuffer;
int CharWidth=CharWidth57;
int CharHeight=CharHeight57;

void io_error(int n, char *s) {
  char buffer[strlen(s)+100];
  sprintf(buffer,"\033[31mIOERROR: errno=%d\033[m\n",n);
  g_outs(buffer);
}



/* Spawns a process with redirected standard input and output
   streams. ARGV is the set of arguments for the process,
   terminated by a NULL element. The first element of ARGV
   should be the command to invoke the process.
   Returns a file descriptor that can be used to communicate
   with the process. */

int spawn (char *argv[]) {
  int ret_fd = -1;
  char slavename[80]="";

  /* Find out if the intended programme really exists and
     is accessible. */
  struct stat stat_buf;
  if (stat (argv[0], &stat_buf) != 0) {g_outs("ERROR accessing programme");return -1;}

#ifdef SAVE_STDERR
  /* Save the standard error stream. */
  int saved_stderr = dup (STDERR_FILENO);
  if(saved_stderr < 0) {g_outs("ERROR saving old STDERR");return -1;}
#endif

  /* Create a pseudo terminal and fork a process attached
     to it. */
  pid_t pid = forkpty (&ret_fd,slavename, NULL, &win);
  if (pid == 0) {
    /* Inside the child process. */

    /* Ensure that terminal echo is switched off so that we
       do not get back from the spawned process the same
       messages that we have sent it. */
    struct termios orig_termios;
    if(tcgetattr(STDIN_FILENO,&orig_termios)<0) {g_outs("ERROR getting current terminal's attributes");return -1;}

    orig_termios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    orig_termios.c_oflag &= ~(ONLCR);

    if(tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios) < 0) {g_outs("ERROR setting current terminal's attributes");return -1;}

#ifdef SAVE_STDERR
    /* Restore stderr. */
    if(dup2(saved_stderr, STDERR_FILENO)<0) {g_outs("ERROR restoring STDERR");return -1;}
#endif
    
    
    printf("Shell-Access on the TomTom\n          (c) Markus Hoffmann   2007-2011\n");
    printf("\nstdin=%d, stdout=%d\n",STDIN_FILENO,STDOUT_FILENO);
    /* This should now already work */
    printf("The child is now going to excecute the shell!\n");
    /* Now spawn the intended programme. */
    if (execv (argv[0], argv)) {
      /* execv() should not return. */
      g_outs("ERROR spawning program\n");
      return -1;
    }
  } else if (pid < 0) {
    io_error(errno,"forkpty");
    if(errno==ENOENT) g_outs("There are no available ttys.\n");
    g_outs("slavename=");g_outs(slavename);
    g_outs("\nERROR spawning program\n");
    return -1;
  }
#ifdef SAVE_STDERR
  else close(saved_stderr);
#endif
  return ret_fd;
}

  /* Splash Message ausgeben */

void intro() {
#if DEBUG
  printf("print splash message.\n");
#endif
  Fb_BlitText((ScreenWidth/2)-(16*CharWidth), 136, BLUE, BLACK, "http://www.opentom.org/TomTom_Console");

  set_color(MAGENTA);  box(10,10,ScreenWidth-10,ScreenHeight-10);
  set_color(LIGHTBLUE);box(20,20,ScreenWidth-20,ScreenHeight-20);
}
void usage(){
  puts("\nUsage: "
       " TTconsole [options] [<shell-cmd>] --- excecute shell\n\n"
       "--exec <command>        --- execute shell command and exit TTconsole\n"
       "--noclear               --- do not clear the screen before excecuting the shell\n"
       "--rotatets              --- rotate touchscreen orientation by 90 deg\n"
       "--keyboardlayout_en     --- use US keyboard layout\n"
       "--bigfont               --- use bigger 8x16 font\n"
       "--bigkeys               --- use bigger keyboard\n"
       "--login <login_binary>  --- Use the login_binary to log in instead of the default [%s]\n"
       "-h --help               --- Usage");
}

void set_fontsize(int big);

void change_fontsize(int big) {
  if(bigfont!=big) {
    bigfont=big;
    set_fontsize(big);
//     pty_change_window_size(terminal_fd, win.ws_row, win.ws_col, win.ws_xpixel, win.ws_ypixel);
    ioctl(terminal_fd, TIOCSWINSZ, &win); /*Announce new screen size*/

  }
}

void set_fontsize(int big) {
  /* Fontgroesse etc anpassen */
  if(big) {
    CharWidth=CharWidth816;
    CharHeight=CharHeight816;
  } else {
    CharWidth=CharWidth57;
    CharHeight=CharHeight57;
  }
  /* Setze information fuer terminals */
  win.ws_row=vinfo.yres/CharHeight;
  win.ws_col=vinfo.xres/CharWidth;
}

static int areadefined=0;
static int copyareastart;
static int copyareaend;

static void clear_cutarea() {
  if(areadefined) {
    /* alte copyarea loeschen */
    int j;
    for(j=copyareastart;j<=copyareaend;j++) {
      Fb_inverse((j%LineLen)*CharWidth,j/LineLen*CharHeight,CharWidth,CharHeight);
    }
    areadefined=0;
  }
}
static void redraw_cutarea() {
  int j;
  if(areadefined) clear_cutarea(); /* alte copyarea loeschen */
  /* area invertieren */
  for(j=copyareastart;j<=copyareaend;j++) {
    Fb_inverse((j%LineLen)*CharWidth,j/LineLen*CharHeight,CharWidth,CharHeight);
  }
  areadefined=1;
}
void draw_message(int z,char *b) {
  Fb_BlitText57(0,z*10,RED,BLACK,b);
  puts(b);
}

#define log_message(b) puts(b)

static OBJECT *objects=keyboard_objects;

/* Test-Routine der Zeichenausgabe. Es wird der Zeichensatz ausgegeben und das
   Keyboard dargestellt. Wenn eine datei intro.ans vorhanden ist, wird diese 
   angezeigt. */

static void screen_test() {
  int i,j,dptr;
  char buffer[80];
  log_message("Doing screen-test.");
  for(i=0;i<16;i++) {
    sprintf(buffer,"0x%02x: ",i);
    g_outs(buffer);
    for(j=0;j<16;j++) {
      sprintf(buffer,"%02x ",i*16+j);
      g_outs(buffer);
    }
    for(j=0;j<16;j++) {
      if(i*16+j>0 && i*16+j!=27 && i*16+j!=26&& i*16+j!=8&& i*16+j!=16) {
        sprintf(buffer,"%c ",(char)(i*16+j));
      } else sprintf(buffer,". ");
      g_outs(buffer);
    }
    g_outs("\n");
  }
  objc_draw(objects,0,-1,0,0);
  dptr=open("intro.ans",O_RDONLY);
  if(dptr!=-1) {
    while(read(dptr,buffer,1)) g_out(*buffer);  
    close(dptr);
  }
}

extern int rotatets,keyboardlayout;

int main(int argc, char** argv) {
  char buffer[100];
  int x=0,y=0,pen=0;
  int sel,retval;
  int clickcount=0;
  struct timeval tv;
  static char *default_shell="/bin/sh";
  int DoExit=0,doexec=0,noclear=0,dotest=0;

  char *arg[argc+1];         /* we pass in maximum argc values */
  int anzarg=1;
  fd_set active_fd_set,read_fd_set;

  /* Initialisierungen */
  TsScreen_Init();
  Fb_Open();
  gem_init();

  cutbuffer=malloc(AnzLine*(LineLen+1)+1); /*Hier wird mit dem kleinen Font gerechnet.*/

  /* Setze information fuer terminals */

  win.ws_xpixel=vinfo.xres;
  win.ws_ypixel=vinfo.yres;
  win.ws_row=vinfo.yres/CharHeight;
  win.ws_col=vinfo.xres/CharWidth;

  /* Now processing the commandline */
  log_message("Processing commandline...");

  if(argc>1) {
    int count;
    for(count=1;count<argc;count++) {    
      if(strcmp(argv[count],"-h")==0) {
        usage();
        DoExit=1;
      } else if (strcmp(argv[count],"--help")==0) {
	usage();
        DoExit=1;
      } else if (strcmp(argv[count],"--exec")==0) doexec=1;
      else if (strcmp(argv[count],"--test")==0) dotest=1;
      else if (strcmp(argv[count],"--noclear")==0) noclear=1;
      else if (strcmp(argv[count],"--rotatets")==0) rotatets=1;
      else if (strcmp(argv[count],"--bigfont")==0) bigfont=1;
      else if (strcmp(argv[count],"--bigkeys")==0) bigkeys=1;
      else if (strcmp(argv[count],"--keyboardlayout_en")==0) keyboardlayout=1; /* 1 means us */
      else if (strcmp(argv[count],"--login")==0) {
        count++;
	if(count<argc && strlen(argv[count])) {
          default_shell=argv[count];
	}
      } else {
        arg[anzarg++]=argv[count];
      }
    }
  }
  arg[0]=default_shell;
  arg[anzarg]=NULL;

  while(TsScreen_pen(&x,&y,&pen)) ; /* flush pen input */
  if(noclear==0) Fb_Clear(0,ScreenHeight,BLACK);   /* clear screen */

  set_fontsize(bigfont);   /* Fontgroesse  anpassen */

  /* Wenn gewuenscht: Keyboard-layout aendern...*/
  if(keyboardlayout) objects=keyboard_objects_en;

  /* Jetzt Keyboard initialisieren und Positionen relozieren */
  keyboard_init(objects);

  intro();    /* Splash Message ausgeben */

  set_color(WHITE);
  set_bcolor(BLACK);

  g_outs("\033c\033[7m   Shell-Access on the TomTom V." VERSION "    \033[m\n        (c) Markus Hoffmann   2007-2011\n");
  sprintf(buffer,"\n\033[32mScreen-Dimensions: w=%d, h=%d, b=%d\n" 
                           "      ->           %dx%d characters.\033[33m\n",
  vinfo.xres,vinfo.yres,vinfo.bits_per_pixel,LineLen,AnzLine);
  g_outs(buffer);


  /* Place keyboard to upper right corner */
  objects[0].ob_x=vinfo.xres-objects[0].ob_width;
  objects[0].ob_y=3; /*not to underwrite screen with outline box */
  if(vinfo.xres>objects[0].ob_width+6) objects[0].ob_x-=3;
 
/* Zuerst stdin und stdout umleiten */
  
  printf("Starting the shell %s\n",arg[0]);
  terminal_fd=spawn(arg);
  if(terminal_fd<0) {
    g_outs("Cannot spawn shell! ERROR, QUIT\n");
    log_message("Cannot spawn shell! ERROR, QUIT.");
    while (TsScreen_pen(&x,&y,&pen));
    while (!TsScreen_pen(&x,&y,&pen)) ; 
    while (TsScreen_pen(&x,&y,&pen));  
    Fb_Close();
    TsScreen_Exit();
    exit(EX_OSERR);
  }     

  if(dotest) screen_test();

  int cc=0;
  char c;
  static int prev_pen = 0;

  printf("Initialize fd_set terminal_fd=%d.\n",terminal_fd);

  FD_ZERO (&active_fd_set);
  FD_SET (tsfd, &active_fd_set); 
  FD_SET (terminal_fd, &active_fd_set); 

  log_message("Draw object tree.");

  objc_draw(objects,BUT_KEYB,BUT_KEYB,objects[0].ob_x,objects[0].ob_y);

  log_message("Enter main loop.");
  int hideit=0;

  while (!DoExit) {
    tv.tv_sec =  1;   /* Wait up to one second. */
    tv.tv_usec = 10000;

    read_fd_set = active_fd_set;
#if DEBUG
    g_outs("PARENT: Waiting for an event...");
#endif  
    retval = select (FD_SETSIZE, &read_fd_set, NULL, NULL, &tv); 
    if(retval<0) {
      if(errno==EINTR) ; /* This is OK */
      else if(errno==EBADF) { /* This is not OK */
        draw_message(0,"TTconsole: select failed! Connection to Shell lost! ");
        while (TsScreen_pen(&x,&y,&pen));
        while (!TsScreen_pen(&x,&y,&pen)); 
        while (TsScreen_pen(&x,&y,&pen));     
	Fb_Close();
        TsScreen_Exit();
        exit(EX_OSERR);
      } else draw_message(0,"TTconsole: select failed! ");
    } else if(!retval) {
     // g_outs("++timeout!\n");
      if(doexec) DoExit=1;  /* This, probably, will also not work.  */
    } else {
      if(FD_ISSET(tsfd,&read_fd_set)) {  /*  Touchscreen event...   */
        if(TsScreen_pen(&x,&y,&pen)) {
          if((sel=objc_find(objects,x,y))!=-1) {
	    if(pen) {
	      if((objects[sel].ob_flags&SELECTABLE) && !(objects[sel].ob_state & DISABLED)) { 
                if(objects[sel].ob_flags & RBUTTON) { /* dann andere unselektieren */
                  int idx=rootob(objects,sel);
                  if(idx>=0) {
	            int start=objects[idx].ob_head;
	            int stop=objects[idx].ob_tail;
	            if(start>=0) {
		      idx=start;
		      while(1) {
		        if(objects[idx].ob_flags & RBUTTON) objects[idx].ob_state=objects[idx].ob_state & (~SELECTED);
  		        if(idx==stop) break;
	                idx=objects[idx].ob_next;
 	 	      }
	            }
                  }
		}
                static int oc=-1;
	        if(oc!=clickcount) {
  	          objects[sel].ob_state^=SELECTED;
	          objc_draw(objects,sel,sel,objects[0].ob_x,objects[0].ob_y);
		  oc=clickcount;
	          if(objects[sel].ob_flags & TOUCHEXIT) {
  	            if(sel==BUT_RETURN) ;
	            else {char a=*(char *)objects[sel].ob_spec; g_out('*');}
	            objects[sel].ob_state&=(~SELECTED);
	            objc_draw(objects,sel,sel,objects[0].ob_x,objects[0].ob_y);       
	          }
	        }
	      } /*else {sprintf(buffer,"Touched object #%d\n",sel);g_outs(buffer);}*/
	    } else {  /* pen=0 */
	//      usleep(100000); /*10ms Keboardrepeat-Verzoegerung fuer neuere Modelle*/
	      if(objects[sel].ob_flags & EXIT) {
  	        if(sel==BUT_RETURN)         {
		  if(objects[BUT_CTRL1].ob_state&SELECTED||objects[BUT_CTRL2].ob_state&SELECTED) 
		       c=27;
		  else c=13;
		  write(terminal_fd,&c,1);
		}
	        else if(sel==BUT_KEYB) objc_draw(objects,0,-1,0,0);
	        else if(sel==BUT_BSP)       {c=8;write(terminal_fd,&c,1);}
	        else if(sel==BUT_CUR_UP)    {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[A",2);}
	        else if(sel==BUT_CUR_DOWN)  {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[B",2);}
	        else if(sel==BUT_CUR_RIGHT) {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[C",2);}
	        else if(sel==BUT_CUR_LEFT)  {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[D",2);}
	        else if(sel==BUT_F1) {
		  c=27;write(terminal_fd,&c,1);
		  if(objects[BUT_SHIFT1].ob_state&SELECTED ||objects[BUT_SHIFT2].ob_state&SELECTED) 
		       write(terminal_fd,"[23~",4);
		  else write(terminal_fd,"OP",2);
		} else if(sel==BUT_F2) {
		  c=27;write(terminal_fd,&c,1);
		  if(objects[BUT_SHIFT1].ob_state&SELECTED ||objects[BUT_SHIFT2].ob_state&SELECTED) 
		       write(terminal_fd,"[24~",4);
		  else write(terminal_fd,"OQ",2);
		} else if(sel==BUT_F3) {
		  c=27;write(terminal_fd,&c,1);
		  if(objects[BUT_SHIFT1].ob_state&SELECTED ||objects[BUT_SHIFT2].ob_state&SELECTED) 
		       write(terminal_fd,"[25~",4);
		  else write(terminal_fd,"OR",2);
		} else if(sel==BUT_F4) {
		  c=27;write(terminal_fd,&c,1);
		  if(objects[BUT_SHIFT1].ob_state&SELECTED ||objects[BUT_SHIFT2].ob_state&SELECTED) 
		       write(terminal_fd,"[26~",4);
		  else write(terminal_fd,"OS",2);
		}
#if 0
	        else if(sel==BUT_F5)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[15~",4);}
	        else if(sel==BUT_F6)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[17~",4);}
	        else if(sel==BUT_F7)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[18~",4);}
	        else if(sel==BUT_F8)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[19~",4);}
	        else if(sel==BUT_F9)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[20~",4);}
	        else if(sel==BUT_F10)       {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[21~",4);}
	        else if(sel==BUT_F11)       {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[23~",4);}
	        else if(sel==BUT_F12)       {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[24~",4);}
#endif                
	        else if(sel==BUT_PGUP)      {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[5~",3);}
	        else if(sel==BUT_PGDOWN)    {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[6~",3);}
	        else if(sel==BUT_INSERT)    {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[2~",3);}
	        else if(sel==BUT_DELETE)    {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[3~",3);}
	        else if(sel==BUT_POS1)      {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[H",2);}
	        else if(sel==BUT_END)       {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[F",2);}
	        else if(sel==BUT_TAB)       {
		  if(objects[BUT_CTRL1].ob_state&SELECTED||objects[BUT_CTRL2].ob_state&SELECTED) 
		       c=27;
		  else c=9;
		  write(terminal_fd,&c,1);
		}
	        else if(sel==BUT_ESC)       {c=27;write(terminal_fd,&c,1);}
	        else if(sel==BUT_BLANK)     {c=32;write(terminal_fd,&c,1);}
	        else if(sel==BUT_PASTE)     {write(terminal_fd,cutbuffer,strlen(cutbuffer));}
	        else if(sel==BUT_QUIT)      DoExit=1;
	        else if(sel==BUT_CLEAR)     {g_out(27);g_outs("c");}
	        else if(sel==BUT_HIDE) {
		  cursor_onoff(0,col*CharWidth,lin*CharHeight);
                  textscreen_redraw(0,0,ScreenWidth/CharWidth,11);      
		  cursor_onoff(1,col*CharWidth,lin*CharHeight);
		  hideit=1;
	        } else if(sel==BUT_MENU) {
                  g_outs("MENU-Function:\n");
		  change_fontsize(!bigfont);
		  g_out(27);g_outs("[H");
		  if(!bigfont) g_outs("Small font activated.\n");
		  else {
                    col=min(col,LineLen-1);
                    lin=min(lin,AnzLine-1);
		    g_outs("Big font activated.\n");
                  }
		  sprintf(buffer,"%dx%d characters.\n",LineLen,AnzLine);
		  g_outs(buffer);
		  hideit=1;
		} else {
		  c=*(char *)objects[sel].ob_spec;
		  if(objects[BUT_SHIFT1].ob_state&SELECTED ||objects[BUT_SHIFT2].ob_state&SELECTED) {
		    objects[BUT_SHIFT1].ob_state&=(~SELECTED);
		    objects[BUT_SHIFT2].ob_state&=(~SELECTED);
		    c=shift_translation(c);
		  }
		  if(objects[BUT_CAPS].ob_state&SELECTED) c=caps_translation(c);
		  if(objects[BUT_CTRL1].ob_state&SELECTED||objects[BUT_CTRL2].ob_state&SELECTED) {
		      objects[BUT_CTRL1].ob_state&=(~SELECTED);
		      objects[BUT_CTRL2].ob_state&=(~SELECTED);
  		      c=c-'a'+1; 
		  }
		  if(objects[BUT_ALTG].ob_state&SELECTED) {
		    objects[BUT_ALTG].ob_state&=(~SELECTED);
		    c=altGr_translation(c);
		  }
		  write(terminal_fd,&c,1);
		}
		objects[sel].ob_state&=(~SELECTED);
		if(hideit) {hideit=0;objc_draw(objects,BUT_KEYB,BUT_KEYB,objects[0].ob_x,objects[0].ob_y);}
                else objc_draw(objects,0,-1,0,0);       		
              }	  
	    }
          } else {
	    if(attributes&AT_MOUSE) {
	      char buffer[32];
	      static int click=-1;
              if(click!=clickcount) {
                if(pen) draw_message(0,"TTconsole: >CLICK< ");
                else    draw_message(0,"TTconsole: >RELEASE< ");
		sprintf(buffer,"\033[%d;%d;%dM",x,y,(pen!=0));
		write(terminal_fd,buffer,strlen(buffer));
                click=clickcount;
	      } else {
                draw_message(0,"TTconsole: rush... ");
	        sprintf(buffer,"\033[%d;%d;%do",x,y,(pen!=0));
		write(terminal_fd,buffer,strlen(buffer));
	      }
	    } else {
              int c=x/CharWidth;
	      int l=y/CharHeight;
	      if (pen) {
	        static int click=-1;
	        if(click!=clickcount) {  /* klick ist neu */
                  draw_message(0,"TTconsole: CLICK ");
                  clear_cutarea(); /* alte copyarea loeschen */
	          copyareastart=c+l*(ScreenWidth/CharWidth);
	          sprintf(buffer,"Areastart=%d.  ",copyareastart); draw_message(2,buffer);
                  click=clickcount;
	        } else { /* klick ist alt (mausbewegung) */
	          copyareaend=max(copyareastart+1,c+l*(ScreenWidth/CharWidth));
	          sprintf(buffer,"Areaend=%d.  ",copyareaend); draw_message(3,buffer);
                  redraw_cutarea();  /* area invertieren */
	        }
              } else {
	        /* losgelassen, also in cutbuffer kopieren */
	        copyareaend=max(copyareastart+1,c+l*(ScreenWidth/CharWidth));
	        sprintf(buffer,"Areaend=%d.  ",copyareaend); draw_message(3,buffer);
                redraw_cutarea();  /* area invertieren */
		
                if(areadefined) {
	          int i=0,j;
	          for(j=copyareastart;j<=copyareaend;j++) {
                    if(textscreen[j].c) cutbuffer[i++]=textscreen[j].c;
		    else {
		      cutbuffer[i]=13;
		      if(i && cutbuffer[i-1]!=13) i++;
		    }
	          }
                  cutbuffer[i]=0;
	          sprintf(buffer,"%d bytes in cutbuffer.",i); draw_message(2,buffer);
	        }
	      }
	    }
	  }
	  if(prev_pen!=pen && !pen) clickcount++;
          prev_pen=pen;
        }
      } else if(FD_ISSET(terminal_fd,&read_fd_set)) {
        cc=read(terminal_fd,&c,1);
        if(cc>0) g_out(c);
	else if(cc==-1) {
	  if(errno==EINTR || errno==EAGAIN) ; /* This is OK */
          else if(errno==EBADF ||errno==EIO) { /* This is not OK */
            draw_message(0,"TTconsole: read failed! Connection to Shell lost! ");
	    DoExit=1;
          } else draw_message(0,"TTconsole: read failed! ");
	}
      } 
    }
  }  
  free(cutbuffer);
  Fb_Close();
  TsScreen_Exit();
  return(EX_OK);
}
