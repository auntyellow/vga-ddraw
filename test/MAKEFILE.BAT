cl /O2 /W3 /MD test.c ddraw.lib gdi32.lib user32.lib
del test.obj
rem cl /O2 /W3 /MD dbgview.c
rem del dbgview.obj