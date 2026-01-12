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
- Folding Range

The following features are planned:
- Rename
- Prepare Rename

## How to Build

### Dependencies

Before we start, make sure you have the following dependencies installed:
- A C++2c compiler.
    - CLANG 19 or later
- CMake
    - 3.30 or later
- Ninja

### Steps

First, let's pull the repository:
```
git clone https://github.com/daiyousei-qz/glsld
cd glsld
```

Now that the repository is ready, you can build the project with the following command:
```
cmake --preset Debug
cmake --build build --parallel
```

Notably, we are using CMake presets to simplify the build process. The example above uses the `Debug` preset, which is configured to build the project in debug mode. You can also find other presets in the `CMakePresets.json` file.