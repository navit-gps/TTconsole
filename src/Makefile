# Makefile for TomTomConsole (c) Markus Hoffmann V.1.02

# Directories
prefix=/usr/local
exec_prefix=${prefix}

BINDIR=${exec_prefix}/bin
DATADIR=${prefix}/share
LIBDIR=${exec_prefix}/lib
MANDIR=${prefix}/man

all: TTconsole bindist


CC = arm-linux-gcc
DEBUG_DEFINES = -O2
STRIP = arm-linux-strip

OBJS= TTconsole.o consolefont.o screen.o ltext.o fdata.o terminal.o graphics.o \
      touchscreen.o keyboard.o

CSRC=$(OBJS:.o=.c)
HSRC= consolefont.h screen.h terminal.h graphics.h touchscreen.h keyboard.h
DIST= README INSTALL COPYING AUTHORS RELEASE_NOTES Makefile $(CSRC) $(HSRC) \
      terminal.bmp TTconsole.cap TTconsole-wrapper
BINDIST= README RELEASE_NOTES TTconsole terminal.bmp TTconsole.cap \
         TTconsole-wrapper


INCS = -I. -I../ttlinux/include
DIR=TTconsole-1.03
DIR2=$(DIR)-bin

DISTOBJS=$(OBJS)
DIST2=$(DISTOBJS:.o=.c)
DEPSRC=$(DISTOBJS:.o=.c)

.c.o:
	$(CC) $(DEBUG_DEFINES) -c -DLINUX_TARGET -DBARCELONA_TARGET $(INCS) $*.c

TTconsole: $(OBJS) Makefile
	arm-linux-gcc -o $@ $(OBJS) -lm /usr/local/armlinux/3.3.3/arm-linux/lib/libutil.a

clean:
	rm -f *.o *.a *~ TTconsole 
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
	rsync -a RELEASE_NOTES /tmp/$(DIR2)/
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

TTconsole.o TTconsole.ln: /usr/include/stdlib.h /usr/include/features.h
TTconsole.o TTconsole.ln: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
TTconsole.o TTconsole.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stddef.h
TTconsole.o TTconsole.ln: /usr/include/stdio.h /usr/include/bits/types.h
TTconsole.o TTconsole.ln: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
TTconsole.o TTconsole.ln: /usr/include/libio.h /usr/include/_G_config.h
TTconsole.o TTconsole.ln: /usr/include/wchar.h /usr/include/bits/wchar.h
TTconsole.o TTconsole.ln: /usr/include/gconv.h
TTconsole.o TTconsole.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stdarg.h
TTconsole.o TTconsole.ln: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
TTconsole.o TTconsole.ln: /usr/include/unistd.h /usr/include/bits/posix_opt.h
TTconsole.o TTconsole.ln: /usr/include/bits/confname.h /usr/include/errno.h
TTconsole.o TTconsole.ln: /usr/include/bits/errno.h /usr/include/linux/errno.h
TTconsole.o TTconsole.ln: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
TTconsole.o TTconsole.ln: /usr/include/asm-generic/errno-base.h /usr/include/sys/time.h
TTconsole.o TTconsole.ln: /usr/include/time.h /usr/include/bits/time.h
TTconsole.o TTconsole.ln: /usr/include/sys/select.h /usr/include/bits/select.h
TTconsole.o TTconsole.ln: /usr/include/bits/sigset.h /usr/include/sys/stat.h
TTconsole.o TTconsole.ln: /usr/include/bits/stat.h /usr/include/sys/socket.h
TTconsole.o TTconsole.ln: /usr/include/sys/uio.h /usr/include/sys/types.h
TTconsole.o TTconsole.ln: /usr/include/bits/uio.h /usr/include/bits/socket.h
TTconsole.o TTconsole.ln: /usr/include/limits.h
TTconsole.o TTconsole.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/limits.h
TTconsole.o TTconsole.ln: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
TTconsole.o TTconsole.ln: /usr/include/asm/sockios.h /usr/include/string.h
TTconsole.o TTconsole.ln: /usr/include/math.h /usr/include/bits/huge_val.h
TTconsole.o TTconsole.ln: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
TTconsole.o TTconsole.ln: /usr/include/termios.h /usr/include/bits/termios.h
TTconsole.o TTconsole.ln: /usr/include/linux/fb.h /usr/include/asm/types.h
TTconsole.o TTconsole.ln: /usr/include/sys/mman.h /usr/include/bits/mman.h
TTconsole.o TTconsole.ln: /usr/include/fcntl.h /usr/include/bits/fcntl.h
TTconsole.o TTconsole.ln: /usr/include/pty.h /usr/include/sys/ioctl.h
TTconsole.o TTconsole.ln: /usr/include/bits/ioctls.h /usr/include/asm/ioctls.h
TTconsole.o TTconsole.ln: /usr/include/asm/ioctl.h /usr/include/bits/ioctl-types.h
TTconsole.o TTconsole.ln: /usr/include/sys/ttydefaults.h screen.h consolefont.h terminal.h
TTconsole.o TTconsole.ln: touchscreen.h graphics.h
consolefont.o consolefont.ln: /usr/include/stdlib.h /usr/include/features.h
consolefont.o consolefont.ln: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
consolefont.o consolefont.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stddef.h
consolefont.o consolefont.ln: /usr/include/stdio.h /usr/include/bits/types.h
consolefont.o consolefont.ln: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
consolefont.o consolefont.ln: /usr/include/libio.h /usr/include/_G_config.h
consolefont.o consolefont.ln: /usr/include/wchar.h /usr/include/bits/wchar.h
consolefont.o consolefont.ln: /usr/include/gconv.h
consolefont.o consolefont.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stdarg.h
consolefont.o consolefont.ln: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
consolefont.o consolefont.ln: /usr/include/linux/fb.h /usr/include/asm/types.h screen.h
consolefont.o consolefont.ln: consolefont.h
screen.o screen.ln: /usr/include/stdlib.h /usr/include/features.h
screen.o screen.ln: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
screen.o screen.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stddef.h
screen.o screen.ln: /usr/include/stdio.h /usr/include/bits/types.h
screen.o screen.ln: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
screen.o screen.ln: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
screen.o screen.ln: /usr/include/bits/wchar.h /usr/include/gconv.h
screen.o screen.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stdarg.h
screen.o screen.ln: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
screen.o screen.ln: /usr/include/fcntl.h /usr/include/bits/fcntl.h
screen.o screen.ln: /usr/include/sys/types.h /usr/include/time.h /usr/include/unistd.h
screen.o screen.ln: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
screen.o screen.ln: /usr/include/signal.h /usr/include/bits/sigset.h
screen.o screen.ln: /usr/include/bits/signum.h /usr/include/errno.h
screen.o screen.ln: /usr/include/bits/errno.h /usr/include/linux/errno.h
screen.o screen.ln: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
screen.o screen.ln: /usr/include/asm-generic/errno-base.h /usr/include/string.h
screen.o screen.ln: /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h
screen.o screen.ln: /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h
screen.o screen.ln: /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h
screen.o screen.ln: /usr/include/sys/stat.h /usr/include/bits/stat.h
screen.o screen.ln: /usr/include/sys/reboot.h /usr/include/sys/time.h
screen.o screen.ln: /usr/include/bits/time.h /usr/include/sys/select.h
screen.o screen.ln: /usr/include/bits/select.h /usr/include/sys/mman.h
screen.o screen.ln: /usr/include/bits/mman.h /usr/include/linux/fb.h
screen.o screen.ln: /usr/include/asm/types.h /usr/include/linux/ioctl.h screen.h
ltext.o ltext.ln: /usr/include/math.h /usr/include/features.h /usr/include/sys/cdefs.h
ltext.o ltext.ln: /usr/include/gnu/stubs.h /usr/include/bits/huge_val.h
ltext.o ltext.ln: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
ltext.o ltext.ln: /usr/include/string.h
ltext.o ltext.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stddef.h
terminal.o terminal.ln: /usr/include/stdlib.h /usr/include/features.h
terminal.o terminal.ln: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
terminal.o terminal.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stddef.h
terminal.o terminal.ln: /usr/include/stdio.h /usr/include/bits/types.h
terminal.o terminal.ln: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
terminal.o terminal.ln: /usr/include/libio.h /usr/include/_G_config.h
terminal.o terminal.ln: /usr/include/wchar.h /usr/include/bits/wchar.h
terminal.o terminal.ln: /usr/include/gconv.h
terminal.o terminal.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stdarg.h
terminal.o terminal.ln: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
terminal.o terminal.ln: /usr/include/linux/fb.h /usr/include/asm/types.h terminal.h
terminal.o terminal.ln: screen.h consolefont.h
graphics.o graphics.ln: /usr/include/stdlib.h /usr/include/features.h
graphics.o graphics.ln: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
graphics.o graphics.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stddef.h
graphics.o graphics.ln: /usr/include/stdio.h /usr/include/bits/types.h
graphics.o graphics.ln: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
graphics.o graphics.ln: /usr/include/libio.h /usr/include/_G_config.h
graphics.o graphics.ln: /usr/include/wchar.h /usr/include/bits/wchar.h
graphics.o graphics.ln: /usr/include/gconv.h
graphics.o graphics.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stdarg.h
graphics.o graphics.ln: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
graphics.o graphics.ln: /usr/include/dirent.h /usr/include/bits/dirent.h
graphics.o graphics.ln: /usr/include/sys/types.h /usr/include/time.h
graphics.o graphics.ln: /usr/include/sys/stat.h /usr/include/bits/stat.h
graphics.o graphics.ln: /usr/include/unistd.h /usr/include/bits/posix_opt.h
graphics.o graphics.ln: /usr/include/bits/confname.h /usr/include/string.h
graphics.o graphics.ln: /usr/include/linux/fb.h /usr/include/asm/types.h screen.h
graphics.o graphics.ln: graphics.h
touchscreen.o touchscreen.ln: /usr/include/stdlib.h /usr/include/features.h
touchscreen.o touchscreen.ln: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
touchscreen.o touchscreen.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stddef.h
touchscreen.o touchscreen.ln: /usr/include/stdio.h /usr/include/bits/types.h
touchscreen.o touchscreen.ln: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
touchscreen.o touchscreen.ln: /usr/include/libio.h /usr/include/_G_config.h
touchscreen.o touchscreen.ln: /usr/include/wchar.h /usr/include/bits/wchar.h
touchscreen.o touchscreen.ln: /usr/include/gconv.h
touchscreen.o touchscreen.ln: /usr/lib/gcc-lib/i586-suse-linux/3.3.5/include/stdarg.h
touchscreen.o touchscreen.ln: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
touchscreen.o touchscreen.ln: /usr/include/fcntl.h /usr/include/bits/fcntl.h
touchscreen.o touchscreen.ln: /usr/include/sys/types.h /usr/include/time.h
