REM text within <> needs to be replaced with original path on the test PC before using this batch file.

set path=%path%;<Keil_v5 path>\UV4;
cd <Jenkins path>\jobs\Coverity_test\workspace\embedded
rd fmlib /s /q
md fmlib

REM fmlib is to be taken from \\fs2\Public\Host_Software\MotionQ\release_1.x.xx folder
copy <FMLib release path>\FMLib_Embedded.lib fmlib\
cd <Jenkins path>\jobs\Coverity_test\workspace
UV4.exe -r embedded\projects\SH-Xpresso-LPC54102\keil-mdk-build\SH-Xpresso-LPC54102.uvprojx
