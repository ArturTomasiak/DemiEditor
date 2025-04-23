@echo off
setlocal enabledelayedexpansion

set COMPILER=gcc
set OUTPUT=exe\DemiEditor.exe

set SOURCES =
for /R src %%f in (*.c) do (
    set sources=!sources! "%%f"
)

set INCLUDES=-I"C:\libraries\glew\include" -I"C:\libraries\freetype2\include\freetype2"
set LIBRARIES="C:\libraries\glew\lib\libglew32.a" -L"C:\libraries\freetype2\lib"
set FLAGS=-lopengl32 -lgdi32 -DGLEW_STATIC -lfreetype -Wall -Werror

where %COMPILER% >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: GCC compiler not found. Please install MinGW or another compatible compiler.
    exit /b 1
)

echo Compiling...
windres rc\ico.rc rc\ico.o
%COMPILER% rc\ico.o %INCLUDES% %SOURCES% -o %OUTPUT% %LIBRARIES% %FLAGS%
echo Creating shortcut...
set SHORTCUT_NAME=DemiEditor.lnk
set SHORTCUT_PATH=%~dp0%SHORTCUT_NAME%
set TARGET_PATH=%~dp0%OUTPUT%
powershell -Command "$WScriptShell = New-Object -ComObject WScript.Shell; $Shortcut = $WScriptShell.CreateShortcut('%SHORTCUT_PATH%'); $Shortcut.TargetPath = '%TARGET_PATH%'; $Shortcut.WorkingDirectory = '%~dp0exe'; $Shortcut.IconLocation = '%TARGET_PATH%'; $Shortcut.Save()"

if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b 1
)
  
echo Compilation successful! Run %OUTPUT% it's shortcut or to execute.
endlocal