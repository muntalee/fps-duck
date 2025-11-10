# Duck FPS

WIP duck fps using some fancy C++ libraries, namely:

* SDL3
* glad
* glm
* stb
* imgui

## Dependencies

* **CMake ≥ 3.16**
* **A C++17 (or newer) compiler**
  * GCC or Clang on Linux
  * Clang (via Xcode tools) on macOS
  * MSVC (Visual Studio) or MinGW on Windows
* **Ninja** (build tool)

## Download

```
git clone --recurse-submodules https://github.com/muntalee/fps-duck
```

## Install

* **Ubuntu / Debian**

  ```bash
  sudo apt update
  sudo apt install build-essential cmake ninja-build libgl1-mesa-dev xorg-dev
  ```

* **Fedora**

  ```bash
  sudo dnf install gcc-c++ cmake ninja-build mesa-libGL-devel libX11-devel libXrandr-devel libXcursor-devel libXi-devel
  ```

* **macOS**

  ```bash
  brew install cmake ninja
  ```

  (OpenGL and Clang come with macOS.)

* **Windows**

  * **Option 1: Visual Studio (MSVC toolchain)**

    * Install [Visual Studio Community](https://visualstudio.microsoft.com/)
      (Choose **Desktop development with C++** workload.)
    * Install [Ninja](https://ninja-build.org/) or with Winget:

      ```powershell
      winget install Ninja-build.Ninja
      ```

  * **Option 2: Scoop + MinGW (GCC toolchain)**

    * Install [Scoop](https://scoop.sh/) (a Windows package manager).
    * Then all other dependencies

      ```powershell
      scoop install mingw
      scoop install ninja cmake
      ```

## Build & Run

The `Makefile` comes bundled with preset commands you may use to compile your project.

```bash
make build     # creates build
make compile   # compiles the build
make run       # build and run
make clean     # cleans up build files / executable
```
