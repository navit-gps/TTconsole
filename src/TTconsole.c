
/* TTconsole.c                            (c) Markus Hoffmann  2007-2008
*/

/* This file is part of TTconsole, the TomTom Console interface
 * ======================================================================
 * TTconsole is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

#include <stdlib.h>
#include <stdio.h>

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
  

void io_error(int n, char *s) {
  char buffer[strlen(s)+100];
  sprintf(buffer,"\033[31mIOERROR: errno=%d\033[m\n",n);
  g_outs(buffer);
}

struct winsize win={25,80,320,240};



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
    
    
    printf("Shell-Access on the TomTom\n          (c) Markus Hoffmann   2007-2008\n");
    printf("\nstdin=%d, stdout=%d\n",STDIN_FILENO,STDOUT_FILENO);
    /* This should now already work */
    printf("The child is now going to excecute the shell!\n");
    /* Now spawn the intended programme. */
    if (execv (argv[0], argv)) {
      /* execv() should not return. */
      g_outs("ERROR spawning programme\n");
      return -1;
    }
  } else if (pid < 0) {
    io_error(errno,"forkpty");
    if(errno==ENOENT) g_outs("There are no available ttys.\n");
    g_outs("slavename=");g_outs(slavename);
    g_outs("\nERROR spawning programme\n");
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

  ltext(0,100,15,25,0,0,"(c) Markus Hoffmann 2008");
  set_color(WHITE);
  set_bcolor(BLACK);

}
void usage(){
  puts("\n Usage:\n ------ \n");
  printf(" TTconsole [options] [<shell-cmd>] --- excecute shell\n\n");
  puts("--exec <command>        --- execute shell command and exit TTconsole");
  puts("-h --help           --- Usage");
}


int main(int argc, char** argv) {
  char buffer[100];
  int x=0, y=0, pen=0;
  int sel;  
  int retval;
  struct timeval tv;

  char *arg[1];arg[0]="/bin/sh";arg[1]=NULL;
  fd_set active_fd_set,read_fd_set;
  OBJECT *objects=keyboard_objects;

  /* Initialisierungen */

  TsScreen_Init();
  FbRender_Open();
  gem_init();

  while(TsScreen_pen(&x,&y,&pen)) ; /* flush pen input */

  intro();    /* Splash Message ausgeben */

  set_color(WHITE);
  set_bcolor(BLACK);
  Fb_Clear(0,4*CharHeight,BLACK);

  printf("first use of g_out.\n");
  g_outs("\033[7m      Shell-Access on the TomTom V.1.03        \033[m\n           (c) Markus Hoffmann   2007-2008      \n");
  sprintf(buffer,"\n\033[32mScreen-Dimensions: w=%d, h=%d, b=%d  -> %dx%d characters.\033[33m\n",
  vinfo.xres,vinfo.yres,vinfo.bits_per_pixel,vinfo.xres/CharWidth,vinfo.yres/CharHeight);
  g_outs(buffer);

  /* Setze information fuer terminals */

  win.ws_row=vinfo.yres/CharHeight;
  win.ws_col=vinfo.xres/CharWidth;
  win.ws_xpixel=vinfo.xres;
  win.ws_ypixel=vinfo.yres;

  /* Place keyboard to upper right corner */
  keyboard_objects[0].ob_x=vinfo.xres-keyboard_objects[0].ob_width;

/* Zuerst stdin und stdout umleiten */

  printf("Starting the shell %s\n",arg[0]);
  terminal_fd=spawn(arg);
  if(terminal_fd<0) {
    g_outs("Cannot spawn shell! ERROR, QUIT\n");
    printf("Cannot spawn shell! ERROR, QUIT\n");
    while (TsScreen_pen(&x,&y,&pen));
    while (!TsScreen_pen(&x,&y,&pen)) ; 
    while (TsScreen_pen(&x,&y,&pen));  
    FbRender_Close();
    TsScreen_Exit();
    exit(0);
  }     


  int DoExit =0;
  int cc=0;
  char c;
  static int prev_pen = 0;

  printf("Initialize fd_set terminal_fd=%d.\n",terminal_fd);

  FD_ZERO (&active_fd_set);
  FD_SET (tsfd, &active_fd_set); 
  printf("SET fd_set.\n");
  FD_SET (terminal_fd, &active_fd_set); 
  printf("SET fd_set.\n");

  objc_draw(objects,BUT_KEYB,BUT_KEYB,0,0);

  /* Now passing the commandline to shell */
  printf("Processing command line.\n");

  if(argc>1) {
    int count;
    for(count=1;count<argc;count++) {    
      if(strcmp(argv[count],"-h")==0) {
        usage();
        DoExit=1;
      } else if (strcmp(argv[count],"--help")==0) {
	usage();
        DoExit=1;
      } else if (strcmp(argv[count],"--exec")==0) {
        DoExit=1;
      } else {
        write(terminal_fd,argv[count],strlen(argv[count]));
        write(terminal_fd," ",1);      
      }
    }
    write(terminal_fd,"\n",1);
  }
  printf("Enter main loop.\n");

  while (!DoExit) {
    tv.tv_sec =  1;   /* Wait up to five seconds. */
    tv.tv_usec = 10000;

    read_fd_set = active_fd_set;
#if DEBUG
    g_outs("PARENT: Waiting for an event...");
#endif  
    retval = select (FD_SETSIZE, &read_fd_set, NULL, NULL, &tv); 
    if(retval<0) g_outs("select failed!\n");
    else if(!retval) {
     // g_outs("++timeout!\n");
    } else {
      if(FD_ISSET(tsfd,&read_fd_set)) {  /*  Touchscreen event...   */
        if(TsScreen_pen(&x,&y,&pen)) {
        if((sel=objc_find(objects,x,y))!=-1) {
	  if(pen) {
	    if((objects[sel].ob_flags&SELECTABLE) && !(objects[sel].ob_state & DISABLED)) { 
              if(objects[sel].ob_flags & RBUTTON) {
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
	      objects[sel].ob_state|=SELECTED;
	      objc_draw(objects,sel,sel,0,0);
	      if(objects[sel].ob_flags & TOUCHEXIT) {
  	        if(sel==BUT_RETURN) ;
	        else {char a=*(char *)objects[sel].ob_spec; g_out('*');}
	        objects[sel].ob_state&=(~SELECTED);
	        objc_draw(objects,sel,sel,0,0);       
	      }
	    } /*else {sprintf(buffer,"Touched object #%d\n",sel);g_outs(buffer);}*/
	    } else {
	      if(objects[sel].ob_flags & EXIT) {
  	        if(sel==BUT_RETURN)         {c=13;write(terminal_fd,&c,1);}
	        else if(sel==BUT_KEYB) objc_draw(objects,0,-1,0,0);
	        else if(sel==BUT_BSP)       {c=8;write(terminal_fd,&c,1);}
	        else if(sel==BUT_CUR_UP)    {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[A",2);}
	        else if(sel==BUT_CUR_DOWN)  {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[B",2);}
	        else if(sel==BUT_CUR_RIGHT) {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[C",2);}
	        else if(sel==BUT_CUR_LEFT)  {c=27;write(terminal_fd,&c,1);write(terminal_fd,"[D",2);}
	        else if(sel==BUT_F1)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"OP",2);}
	        else if(sel==BUT_F2)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"OQ",2);}
	        else if(sel==BUT_F3)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"OR",2);}
	        else if(sel==BUT_F4)        {c=27;write(terminal_fd,&c,1);write(terminal_fd,"OS",2);}
	        else if(sel==BUT_TAB)       {c=9;write(terminal_fd,&c,1);}
	        else if(sel==BUT_ESC)       {c=27;write(terminal_fd,&c,1);}
	        else if(sel==BUT_BLANK)     {c=32;write(terminal_fd,&c,1);}
	        else if(sel==BUT_QUIT) DoExit=1;
	        else if(sel==BUT_CLEAR) {g_out(27);g_outs("[2J");g_out(27);g_outs("[m");g_out(27);g_outs("[H");}
	        else if(sel==BUT_HELP) {
                  /* hier koennte man ein form_alert ausfuehen, wenn es schon funktionierte....*/
		  g_outs("TTconsole:\n==========\n\nTouch into the right upper corner to activate the virtual keyboard.\n");
		}
	        else {
		  c=*(char *)objects[sel].ob_spec;
		  if(objects[BUT_SHIFT1].ob_state&SELECTED ||objects[BUT_SHIFT2].ob_state&SELECTED) {
		    objects[BUT_SHIFT1].ob_state&=(~SELECTED);
		    objects[BUT_SHIFT2].ob_state&=(~SELECTED);
		    if(c=='^') c='°';
		    else if(c=='1') c='!';
		    else if(c=='2') c='\"';
		    else if(c=='3') c='§';
		    else if(c=='4') c='$';
		    else if(c=='5') c='%';
		    else if(c=='6') c='&';
		    else if(c=='7') c='/';
		    else if(c=='8') c='(';
		    else if(c=='9') c=')';
		    else if(c=='0') c='=';
		    else if(c=='`') c='´';
		    else if(c=='+') c='*';
		    else if(c=='#') c='\'';
		    else if(c=='<') c='>';
		    else if(c==',') c=';';
		    else if(c=='.') c=':';
		    else if(c=='-') c='_';
		    else c=toupper(c);
		  }   
		  if(objects[BUT_CTRL1].ob_state&SELECTED||objects[BUT_CTRL2].ob_state&SELECTED) {
		      objects[BUT_CTRL1].ob_state&=(~SELECTED);
		      objects[BUT_CTRL2].ob_state&=(~SELECTED);
  		      c=c-'a'+1; 
		  }
		  if(objects[BUT_ALTG].ob_state&SELECTED) {
		    objects[BUT_ALTG].ob_state&=(~SELECTED);
		    if(c=='q') c='@';
		    else if(c=='<') c='|';
		    else if(c=='+') c='~';
		  }
		  write(terminal_fd,&c,1);
		}
	        objects[sel].ob_state&=(~SELECTED);
	        objc_draw(objects,0,-1,0,0);       		
              }	  
	    }
          } else if (pen && x>ScreenWidth/2) {
            PutLinePoint(x, y, MAGENTA, 3);  
          } else if (pen) {
            int c,l;
	    char a;
	    c=x/CharWidth;
	    l=y/CharHeight;
	    Fb_inverse(c*CharWidth,l*CharHeight,CharWidth,CharHeight);
	    a=textscreen[c][l].c;
	    if(a) {
	      g_out(a);
	      write(terminal_fd,&a,1);
	    }  
          }
          prev_pen = pen;
        }
      } else if(FD_ISSET(terminal_fd,&read_fd_set)) {
        cc=read(terminal_fd,&c,1);
        if(cc) g_out(c);
      } 
    }
  }  
  FbRender_Close();
  TsScreen_Exit();
  return 0;
}
