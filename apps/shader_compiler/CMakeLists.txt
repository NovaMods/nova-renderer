cmake_minimum_required(VERSION 3.12)
project(nova-renderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/extern/dxc-wrapper/extern/dxc")

set(SHADER_COMPILER_SOURCES
    src/main.cpp "src/shader_includer.cpp" "src/shader_includer.hpp")

add_executable(shader-compiler ${SHADER_COMPILER_SOURCES})
target_link_libraries(shader-compiler PUBLIC dxc-wrap)

