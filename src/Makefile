# Makefile for TomTomConsole (c) Markus Hoffmann V.1.02
# This is the path for compilation without floating point emulation
#PATH=/usr/local/bin:/usr/bin:/usr/local/armlinux/3.3.3/bin:/bin

# This is the path to the original TomTom development environment
PATH=/usr/local/bin:/usr/bin:/usr/local/armlinux/3.3.3/bin:/bin

# Directories
prefix=/usr
exec_prefix=${prefix}

BINDIR=${exec_prefix}/bin
DATADIR=${prefix}/share
LIBDIR=${exec_prefix}/lib
MANDIR=${prefix}/share/man


LIBNO=1.13

all: TTconsole bindist dist TTconsole-l utility-bin-$(LIBNO).zip


CC = arm-linux-gcc
DEBUG_DEFINES = -Wall -O3
STRIP = arm-linux-strip

OBJS= TTconsole.o consolefont.o screen.o terminal.o graphics.o \
      touchscreen.o keyboard.o 5x7.o 8x16.o

CSRC=$(OBJS:.o=.c)
HSRC= consolefont.h screen.h terminal.h graphics.h touchscreen.h keyboard.h
DIST= README INSTALL COPYING AUTHORS RELEASE_NOTES Makefile $(CSRC) $(HSRC) \
      terminal.bmp TTconsole.cap TTconsole-wrapper dogfeed
BINDIST= README RELEASE_NOTES TTconsole terminal.bmp TTconsole.cap \
         TTconsole-wrapper dogfeed demo.ans/*


INCS = -I. -I$(HOME)/TomTom/ttlinux/include
DIR=TTconsole-$(LIBNO)
DIR2=$(DIR)-bin

DISTOBJS=$(OBJS)
DIST2=$(DISTOBJS:.o=.c)
DEPSRC=$(DISTOBJS:.o=.c)

.c.o:
	$(CC) $(DEBUG_DEFINES) -c -DLINUX_TARGET -DBARCELONA_TARGET $(INCS) $*.c

TTconsole: $(OBJS) Makefile
	arm-linux-gcc -o $@ $(OBJS) -lm /usr/local/armlinux/3.3.3/arm-linux/lib/libutil.a
	$(STRIP) $@
TTconsole-l: $(DIST2) Makefile
	gcc -DNATIVE -I. -o $@ $(DIST2) -lm -lutil
	strip $@
utility-bin-$(LIBNO).zip: utility/*
	zip -r $@ utility
clean:
	rm -f *.o *.a *~ TTconsole *-bin.zip TTconsole-l
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
	mkdir /tmp/$(DIR2)/bin
	mkdir /tmp/$(DIR2)/text
	mkdir /tmp/$(DIR2)/SDKRegistry
	rsync -a README /tmp/$(DIR2)/
	rsync -a RELEASE_NOTES /tmp/$(DIR2)/
	rsync -a TTconsole /tmp/$(DIR2)/bin/
	rsync -a TTconsole-wrapper /tmp/$(DIR2)/bin/
	rsync -a dogfeed /tmp/$(DIR2)/bin/
	rsync -a terminal.bmp /tmp/$(DIR2)/SDKRegistry/
	rsync -a TTconsole.cap /tmp/$(DIR2)/SDKRegistry/
	rsync -a demo.ans/* /tmp/$(DIR2)/text/
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

touchscreen.o touchscreen.ln: /usr/include/linux/i2c.h /usr/include/linux/types.h
touchscreen.o touchscreen.ln: /usr/include/linux/posix_types.h /usr/include/linux/stddef.h
touchscreen.o touchscreen.ln: /usr/include/asm/posix_types.h
touchscreen.o touchscreen.ln: /usr/include/asm/posix_types_32.h screen.h terminal.h
keyboard.o keyboard.ln: /usr/include/ctype.h /usr/include/features.h
keyboard.o keyboard.ln: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
keyboard.o keyboard.ln: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
keyboard.o keyboard.ln: /usr/include/bits/types.h /usr/include/bits/typesizes.h
keyboard.o keyboard.ln: /usr/include/endian.h /usr/include/bits/endian.h consolefont.h
keyboard.o keyboard.ln: graphics.h screen.h keyboard.h
