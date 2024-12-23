GLSLD is a WIP language server for GLSL. It aims to implement a frontend of a language that's comptible with most GLSL source code and, based on it, a language server that provides IDE features for the GLSL language.

This project includes the following components:
- glsld-core: A core library that provides glsl parser and other utilities.
- glsld-lexgen: A lexer genenerator for the glsl language.
- glsld-wrapper: A standalone wrapper executable around the glsld-core.
- glsld-server: A language server library that implement language server protocol on top of glsld-core.
- glsld: The language server executable.
- glsld-test: The unit test executable.

## Features
Currently, language features below are partially implemented:
- Go to Declaration
- Go to Definition
- Hover
- References
- Document Symbols
- Semantic Tokens
- Inlay Hint
- Completion Proposals
- Signature Help

The following features are planned:
- Rename
- Prepare Rename

## Roadmap
This work is still in a very early development state, but the language server is already working in limited capacity. Planned improvements are:
- Performance optimization for large source file. Current implementation is preliminary and may not perform well for large shaders.
- Support more preprocessors. Currently, many preprocessors are ignored as if they are comments. Need work to properly support them.
- Support constant evaluation of constant expressions.
- Support documentations.

## How to Build

### Dependencies
- A C++23 compiler.
    - MSVC 19.40 or later
    - CLANG 18 or later
- CMake
- vcpkg
    - fmt
    - argparse
    - nlohmann_json
    - BS_thread_pool
    - catch2

### Package Manager
GLSLD uses vcpkg to manage its dependencies. Before compiling this project, please set up vcpkg to your system using the follow commands:
1. `git clone https://github.com/microsoft/vcpkg.git`
2. `cd vcpkg && ./bootstrap-vcpkg.sh`
See also https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash

It is recommended you also set the VCPKG_ROOT environment variable to your vcpkg path so that this project will discover your vcpkg installation automatically. Otherwise, please make sure set CMAKE parameter CMAKE_TOOLCHAIN_FILE to your local vcpkg's tookchain file `vcpkg.cmake`.

### Steps
Assuming your current work folder is at root of the repository, the following commands should build glsld:
```
mkdir build
cd build
cmake ..
cmake --build .
```

GLSLD should generally work in Windows and Linux environment. For other platforms, GLSLD may also work because it is written with cross-platform in mind. However, it might break from time to time.