set INCLUDE=C:\WINDDK\2600.1106\inc\crt;C:\WINDDK\2600.1106\inc\wxp;C:\WINDDK\2600.1106\inc\ddk\wxp;
set LIB=C:\WINDDK\2600.1106\lib\wxp\i386

C:\WINDDK\2600.1106\bin\x86\cl.exe /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 dvga.c
C:\WINDDK\2600.1106\bin\x86\link.exe /SUBSYSTEM:NATIVE /DRIVER /ENTRY:DriverEntry /OUT:dvga.sys dvga.obj ntoskrnl.lib hal.lib
del dvga.obj

C:\WINDDK\2600.1106\bin\x86\cl.exe /c /D_X86_ /DSTD_CALL /DNDEBUG /DDVGA2 /Gz /Ox /W3 dvga.c
C:\WINDDK\2600.1106\bin\x86\link.exe /SUBSYSTEM:NATIVE /DRIVER /ENTRY:DriverEntry /OUT:dvga2.sys dvga.obj ntoskrnl.lib hal.lib
del dvga.obj

C:\WINDDK\2600.1106\bin\x86\cl.exe /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 dpal.c
C:\WINDDK\2600.1106\bin\x86\link.exe /SUBSYSTEM:NATIVE /DRIVER /ENTRY:DriverEntry /OUT:dpal.sys dpal.obj ntoskrnl.lib hal.lib
del dpal.obj