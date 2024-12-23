cmake_minimum_required(VERSION 3.30)

option(GLSLD_BUILD_WRAPPER "Build glsld-wrapper" ON)
option(GLSLD_BUILD_LANGUAGE_SERVER "Build glsld language server" ON)
option(GLSLD_BUILD_UNIT_TEST "Build glsld unit test" ON)

# Load vcpkg
if(NOT CMAKE_TOOLCHAIN_FILE)
    if(NOT ENV{VCPKG_ROOT})
        message(FATAL_ERROR "Couldn't find vcpkg. Please set VCPKG_ROOT environment variable or pass CMAKE_TOOLCHAIN_FILE")
    endif()

    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(VCPKG_TARGET_TRIPLET "x64-linux")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(VCPKG_TARGET_TRIPLET "arm64-osx")
else()
    message(FATAL_ERROR "Failed to infer vcpkg triplet")
endif()

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
    message(STATUS "[Compiler] GCC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "CLANG")
    add_compile_definitions(GLSLD_COMPILER_CLANG)
    set(GLSLD_COMPILER_CLANG 1)
    message(STATUS "[Compiler] Clang")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(GLSLD_DEBUG)
    set(GLSLD_DEBUG 1)
    message(STATUS "[Build Type] Debug")
else()
    message(STATUS "[Build Type] ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_STANDARD 23)

if(GLSLD_OS_WIN)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    if(GLSLD_COMPILER_MSVC)
        add_compile_options(-permissive-)
    endif()
endif()

find_package(fmt CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")

if(GLSLD_BUILD_UNIT_TEST)
    find_package(Catch2 CONFIG REQUIRED)
endif()

# GLSLD Lexer Generator
file(GLOB_RECURSE GLSLD_LEXGEN_HEADER_FILE glsld-lexgen/include/*.h)
file(GLOB_RECURSE GLSLD_LEXGEN_SOURCE_FILE glsld-lexgen/src/*.cpp)

add_executable(glsld-lexgen ${GLSLD_LEXGEN_HEADER_FILE} ${GLSLD_LEXGEN_SOURCE_FILE})
target_include_directories(glsld-lexgen PUBLIC glsld-core/include)
target_include_directories(glsld-lexgen PUBLIC glsld-lexgen/include)

target_link_libraries(glsld-lexgen PRIVATE fmt::fmt)

add_custom_command(
    OUTPUT Tokenize.generated.cpp
    COMMAND glsld-lexgen "Tokenize.generated.cpp"
    COMMENT "Generating Tokenize.generated.cpp"
    VERBATIM
)

# GLSL Core Library
file(GLOB_RECURSE GLSLD_COMMON_HEADER_FILE glsld-core/include/*.h)
file(GLOB_RECURSE GLSLD_COMMON_SOURCE_FILE glsld-core/src/*.cpp)

add_library(glsld-core STATIC ${GLSLD_COMMON_HEADER_FILE} ${GLSLD_COMMON_SOURCE_FILE} "Tokenize.generated.cpp")
target_include_directories(glsld-core PUBLIC glsld-core/include)

target_link_libraries(glsld-core PUBLIC fmt::fmt)

# GLSLD Language Server
if(GLSLD_BUILD_LANGUAGE_SERVER OR GLSLD_BUILD_UNIT_TEST)
    file(GLOB_RECURSE GLSLD_LANGUAGE_SERVER_HEADER_FILE glsld-server/include/*.h)
    file(GLOB_RECURSE GLSLD_LANGUAGE_SERVER_SOURCE_FILE glsld-server/src/*.cpp)

    add_library(glsld-server STATIC ${GLSLD_LANGUAGE_SERVER_HEADER_FILE} ${GLSLD_LANGUAGE_SERVER_SOURCE_FILE})
    target_include_directories(glsld-server PUBLIC glsld-server/include)

    target_link_libraries(glsld-server PUBLIC glsld-core)

    target_link_libraries(glsld-server PRIVATE nlohmann_json::nlohmann_json)
    target_include_directories(glsld-server PRIVATE ${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS})
endif()

# GLSLD
if(GLSLD_BUILD_LANGUAGE_SERVER)
    file(GLOB_RECURSE GLSLD_HEADER_FILE glsld/include/*.h)
    file(GLOB_RECURSE GLSLD_SOURCE_FILE glsld/src/*.cpp)

    add_executable(glsld ${GLSLD_HEADER_FILE} ${GLSLD_SOURCE_FILE})
    target_include_directories(glsld PRIVATE glsld/include)

    target_link_libraries(glsld PRIVATE glsld-server)

    target_link_libraries(glsld PRIVATE argparse::argparse)
endif()

# GLSLD Wrapper
if(GLSLD_BUILD_WRAPPER)
    file(GLOB_RECURSE GLSLD_WRAPPER_HEADER_FILE glsld-wrapper/include/*.h)
    file(GLOB_RECURSE GLSLD_WRAPPER_SOURCE_FILE glsld-wrapper/src/*.cpp)

    add_executable(glsld-wrapper ${GLSLD_WRAPPER_HEADER_FILE} ${GLSLD_WRAPPER_SOURCE_FILE})
    target_include_directories(glsld-wrapper PRIVATE glsld-wrapper/include)

    target_link_libraries(glsld-wrapper PRIVATE glsld-core)

    target_link_libraries(glsld-wrapper PRIVATE argparse::argparse)
endif()

# GLSLD Unit Test
if(GLSLD_BUILD_UNIT_TEST)
    include(CTest)

    file(GLOB_RECURSE GLSLD_TEST_HEADER_FILE glsld-test/include/*.h)
    file(GLOB_RECURSE GLSLD_TEST_SOURCE_FILE glsld-test/src/*.cpp)

    add_executable(glsld-test ${GLSLD_TEST_HEADER_FILE} ${GLSLD_TEST_SOURCE_FILE})
    target_include_directories(glsld-test PRIVATE glsld-test/include)

    target_link_libraries(glsld-test PRIVATE glsld-core glsld-server)

    target_link_libraries(glsld-test PRIVATE Catch2::Catch2WithMain)

    include(Catch)
    catch_discover_tests(glsld-test)
endif()