echo off

call config.bat

if exist  "%UV4_PATH%" goto start
goto usage

:start
cd release
del *.bin
del *.rom
cd ..
cd ..\boot\tuwanboot\MDK-ARM
call build.bat
cd..\..\..\build
echo %cd%



echo build IMAGE B
del ..\tuwan3g\MDK-ARM\tuwan3g.uvprojx
copy tuwan3gB.uvprojx  ..\tuwan3g\MDK-ARM\tuwan3g.uvprojx
del ..\tuwan3g\Drivers\CMSIS\Device\ST\STM32F4xx\Source\Templates\system_stm32f4xx.c
copy system_stm32f4xxB.c  ..\tuwan3g\Drivers\CMSIS\Device\ST\STM32F4xx\Source\Templates\system_stm32f4xx.c
cd ..\tuwan3g\MDK-ARM
call buildB.bat



cd ..\..\build
echo %cd%
echo build IMAGE A
del ..\tuwan3g\MDK-ARM\tuwan3g.uvprojx
copy tuwan3gA.uvprojx  ..\tuwan3g\MDK-ARM\tuwan3g.uvprojx
del ..\tuwan3g\Drivers\CMSIS\Device\ST\STM32F4xx\Source\Templates\system_stm32f4xx.c
copy system_stm32f4xxA.c  ..\tuwan3g\Drivers\CMSIS\Device\ST\STM32F4xx\Source\Templates\system_stm32f4xx.c
cd ..\tuwan3g\MDK-ARM
call buildA.bat

cd ..\..\build
tuwan3Tools.exe release\tuwanboot.bin release\tuwan3gAv%PACK_VER1%.%PACK_VER2%.%PACK_VER3%.rom release\tuwan3gBv%PACK_VER1%.%PACK_VER2%.%PACK_VER3%.rom %PACK_VER1% %PACK_VER2% %PACK_VER3%

del release\*.rom
copy *.rom release\
del *.rom


copy *.bin release\
del *.bin




pause
goto end

:usage

echo.
echo 1. don't install KeilC
echo 2. path error
echo.
echo.

pause
:end