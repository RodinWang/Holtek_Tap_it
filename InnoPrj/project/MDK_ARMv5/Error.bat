@echo OFF
set LOG_PATH =C:\Users\xiaozhi\Documents\MyProject5\InnoPrj\project\MDK_ARMv5\n
type "%LOG_PATH%"build_log.txt
>nul del /q /s "%LOG_PATH%"build_log.txt
pause
exit
