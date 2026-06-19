windres ddraw.rc -o version.o
gcc -Iinc -O3 -Wall -Wl,--enable-stdcall-fixup -s src\*.c -shared -o ddraw.dll ddraw.def version.o -lgdi32 -lwinmm
del version.o