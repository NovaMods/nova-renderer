cmake_minimum_required(VERSION 3.3)
project(renderer)

add_subdirectory(src)
add_subdirectory(test)

include_directories(
        "C:/win-builds/lib/gcc/mingw32/4.8.1/include/c++"
        "C:/win-builds/include"
)

link_directories(
        "C:/win-builds/lib"
)

set(CMAKE_BUILD_TYPE Debug)
