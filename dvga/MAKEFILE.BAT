del dvga.sys
sc stop DVGA
sc delete DVGA
C:\WINDDK\2600.1106\bin\x86\cl.exe /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 /I C:\WINDDK\2600.1106\inc\ddk\wxp /I C:\WINDDK\2600.1106\inc\crt /I C:\WINDDK\2600.1106\inc\wxp dvga.c
C:\WINDDK\2600.1106\bin\x86\link.exe /SUBSYSTEM:NATIVE /DRIVER /ENTRY:DriverEntry /LIBPATH:C:\WINDDK\2600.1106\lib\wxp\i386 /OUT:dvga.sys dvga.obj ntoskrnl.lib hal.lib
del dvga.obj
copy dvga.sys %SystemRoot%\System32\drivers
sc create DVGA type= kernel binPath= System32\drivers\dvga.sys start= system
sc start DVGA

del dvga2.sys
sc stop DVGA2
sc delete DVGA2
C:\WINDDK\2600.1106\bin\x86\cl.exe /c /D_X86_ /DSTD_CALL /DNDEBUG /DDVGA2 /Gz /Ox /W3 /I C:\WINDDK\2600.1106\inc\ddk\wxp /I C:\WINDDK\2600.1106\inc\crt /I C:\WINDDK\2600.1106\inc\wxp dvga.c
C:\WINDDK\2600.1106\bin\x86\link.exe /SUBSYSTEM:NATIVE /DRIVER /ENTRY:DriverEntry /LIBPATH:C:\WINDDK\2600.1106\lib\wxp\i386 /OUT:dvga2.sys dvga.obj ntoskrnl.lib hal.lib
del dvga.obj
copy dvga2.sys %SystemRoot%\System32\drivers
sc create DVGA2 type= kernel binPath= System32\drivers\dvga2.sys start= system
sc start DVGA2

del dpal.sys
sc stop DPAL
sc delete DPAL
C:\WINDDK\2600.1106\bin\x86\cl.exe /c /D_X86_ /DSTD_CALL /DNDEBUG /Gz /Ox /W3 /I C:\WINDDK\2600.1106\inc\ddk\wxp /I C:\WINDDK\2600.1106\inc\crt /I C:\WINDDK\2600.1106\inc\wxp dpal.c
C:\WINDDK\2600.1106\bin\x86\link.exe /SUBSYSTEM:NATIVE /DRIVER /ENTRY:DriverEntry /LIBPATH:C:\WINDDK\2600.1106\lib\wxp\i386 /OUT:dpal.sys dpal.obj ntoskrnl.lib hal.lib
del dpal.obj
copy dpal.sys %SystemRoot%\System32\drivers
sc create DPAL type= kernel binPath= System32\drivers\dpal.sys start= system
sc start DPAL
