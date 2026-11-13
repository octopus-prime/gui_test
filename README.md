# gui_test

Small CMake/C++23 desktop GUI demo using Dear ImGui, GLFW, and OpenGL 3.3.
The ImGui and GLFW sources are downloaded by CMake and built as part of this
project; no separate ImGui/GLFW library installation is needed.

## Linux / WSL

On Ubuntu or Debian, install a C++ compiler, CMake, Git, and GLFW's X11/OpenGL
development dependencies:

```sh
sudo apt update
sudo apt install build-essential cmake git libgl1-mesa-dev \
  libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  libxxf86vm-dev
```

In WSL, run the app from a WSLg-enabled environment so a display server is
available. Then build and launch:

```sh
cmake -S . -B build
cmake --build build --parallel
./build/gui_test
```

## Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload,
CMake, and Git. From a Developer PowerShell (or a terminal where the MSVC
compiler is available), run:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\gui_test.exe
```

With a single-configuration generator such as Ninja, the executable is usually
at `build\gui_test.exe`.

The first CMake configure downloads the pinned GLFW and Dear ImGui releases;
subsequent builds use CMake's local dependency cache.
