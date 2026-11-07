windres ddraw.rc -o version.o
gcc -DNDEBUG -DDIRECTDRAW_VERSION=0x0500 -Iinc -Iinc_vc98 -O3 -Wall -Wl,--enable-stdcall-fixup -s src\*.c dvga9x\dvga9x.c -shared -o ddraw.dll ddraw.def version.o -lgdi32 -lwinmm
del version.o