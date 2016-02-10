# Makefile for TomTomConsole (c) Markus Hoffmann V.1.02
PATH=/home/hoffmann/bin:/usr/local/bin:/usr/bin:/usr/X11R6/bin:/usr/bin/X11:/usr/local/armlinux/3.3.3/bin:/usr/bin:/bin

# Directories
prefix=/usr/local
exec_prefix=${prefix}

BINDIR=${exec_prefix}/bin
DATADIR=${prefix}/share
LIBDIR=${exec_prefix}/lib
MANDIR=${prefix}/man

all: TTconsole bindist dist


CC = arm-linux-gcc
DEBUG_DEFINES = -O3
STRIP = arm-linux-strip

OBJS= TTconsole.o consolefont.o screen.o terminal.o graphics.o \
      touchscreen.o keyboard.o

CSRC=$(OBJS:.o=.c)
HSRC= consolefont.h screen.h terminal.h graphics.h touchscreen.h keyboard.h
DIST= README INSTALL COPYING AUTHORS RELEASE_NOTES Makefile $(CSRC) $(HSRC) \
      terminal.bmp TTconsole.cap TTconsole-wrapper
BINDIST= README RELEASE_NOTES TTconsole terminal.bmp TTconsole.cap \
         TTconsole-wrapper


INCS = -I. -I$(HOME)/TomTom/ttlinux/include
DIR=TTconsole-1.11
DIR2=$(DIR)-bin

DISTOBJS=$(OBJS)
DIST2=$(DISTOBJS:.o=.c)
DEPSRC=$(DISTOBJS:.o=.c)

.c.o:
	$(CC) $(DEBUG_DEFINES) -c -DLINUX_TARGET -DBARCELONA_TARGET $(INCS) $*.c

TTconsole: $(OBJS) Makefile
	arm-linux-gcc -o $@ $(OBJS) -lm /usr/local/armlinux/3.3.3/arm-linux/lib/libutil.a
	$(STRIP) $@
clean:
	rm -f *.o *.a *~ TTconsole *-bin.zip
dist :	$(DIR).tar.gz

$(DIR).tar.gz : $(DIST2)
	rm -rf /tmp/$(DIR)
	mkdir /tmp/$(DIR)
	(tar cf - $(DIST))|(cd /tmp/$(DIR); tar xpf -)
	(cd /tmp; tar cf - $(DIR)|gzip -9 > $(DIR).tar.gz)
	mv /tmp/$(DIR).tar.gz .
bindist : $(DIR2).zip

$(DIR2).zip : $(BINDIST)
	rm -rf /tmp/$(DIR2)
	mkdir /tmp/$(DIR2)
	mkdir /tmp/$(DIR2)/TTconsole
	mkdir /tmp/$(DIR2)/SDKRegistry
	rsync -a README /tmp/$(DIR2)/
	rsync -a RELEASE_NOTES /tmp/$(DIR2)/TTconsole/
	rsync -a TTconsole /tmp/$(DIR2)/TTconsole/
	rsync -a TTconsole-wrapper /tmp/$(DIR2)/TTconsole/
	rsync -a terminal.bmp /tmp/$(DIR2)/SDKRegistry/
	rsync -a TTconsole.cap /tmp/$(DIR2)/SDKRegistry/
	(cd /tmp ; zip -r $(DIR2).zip $(DIR2))
	mv /tmp/$(DIR2).zip .	
depend : $(DEPSRC) 
	cp Makefile Makefile.bak
	chmod +w Makefile
	makedepend $(INC) $(DEF) $(DEPSRC) 
	sed -n '/^# DO NOT DELETE/,$$p' < Makefile > Makefile.tmp
	sed -n '1,/^# DO NOT DELETE/p' < Makefile > Makefile.new
	tail +2 Makefile.tmp|\
	sed 's/^\([A-Za-z0-9_]*\)\.o:/\1.o \1.ln:/g'>>Makefile.new
	rm -f Makefile.tmp
	mv Makefile.new Makefile

# DO NOT DELETE

touchscreen.o touchscreen.ln: /usr/include/endian.h /usr/include/bits/endian.h
touchscreen.o touchscreen.ln: /usr/include/sys/select.h /usr/include/bits/select.h
touchscreen.o touchscreen.ln: /usr/include/bits/sigset.h /usr/include/bits/time.h
touchscreen.o touchscreen.ln: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
touchscreen.o touchscreen.ln: /usr/include/alloca.h /usr/include/stdio.h
touchscreen.o touchscreen.ln: /usr/include/libio.h /usr/include/_G_config.h
touchscreen.o touchscreen.ln: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
touchscreen.o touchscreen.ln: /usr/include/bits/sys_errlist.h /usr/include/fcntl.h
touchscreen.o touchscreen.ln: /usr/include/bits/fcntl.h
keyboard.o keyboard.ln: consolefont.h graphics.h keyboard.h
