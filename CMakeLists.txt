cmake_minimum_required(VERSION 3.18)
project(glsld CXX)

add_compile_options(/std:c++20)

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# GLSL FrontEnd
file(GLOB_RECURSE GLSL_FRONTEND_HEADER_FILE glsl-fe/include/*.h)
file(GLOB_RECURSE GLSL_FRONTEND_SOURCE_FILE glsl-fe/src/*.cpp)

add_library(glsl-fe STATIC ${GLSL_FRONTEND_HEADER_FILE} ${GLSL_FRONTEND_SOURCE_FILE})
target_include_directories(glsl-fe PUBLIC glsl-fe/include)

target_link_libraries(glsl-fe PRIVATE fmt::fmt)

# GLSLD
file(GLOB_RECURSE GLSLD_HEADER_FILE glsld/include/*.h)
file(GLOB_RECURSE GLSLD_SOURCE_FILE glsld/src/*.cpp)

add_executable(glsld ${GLSLD_HEADER_FILE} ${GLSLD_SOURCE_FILE})
target_include_directories(glsld PRIVATE glsld/include)

target_link_libraries(glsld PRIVATE glsl-fe)

target_link_libraries(glsld PRIVATE fmt::fmt)
target_link_libraries(glsld PRIVATE nlohmann_json::nlohmann_json)

# GLSL Processor
file(GLOB_RECURSE GLSLP_HEADER_FILE glslp/include/*.h)
file(GLOB_RECURSE GLSLP_SOURCE_FILE glslp/src/*.cpp)

add_executable(glslp ${GLSLP_HEADER_FILE} ${GLSLP_SOURCE_FILE})
target_include_directories(glslp PRIVATE glslp/include)

target_link_libraries(glslp PRIVATE glsl-fe)

target_link_libraries(glslp PRIVATE fmt::fmt)
target_link_libraries(glslp PRIVATE nlohmann_json::nlohmann_json)