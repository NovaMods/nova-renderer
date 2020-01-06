cmake_minimum_required(VERSION 3.12)
project(Rex)

file(GLOB_RECURSE RX_SOURCES "*.cpp" "*.h")

add_library(rex STATIC ${RX_SOURCES})
target_include_directories(rex SYSTEM rex/include)
