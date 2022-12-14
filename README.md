# AR Depth Estimation
This bachlor project is for creating a depth map for AR applications

## How to build

## Windows build
### Requirements
- clang
- meson
- C++ standard library tested with Visual Studio stdlib

### Install
1. Install git and clone the repository `git clone https://gitlab.hrz.tu-chemnitz.de/daei--tu-chemnitz.de/ardepthestimation.git` or download as archive
### How to build
1. Setup the build directory  ``meson setup builddir``
2. Compile the code ``meson compile -C builddir``
2. For all builds afer this you only run ``meson compile -C buildir``
3. Copy these files into ``builddir/src``
    * ``subprojects/openvr/bin/win64/openvr_api.dll``
    * ``builddir/subprojects/glfw/glad.dll``
    * ``builddir/subprojects/glfw/glfw-3.dll``
    * Optionally  the ``assets`` folder

### How to run
Run the executable from this directory e.g. ``.\builddir\src\main.exe``.
If you copied the assets folder you can also run the executable directly from it's folder, but be aware that you need to manually update the assets folder if you update it's contents!


## WSL build
### Requirements
- compiler for c++ (tested with MingW64-gcc)
- meson
- WSL (Windows Subsystem for Linux)

### Install Requirements
1. Install git and clone the repository `git clone https://gitlab.hrz.tu-chemnitz.de/daei--tu-chemnitz.de/ardepthestimation.git`
2. Install WSL see https://docs.microsoft.com/en-us/windows/wsl/install
3. Install Debian WSL
4. Startup Debian WSL and go into the arDepthEstimation folder
5. Install Requirements see [Requirements WSL](#requirements-wsl)
### Requirements WSL
```
sudo apt update
sudo apt install meson clang cmake pkg-config xorg-dev mingw-w64 mingw-w64-tools git
```
### How to build
1. For the first build run:
```
meson setup builddir --crossfile windows_cross_file.txt
meson compile -C builddir
```
2. For all builds afer this you only need:
```
meson compile -C buildir
```

### How to run
Run the executable from this directory e.g. ``./builddir/src/main.exe``.

- If you run the program directly from the directory be aware that the ``assets`` folder will only be copied on the first creation of ``builddir``.
Thus if you update the assets folder you must also update ``buildir/src/assets``.
 
For running the tests use `meson test -C builddir`.
- You must close the opening Window for the test to pass.

# External Code
I use other Open Source projects see [Dependencies](DEPENDENCIES.md)

# Code Structure
- External Dependencies are under the ``subprojects`` folder and are downloaded on first build
- My Source Code is under ``src``
  - `[filename].hpp` for src files
  - `[filename]_test.cpp` for the tests
  - `main.cpp` for the main entry point
  - `test_main.cpp` for the Google Test entry point which calls all linked tests (a single `[filename]_test.cpp` and `test_main.cpp` are always linked together) 