echo *****************************************
echo compile - link TUWAN BOOTLOADER channel
del tuwanboot\*.bin
del tuwanboot\*.o
echo %cd%
%UV4_PATH%\UV4 -b tuwanboot.uvprojx
copy /y tuwanboot\tuwanboot.bin %OP_PATH%
echo compile - link TUWAN BOOTLOADER finish