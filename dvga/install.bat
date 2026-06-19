sc stop DVGA
sc delete DVGA
copy dvga.sys %SystemRoot%\System32\drivers
sc create DVGA type= kernel binPath= System32\drivers\dvga.sys start= system
sc start DVGA

sc stop DVGA2
sc delete DVGA2
copy dvga2.sys %SystemRoot%\System32\drivers
sc create DVGA2 type= kernel binPath= System32\drivers\dvga2.sys start= system
sc start DVGA2

sc stop DPAL
sc delete DPAL
copy dpal.sys %SystemRoot%\System32\drivers
sc create DPAL type= kernel binPath= System32\drivers\dpal.sys start= system
sc start DPAL