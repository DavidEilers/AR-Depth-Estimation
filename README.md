# AR Depth Estimation
This bachlor project is for creating a depth map for AR applications

## How to build
### Requirements
- compiler for c++ (tested with clang and gcc)
- meson
### How to build
```
meson builddir
meson compile -C builddir
```

## How to run
 You run the excecutable in the builddir.  
 For running the tests use `meson test`.

# External Code
I use other Open Source projects see [Dependencies](DEPENDENCIES.md)

# Code Structure
- External Dependencies are under the ``subprojects`` folder and are downloaded on first build
- My Source Code is under ``src``
  - `[filename].hpp` for src files
  - `[filename]_test.cpp` for the tests
  - `main.cpp` for the main entry point
  - `test_main.cpp` for the Google Test entry point which calls all linked tests (a single `[filename]_test.cpp` and `test_main.cpp` are always linked together) 