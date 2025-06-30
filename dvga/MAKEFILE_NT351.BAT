set INCLUDE=C:\DDK\inc;C:\MSVC20\INCLUDE
set LIB=C:\DDK\lib\i386\FREE;C:\MSVC20\LIB

del dvga.sys
sc stop DVGA
sc delete DVGA
C:\MSVC20\BIN\CL.EXE /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 dvga.c
C:\MSVC20\BIN\LINK.EXE /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /OUT:dvga.sys dvga.obj ntoskrnl.lib hal.lib OLDNAMES.LIB
del dvga.obj
copy dvga.sys %SystemRoot%\System32\drivers
sc create DVGA type= kernel binPath= System32\drivers\dvga.sys start= system
sc start DVGA

del dvga2.sys
sc stop DVGA2
sc delete DVGA2
C:\MSVC20\BIN\CL.EXE /c /D_X86_ /DSTD_CALL /DNDEBUG /DDVGA2 /Gz /Ox /W3 dvga.c
C:\MSVC20\BIN\LINK.EXE /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /OUT:dvga2.sys dvga.obj ntoskrnl.lib hal.lib OLDNAMES.LIB
del dvga.obj
copy dvga2.sys %SystemRoot%\System32\drivers
sc create DVGA2 type= kernel binPath= System32\drivers\dvga2.sys start= system
sc start DVGA2

del dpal.sys
sc stop DPAL
sc delete DPAL
C:\MSVC20\BIN\CL.EXE /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 dpal.c
C:\MSVC20\BIN\LINK.EXE /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /OUT:dpal.sys dpal.obj ntoskrnl.lib hal.lib OLDNAMES.LIB
del dpal.obj
copy dpal.sys %SystemRoot%\System32\drivers
sc create DPAL type= kernel binPath= System32\drivers\dpal.sys start= system
sc start DPAL
