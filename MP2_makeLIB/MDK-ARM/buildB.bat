echo *****************************************
echo compile - link TUWAN IMAGEB channel
del tuwan3g\*.bin
del tuwan3g\*.o
echo %cd%
%UV4_PATH%\UV4 -b tuwan3g.uvprojx
copy /y tuwan3g\tuwan3g.bin %OP_PATH%\tuwan3gBv%PACK_VER1%.%PACK_VER2%.%PACK_VER3%.rom
echo compile - link TUWAN IMAGEA finish