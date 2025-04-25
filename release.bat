@echo off
setlocal enabledelayedexpansion

set COMPILER=clang
set OUTPUT=exe\DemiEditor.exe

set SOURCES=
for /R src %%f in (*.c) do (
    set sources=!sources! "%%f"
)

set INCLUDES=-I"C:\glew-2.2.0\include" -I"C:\freetype\include\freetype2"
set LIBRARIES="C:\glew-2.2.0\lib\Release\x64\glew32s.lib" -L"C:\freetype\lib"
set LINKER_FLAGS=-lopengl32 -luser32 -lshcore -lgdi32 -lfreetype
set COMPILER_FLAGS=-std=c23 -fuse-ld=lld -Wvarargs -Wall -Werror
set DEFINES=-DGLEW_STATIC -D_CRT_SECURE_NO_WARNINGS
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