cmake_minimum_required(VERSION 3.8)
project(nova-profiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(SOURCE_FILES
        src/profiler.cpp include/nova/profiler.h)

add_library(nova-profiler STATIC ${SOURCE_FILES})
