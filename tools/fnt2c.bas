' Little tool to make a .c file out of a fitmap font file.
' (c) Markus Hoffmann 2005
' written in X11-Basic (on linux)
'
'
t$=SPACE$(4096)

BLOAD "fnt/spat-a.fnt",VARPTR(t$)

FOR i=0 to 255
  PRINT "/* 0x";hex$(i,2,2,1);" (";i;") ";
  IF i>31 AND i<=ASC("~")
    PRINT " '";CHR$(i);"'";
  ENDIF
  PRINT " */ ";
  IF 0
    FOR j=0 to 15
      PRINT "0x";hex$(255 AND PEEK(VARPTR(t$)+j*256+i),2,2,1);",";
    NEXT j
    PRINT
  ELSE
    PRINT
    FOR j=0 to 15
      PRINT "0x";hex$(255 AND PEEK(VARPTR(t$)+j*256+i),2,2,1);",  /*";
      tt$=bin$(255 AND PEEK(VARPTR(t$)+j*256+i))
      tt$=REPLACE$(tt$,"0","  ")
      tt$=REPLACE$(tt$,"1","##")
      PRINT tt$;" */"
    NEXT j
    PRINT
  ENDIF
  ' pause 0.2
NEXT i
PRINT "};"
QUIT
