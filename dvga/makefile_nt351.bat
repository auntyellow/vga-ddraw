set INCLUDE=C:\DDK\inc;C:\MSVC20\INCLUDE
set LIB=C:\DDK\lib\i386\FREE;C:\MSVC20\LIB

C:\MSVC20\BIN\CL.EXE /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 dvga.c
C:\MSVC20\BIN\LINK.EXE /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /OUT:dvga.sys dvga.obj ntoskrnl.lib hal.lib OLDNAMES.LIB
del dvga.obj

C:\MSVC20\BIN\CL.EXE /c /D_X86_ /DSTD_CALL /DNDEBUG /DDVGA2 /Gz /Ox /W3 dvga.c
C:\MSVC20\BIN\LINK.EXE /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /OUT:dvga2.sys dvga.obj ntoskrnl.lib hal.lib OLDNAMES.LIB
del dvga.obj

C:\MSVC20\BIN\CL.EXE /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 dpal.c
C:\MSVC20\BIN\LINK.EXE /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /OUT:dpal.sys dpal.obj ntoskrnl.lib hal.lib OLDNAMES.LIB
del dpal.obj