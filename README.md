A WIP language server for GLSL.

## Roadmap
This work is still in a very early development state, but the language server is already working in limited capacity. Planned improvements are:
- Support preprocessor. Currently, all preprocessors are ignored as if they are comments. Need work to properly support them.
- Support constant evaluation of constant expressions.
- Support documentations.

## How to Build

GLSLD uses vcpkg to manage its dependencies. You'll need vcpkg to build.

Assuming your current work folder is at root of the repository, the following commands should build glsld:
```
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="<vcpkg-toolchain-file>" -DVCPKG_TARGET_TRIPLET="<vcpkg-triplet>"
cmake --build .
```

For Windows users, please use the following vcpkg configuration:
- vcpkg-toolchain-file: `<vcpkg-root>/scripts/buildsystems/vcpkg.cmake`
- vcpkg-triplet: `x64-windows-static`

For Linux users, please use the following vcpkg configuration:
- vcpkg-toolchain-file: `<vcpkg-root>/scripts/buildsystems/vcpkg.cmake`
- vcpkg-triplet: `x64-linux`

For other platforms, GLSLD may also work because it is written with pure standard c++ and a few cross-platform libraies. Please use whatever configuration that works for you.