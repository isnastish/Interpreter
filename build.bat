
@echo off

rem /(E|EP) - copies preprocessor output to standard output.
rem /F - set stack size.
rem /Fa - creates as assembly listing file.
rem /O1 - creates small code.
rem /O2 - creates fast code.
rem /Od - disable optimization.
rem /TC - specifies that all source files are C files.
rem /TP - specifies that all source files are C++ files.
rem /w - disable all warnings.
rem /W0 /W1 /W2 /W3 /W4 - set output warning level.
rem /Za - disables some C89 language extensions in C code.
rem /Ze - enables C89 language extensions.
rem /Zi - generates complete debug information.
rem /nologo - disable banner.
rem /FC - display the full path to the source in the error/warning messages.

rem -O2 really speeds up the program.

set compiler_options=/nologo /FC /TC /Zi /FC /D_DEBUG
set linker_options=

if not exist ..\..\build (mkdir ..\..\build)

pushd ..\..\build

cl ^
 %compiler_options% ..\Interpreter\src\main.c /link %linker_options% %*

popd

echo Compilation finished!
