@echo off
setlocal enabledelayedexpansion

set COMPILER=clang-cl
set OUTPUT=exe\DemiDebug.exe

set SOURCES=
for /R src %%f in (*.c) do (
    set sources=!sources! "%%f"
)

set INCLUDES=/I"C:\libraries\glew\include" /I"C:\libraries\freetype\include\freetype2" /I"C:\libraries\zlib\include" /I"C:\libraries\libpng\include"
set LIBRARIES="C:\libraries\glew\lib\Release\x64\glew32s.lib" "C:\libraries\freetype\lib\freetype.lib" "C:\libraries\zlib\lib\zs.lib" "C:\libraries\libpng\lib\libpng16_static.lib"
set LINKER_FLAGS=/link opengl32.lib user32.lib shcore.lib comdlg32.lib gdi32.lib
set COMPILER_FLAGS=/W3 /Ox /MT
set DEFINES=/DGLEW_STATIC /DPNG_STATIC /DZLIB_STATIC /D_CRT_SECURE_NO_WARNINGS /Ddemidebug
where %COMPILER% >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: %COMPILER% not found.
    exit /b 1
)

echo Compiling...
%COMPILER% %SOURCES% %COMPILER_FLAGS% -o %OUTPUT% %DEFINES% %INCLUDES% %LIBRARIES% %LINKER_FLAGS%
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b 1
)

endlocal