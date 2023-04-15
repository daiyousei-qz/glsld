GLSLD is a WIP language server for GLSL. It aims to implement a frontend of a language that's comptible with most GLSL source code and, based on it, a language server that provides intellisense experience for the GLSL language.

## Roadmap
This work is still in a very early development state, but the language server is already working in limited capacity. Planned improvements are:
- Performance optimization for large source file. Current implementation is preliminary and may not perform well for large shaders.
- Support more preprocessors. Currently, many preprocessors are ignored as if they are comments. Need work to properly support them.
- Support constant evaluation of constant expressions.
- Support documentations.

## How to Build

### Dependencies
- A C++20 compiler.
    - MSVC 2022 or later
    - GCC 12 or later
- CMake
- vcpkg
    - fmt
    - nlohmann_json
    - BS_thread_pool

### Package Manager
GLSLD uses vcpkg to manage its dependencies. Before compiling this project, please either:
- Set CMAKE parameter CMAKE_TOOLCHAIN_FILE to your local vcpkg's tookchain file `vcpkg.cmake`
- Use `git submodule init --recursive` to clone a copy of vcpkg that's referenced by this repository.

### Steps
Assuming your current work folder is at root of the repository, the following commands should build glsld:
```
mkdir build
cd build
cmake ..
cmake --build .
```

GLSLD should generally work in Windows and Linux environment. For other platforms, GLSLD may also work because it is written with cross-platform in mind. However, it might break from time to time.