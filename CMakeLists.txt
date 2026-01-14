cmake_minimum_required(VERSION 3.30)

option(GLSLD_BUILD_WRAPPER "Build glsld-wrapper" ON)
option(GLSLD_BUILD_LANGUAGE_SERVER "Build glsld language server" ON)
option(GLSLD_BUILD_UNIT_TEST "Build glsld unit test" ON)
option(GLSLD_ENABLE_TEST_COVERAGE "Enable coverage test for unit test" OFF)

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
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
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

# TODO: transition to C++20 modules
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(GLSLD_OS_WIN)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    if(GLSLD_COMPILER_MSVC)
        add_compile_options(-permissive-)
    endif()
endif()

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f # 12.1.0
)

CPMAddPackage(
    NAME argparse
    GITHUB_REPOSITORY p-ranav/argparse
    GIT_TAG 3eda91b2e1ce7d569f84ba295507c4cd8fd96910 # v3.2
)

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG 55f93686c01528224f448c19128836e7df245f72 # v3.12.0
)

if (GLSLD_BUILD_LANGUAGE_SERVER OR GLSLD_BUILD_UNIT_TEST)
    CPMAddPackage(
        NAME spdlog
        GITHUB_REPOSITORY gabime/spdlog
        GIT_TAG 79524ddd08a4ec981b7fea76afd08ee05f83755d # v1.17.0
        OPTIONS "SPDLOG_FMT_EXTERNAL ON"
    )

    CPMAddPackage(
        NAME boost_pfr
        GITHUB_REPOSITORY boostorg/pfr
        GIT_TAG 7aa41ed4f5aaa7e52570030e7b28f28cede4f9c1 # boost-1.90.0
        OPTIONS "BUILD_TESTING OFF"
    )

    CPMAddPackage(
        NAME magic_enum
        GITHUB_REPOSITORY Neargye/magic_enum
        GIT_TAG e046b69a3736d314fad813e159b1c192eaef92cd # v0.9.7
    )

    CPMAddPackage(
        NAME stdexec
        GITHUB_REPOSITORY NVIDIA/stdexec
        GIT_TAG 970dbace4ad52a38c9b18665d077f14159792b23 # main as of 2026-01-12
        OPTIONS "STDEXEC_BUILD_TESTS OFF"
                "STDEXEC_BUILD_EXAMPLES OFF"
    )
    if (GLSLD_COMPILER_CLANG)
        # FIXME: Remove when stdexec is fixed
        target_compile_options(stdexec INTERFACE -Wno-deprecated-missing-comma-variadic-parameter)
    endif()

    CPMAddPackage(
        NAME glslang
        GITHUB_REPOSITORY KhronosGroup/glslang
        GIT_TAG b5782e52ee2f7b3e40bb9c80d15b47016e008bc9 # 16.1.0
        OPTIONS "BUILD_EXTERNAL OFF"
                "ENABLE_SPIRV OFF"
                "ENABLE_HLSL OFF"
    )
endif()

if(GLSLD_BUILD_UNIT_TEST)
    CPMAddPackage(
        NAME catch2
        GITHUB_REPOSITORY catchorg/Catch2
        GIT_TAG 88abf9bf325c798c33f54f6b9220ef885b267f4f # v3.12.0
        OPTIONS "CATCH_BUILD_STATIC_LIBRARY ON"
    )

    # Make Catch2 cmake-integration modules available
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

# GLSLD Lexer Generator
file(GLOB_RECURSE GLSLD_LEXGEN_HEADER_FILE glsld-lexgen/include/*.h)
file(GLOB_RECURSE GLSLD_LEXGEN_SOURCE_FILE glsld-lexgen/src/*.cpp)

add_executable(glsld-lexgen ${GLSLD_LEXGEN_HEADER_FILE} ${GLSLD_LEXGEN_SOURCE_FILE})
target_include_directories(glsld-lexgen PUBLIC glsld-core/include)
target_include_directories(glsld-lexgen PUBLIC glsld-lexgen/include)

target_link_libraries(glsld-lexgen PRIVATE fmt::fmt)
target_link_libraries(glsld-lexgen PRIVATE magic_enum::magic_enum)

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
target_link_libraries(glsld-core PUBLIC magic_enum::magic_enum)

# GLSLD Wrapper
if(GLSLD_BUILD_WRAPPER)
    file(GLOB_RECURSE GLSLD_WRAPPER_HEADER_FILE glsld-wrapper/include/*.h)
    file(GLOB_RECURSE GLSLD_WRAPPER_SOURCE_FILE glsld-wrapper/src/*.cpp)

    add_executable(glsld-wrapper ${GLSLD_WRAPPER_HEADER_FILE} ${GLSLD_WRAPPER_SOURCE_FILE})
    target_include_directories(glsld-wrapper PRIVATE glsld-wrapper/include)
    target_include_directories(glsld-wrapper PRIVATE share)

    target_link_libraries(glsld-wrapper PRIVATE glsld-core)

    target_link_libraries(glsld-wrapper PRIVATE argparse::argparse)
    target_link_libraries(glsld-wrapper PRIVATE nlohmann_json::nlohmann_json)
endif()

# GLSLD Language Server
if(GLSLD_BUILD_LANGUAGE_SERVER OR GLSLD_BUILD_UNIT_TEST)
    file(GLOB_RECURSE GLSLD_LANGUAGE_SERVER_HEADER_FILE glsld-server/include/*.h)
    file(GLOB_RECURSE GLSLD_LANGUAGE_SERVER_SOURCE_FILE glsld-server/src/*.cpp)

    add_library(glsld-server STATIC ${GLSLD_LANGUAGE_SERVER_HEADER_FILE} ${GLSLD_LANGUAGE_SERVER_SOURCE_FILE})
    target_include_directories(glsld-server PUBLIC glsld-server/include)

    target_link_libraries(glsld-server PUBLIC glsld-core)

    target_link_libraries(glsld-server PUBLIC spdlog::spdlog)
    target_link_libraries(glsld-server PUBLIC nlohmann_json::nlohmann_json)
    target_link_libraries(glsld-server PUBLIC Boost::pfr)
    target_link_libraries(glsld-server PRIVATE glslang::glslang glslang::glslang-default-resource-limits)
    target_link_libraries(glsld-server PUBLIC STDEXEC::stdexec)
endif()

# GLSLD
if(GLSLD_BUILD_LANGUAGE_SERVER)
    file(GLOB_RECURSE GLSLD_HEADER_FILE glsld/include/*.h)
    file(GLOB_RECURSE GLSLD_SOURCE_FILE glsld/src/*.cpp)

    add_executable(glsld ${GLSLD_HEADER_FILE} ${GLSLD_SOURCE_FILE})
    target_include_directories(glsld PRIVATE glsld/include)
    target_include_directories(glsld PRIVATE share)

    target_link_libraries(glsld PRIVATE glsld-server)

    target_link_libraries(glsld PRIVATE argparse::argparse)
endif()

# GLSLD Unit Test
if(GLSLD_BUILD_UNIT_TEST)
    include(CTest)
    include(Catch)

    file(GLOB_RECURSE GLSLD_TEST_HEADER_FILE glsld-test/include/*.h)
    file(GLOB_RECURSE GLSLD_TEST_SOURCE_FILE glsld-test/src/*.cpp)

    add_executable(glsld-test ${GLSLD_TEST_HEADER_FILE} ${GLSLD_TEST_SOURCE_FILE})
    target_include_directories(glsld-test PRIVATE glsld-test/include)

    target_link_libraries(glsld-test PRIVATE glsld-core glsld-server)

    target_link_libraries(glsld-test PRIVATE Catch2::Catch2WithMain)

    catch_discover_tests(glsld-test)

    if(GLSLD_ENABLE_TEST_COVERAGE AND GLSLD_COMPILER_CLANG)
        target_compile_options(glsld-core PUBLIC -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(glsld-core PUBLIC -fprofile-instr-generate -fcoverage-mapping)
    endif()
endif()