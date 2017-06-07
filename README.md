To compile

@echo off

REM Replace with your own paths!!!!!
set VCVarsAllPath="A:/SYSTEM/Visual Studio 2015/VC/vcvarsall.bat"
set SourceFile="X:\sp_sort\code\win32.c"
set BuildPath="X:\sp_sort\build"

ctime -begin "sp_sort.ctm"
call %VCVarsAllPath% x64

set CommonCompilerFlags=-Zi -MD -nologo -Gm -EHa -wd4311 -wd4312
set CommonLinkerFlags=/incremental:no /opt:ref User32.lib Gdi32.lib OpenGL32.lib

pushd %BuildPath%
cl %CommonCompilerFlags% %SourceFile% /link %CommonLinkerFlags%
set LastError=%ERRORLEVEL%
popd
ctime -end "sp_sort.ctm" %ERRORLEVEL%
