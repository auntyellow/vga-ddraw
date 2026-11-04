cl /DDEBUG=1 /O2 /W3 /MD testdvga.c dvga9x.c /Fetestdvga9x.exe gdi32.lib user32.lib
del *.obj