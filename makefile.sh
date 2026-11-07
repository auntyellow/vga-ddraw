i686-w64-mingw32-windres ddraw.rc -o version.o
i686-w64-mingw32-gcc -DNDEBUG -DDIRECTDRAW_VERSION=0x0500 -Iinc -O3 -Wall -Wl,--enable-stdcall-fixup -s src/*.c dvga9x/dvga9x.c -shared -o ddraw.dll ddraw.def version.o -lgdi32 -lwinmm
rm version.o