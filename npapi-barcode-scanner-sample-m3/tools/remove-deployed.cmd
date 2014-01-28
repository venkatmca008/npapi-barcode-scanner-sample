REM delete remote file
pdel "\Program Files\Intermec HTML5 Browser\Plugins\npmyscanner.dll"
REM update remote DLL
pput "..\Windows Mobile 6 Professional SDK (ARMV4I)\Debug\npmyscanner.dll" "\Program Files\Intermec HTML5 Browser\Plugins\npmyscanner.dll"
pput -f ..\samples\MyScanner.htm "\Program Files\Intermec HTML5 Browser\MyScanner.htm"
pput -f ..\samples\MyScanner1.htm "\Program Files\Intermec HTML5 Browser\MyScanner1.htm"
pput -f ..\samples\MyScanner2.htm "\Program Files\Intermec HTML5 Browser\MyScanner2.htm"
