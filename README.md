A WIP language server for GLSL.

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

For other platforms, please use whatever configuration that works for you.