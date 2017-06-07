@echo off

set VCVarsAllPath="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
set SourceFile="a:/sp_sort/code/win32.c"
set BuildPath="A:\sp_sort\build"

ctime -begin "sp_sort.ctm"
call %VCVarsAllPath% x64

REM Replace -MD with -MT for less dependencies (IE no VCRuntimeXXX.dll missing)
REM -MD Is faster for build than -MT

set CommonCompilerFlags=-Zi -MD -nologo -Gm -EHa -wd4311 -wd4312
set CommonLinkerFlags=/incremental:no /opt:ref User32.lib Gdi32.lib OpenGL32.lib

pushd %BuildPath%
cl %CommonCompilerFlags% %SourceFile% /link %CommonLinkerFlags%
set LastError=%ERRORLEVEL%
popd
ctime -end "sp_sort.ctm" %ERRORLEVEL%
