cmake_minimum_required(VERSION 3.18)

# Load vcpkg
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(GLSLD_VCPKG_TRIPLET "x64-windows-static")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(GLSLD_VCPKG_TRIPLET "x64-linux")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    # FIXME: test for x86?
    set(GLSLD_VCPKG_TRIPLET "arm64-osx")
else()
    message(FATAL_ERROR "Failed to infer vcpkg triplet")
endif()

set(VCPKG_TARGET_TRIPLET ${GLSLD_VCPKG_TRIPLET} CACHE STRING "" FORCE)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# NOTE it is important to have project declaration to come after vcpkg config
project(glsld CXX)

# OS and Compiler Config
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(GLSLD_OS_WIN)
    set(GLSLD_OS_WIN 1)
    message(STATUS "[OS] Windows")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions(GLSLD_OS_LINUX)
    set(GLSLD_OS_LINUX 1)
    message(STATUS "[OS] Linux")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    add_compile_definitions(GLSLD_OS_DARWIN)
    set(GLSLD_OS_DARWIN 1)
    message(STATUS "[OS] Darwin")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions(GLSLD_COMPILER_MSVC)
    set(GLSLD_COMPILER_MSVC 1)
    message(STATUS "[Compiler] MSVC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_definitions(GLSLD_COMPILER_GCC)
    set(GLSLD_COMPILER_GCC 1)
    message(STATUS "[Compiler] Gcc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "CLANG")
    add_compile_definitions(GLSLD_COMPILER_CLANG)
    set(GLSLD_COMPILER_CLANG 1)
    message(STATUS "[Compiler] Clang")
endif()

set(CMAKE_CXX_STANDARD 20)

if(GLSLD_COMPILER_MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(-permissive-)
endif()

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")

# GLSL FrontEnd
file(GLOB_RECURSE GLSL_FRONTEND_HEADER_FILE glsl-fe/include/*.h)
file(GLOB_RECURSE GLSL_FRONTEND_SOURCE_FILE glsl-fe/src/*.cpp)

add_library(glsl-fe STATIC ${GLSL_FRONTEND_HEADER_FILE} ${GLSL_FRONTEND_SOURCE_FILE})
target_include_directories(glsl-fe PUBLIC glsl-fe/include)

target_link_libraries(glsl-fe PRIVATE fmt::fmt)

# GLSLD Lexer Generator
file(GLOB_RECURSE GLSLD_LEXGEN_HEADER_FILE glsld-lexgen/include/*.h)
file(GLOB_RECURSE GLSLD_LEXGEN_SOURCE_FILE glsld-lexgen/src/*.cpp)

add_executable(glsld-lexgen ${GLSLD_LEXGEN_HEADER_FILE} ${GLSLD_LEXGEN_SOURCE_FILE})
target_include_directories(glsld-lexgen PUBLIC glsld-lexgen/include)

target_link_libraries(glsld-lexgen PRIVATE glsl-fe)
target_link_libraries(glsld-lexgen PRIVATE fmt::fmt)

# GLSLD
file(GLOB_RECURSE GLSLD_HEADER_FILE glsld/include/*.h)
file(GLOB_RECURSE GLSLD_SOURCE_FILE glsld/src/*.cpp)

add_executable(glsld ${GLSLD_HEADER_FILE} ${GLSLD_SOURCE_FILE})
target_include_directories(glsld PRIVATE glsld/include)

target_link_libraries(glsld PRIVATE glsl-fe)

target_link_libraries(glsld PRIVATE fmt::fmt)
target_link_libraries(glsld PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(glsld PRIVATE ${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS})

# GLSL Processor
file(GLOB_RECURSE GLSLP_HEADER_FILE glslp/include/*.h)
file(GLOB_RECURSE GLSLP_SOURCE_FILE glslp/src/*.cpp)

add_executable(glslp ${GLSLP_HEADER_FILE} ${GLSLP_SOURCE_FILE})
target_include_directories(glslp PRIVATE glslp/include)

target_link_libraries(glslp PRIVATE glsl-fe)

target_link_libraries(glslp PRIVATE fmt::fmt)
target_link_libraries(glslp PRIVATE nlohmann_json::nlohmann_json)