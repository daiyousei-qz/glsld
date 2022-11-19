cmake_minimum_required(VERSION 3.18)
project(glsld CXX)

add_compile_options(/std:c++20)

file(GLOB_RECURSE SOURCE_FILE src/*.cpp)
file(GLOB_RECURSE HEADER_FILE include/*.h)

add_executable(App ${HEADER_FILE} ${SOURCE_FILE})
target_include_directories(App PRIVATE include/)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(App PRIVATE fmt::fmt)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(App PRIVATE nlohmann_json::nlohmann_json)