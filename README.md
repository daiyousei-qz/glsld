GLSLD is a language server for GLSL. It aims to implement a frontend of a language that's comptible with most GLSL source code and, based on it, a language server that provides IDE features for the GLSL language.

This project includes the following components:
- glsld-core: A core library that provides glsl parser and other utilities.
- glsld-lexgen: A lexer genenerator for the glsl language.
- glsld-wrapper: A standalone wrapper executable that is built around the glsld-core.
- glsld-server: A language server library that implement language server protocol on top of glsld-core.
- glsld: The language server executable built on top of the glsld-server.
- glsld-test: The unit test executable built on top of the glsld-server.

## Features
Currently, language features below are (partially) implemented:
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

## How to Build

### Dependencies
- A C++23 compiler.
    - CLANG 19 or later
- CMake
- vcpkg
    - fmt
    - spdlog
    - argparse
    - nlohmann_json
    - BS_thread_pool
    - catch2
    - boost-pfr
    - magic-enum

### Package Manager
GLSLD uses vcpkg to manage its dependencies. It is recommended that you use the vcpkg from git submodule of this repository.

### Steps
We assume you have cloned this repository fresh and your current work folder is at root of the repository.

First, use the following command to prepare the vcpkg:
```
git submodule update --init
./external/vcpkg/bootstrap-vcpkg
```

Now that the vcpkg is ready, you can build the debug build of the project with the following command:
```
cmake --preset Debug
cmake --build build
```

GLSLD should generally work in Windows and Linux environment. For other platforms, GLSLD may also work because it is written with cross-platform in mind. However, it might break from time to time.