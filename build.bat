@echo off
setlocal enabledelayedexpansion

set COMPILER=clang
set OUTPUT=exe\DemiEditor.exe

set SOURCES =
for /R src %%f in (*.c) do (
    set sources=!sources! "%%f"
)

set INCLUDES=-I"C:\glew-2.2.0\include" -I"C:\freetype\include\freetype2"
set LIBRARIES="C:\glew-2.2.0\lib\Release\x64\glew32s.lib" -L"C:\freetype\lib"
set FLAGS=-DGLEW_STATIC -lopengl32 -luser32 -lkernel32 -lshcore -lgdi32 -lfreetype

where %COMPILER% >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: clang compiler not found. Please install MinGW or another compatible compiler.
    exit /b 1
)

echo Compiling...
%COMPILER% %INCLUDES% %SOURCES% -o %OUTPUT% %LIBRARIES% %FLAGS%
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b 1
)
  
echo Compilation successful! Run %OUTPUT% it's shortcut or to execute.
endlocal