## DemiEditor

A work in progress lightweight text editor written in C using opengl and freetype that runs out of the box.

As of current, it only supports windows 10+ and hardware compatible with opengl 4.6

![demi screenshot](screenshot.png)

### Table of Contents

- [Project Structure](#project-structure)
- [Icon](#icon)
- [Linux](#linux)
- [Build Instructions](#compilation)
- [Version History](#version-history)

### Project Structure

The overall idea is that the entry point (win_main.c for windows) is limited to doing window handling and operating system specific setup, refering to editor.c for the editor's logic and rendering, out of which editor.c has parts of it abstracted away in subfolders of src

```bash
src/
├── win_main.c         # main function, program loop and window management
├── win_error.c        # debug and error handling for Windows
├── includes.h         # global includes and macros
├── editor.c           # editor logic and rendering
├── character/         # Character handling and buffer system
├── shader/            # Shader compilation and uniform handling
├── object/            # VAO and VBO abstraction
├── cursor/            # Cursor rendering and position handling
├── math/              # 4x4 matrix math functions
resources/
├── fonts/
├── icons/
├── shaders/
```

### Icon

Project's current icon is magic_hat by [mingcute](https://www.mingcute.com/)

### Linux

As of current, linux is not supported. I intend wayland support once the app is feature complete.

### compilation

Ensure that the variables within .bat files are correct for your setup, then launch debug.bat or release.bat depending on whether you want to compile a debug or release version.

requirements:

- c compiler
- glew
- freetype2

#### compiler

no compiler specific code is writen, but not all builds have full support of windows.h (f.e. winlibs gcc build doesn't support modern dpi functions)

I use [LLVM clang](https://clang.llvm.org/), which requires visual studio or it's build tools, more specifically:
- c++ development 
- C++ Clang Compiler for Windows
- MSBuild support for LLVM (clang-cli) toolset

#### glew

I statically link [glew](https://github.com/nigels-com/glew/releases/tag/glew-2.2.0)

#### freetype2

I dynamically link a custom build of [freetype] (if not following my setup, make sure it's a core version of freetype without optional dependencies). My setup requires [cmake](https://cmake.org/).

```
cmake C:\Users\frogger\Downloads\freetype-master -G "Visual Studio 17 2022" -T ClangCL ^
-DBUILD_SHARED_LIBS=ON ^
-DFT_REQUIRE_ZLIB=OFF ^
-DFT_REQUIRE_PNG=OFF ^
-DFT_REQUIRE_BZIP2=OFF ^
-DFT_REQUIRE_HARFBUZZ=OFF ^
-DFT_REQUIRE_BROTLI=OFF ^
-DCMAKE_INSTALL_PREFIX=C:\freetype

cmake --build . --config Release
cmake --install . --config Release
```

### Version History

`0.4` -> changelog:
- ctrl+v
- scrolling text
- moving via mouse
- improved key up/down
- safety checks in to delete char, WM_CHAR default and render text
- skipping rendering and calculations for text above or below the camera
- replacing `cursor_update_position` from editor loop to everywhere the cursor position changes
- minor fixes and improvements

`0.3` -> First working demo. Obscure in features, doesn't actually edit files and limited to keyboard movement.