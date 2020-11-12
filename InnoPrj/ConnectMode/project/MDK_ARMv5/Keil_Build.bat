@echo OFF
? start mshta vbscript:createobject("wscript.shell").run("""%~0"" ::",0)(window.close)&&exit
set KEIL_EXE=D:\keil_V5\UV4\UV4.exe
set UV_PRO_PATH=C:\Users\xiaozhi\Documents\MyProject5\InnoPrj\ConnectMode\project\MDK_ARMv5\VCP_IAP.uvmpw
set LOG_PATH=C:\Users\xiaozhi\Documents\MyProject5\InnoPrj\ConnectMode\project\MDK_ARMv5\
for %%x in ("%LOG_PATH%") do set LOG_PATH=%%~sx
echo .>%LOG_PATH%build_log.txt
%KEIL_EXE% -j0 -b "%UV_PRO_PATH%" -o %LOG_PATH%build_log.txt
if %ERRORLEVEL% NEQ 0 goto BUILD_FAIL
echo                                 [Done]
del /q /s %LOG_PATH%build_log.txt
goto :eof

:BUILD_FAIL
start cmd /k "C:\Users\xiaozhi\Documents\MyProject5\InnoPrj\ConnectMode\project\MDK_ARMv5\Error.bat"







