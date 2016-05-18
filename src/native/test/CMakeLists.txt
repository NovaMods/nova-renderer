cmake_minimum_required(VERSION 3.3)
project(renderer)

include_directories(
        "C:/win-builds/lib/gcc/mingw32/4.8.1/include/c++"
        "C:/win-builds/include"
)

link_directories(
        "C:/win-builds/lib"
        "C:/Users/David/Documents/Nova Renderer/src/native/build/out/Debug"
)

set(SOURCE_FILES
        main.cpp
        shader_test.cpp
        test_utils.cpp
        )

# configure the project
add_executable(nova_test ${SOURCE_FILES})

target_include_directories(nova_test SYSTEM PRIVATE)
target_link_libraries(nova_test nova-renderer.dll)

set_property(TARGET nova_test PROPERTY CXX_STANDARD 14)
set_property(TARGET nova_test PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
