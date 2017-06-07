@echo off
setlocal enabledelayedexpansion

ctime -begin "sp_sort.ctm"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

set ExeName="sp_sort"
set SourceFile="a:/sp_sort/code/win32.c"

set Includes="A:\projectmanager\Library"

set BuildSpeed="F"
set BuildMode="D"
set BuildDefine=""

REM Replace -MD with -MT for less dependencies (IE no VCRuntimeXXX.dll missing)
REM -MD Is faster for build than -MT

set CommonCompilerFlags=-Zi -MD -nologo -Gm -EHa -wd4311 -wd4312 /I%Includes%
set CommonLinkerFlags=/incremental:no /opt:ref User32.lib Gdi32.lib OpenGL32.lib

if %BuildMode% == "D" (
	set CommonCompilerFlags=%CommonCompilerFlags% -Od
	set BuildDefine=DEBUG_BUILD
) else if %BuildMode% == "R" (
	set CommonCompilerFlags=%CommonCompilerFlags% -O2
	set BuildDefine=RELEASE_BUILD
)

set BuildPath="A:\sp_sort\build"

pushd %BuildPath%
cl %CommonCompilerFlags% -D%BuildDefine% -Fe%ExeName% %SourceFile% /link %CommonLinkerFlags%
set LastError=%ERRORLEVEL%
popd
ctime -end "sp_sort.ctm" %ERRORLEVEL%
