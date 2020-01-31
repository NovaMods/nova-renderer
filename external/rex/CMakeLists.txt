cmake_minimum_required(VERSION 3.12)
project(Rex)

file(GLOB_RECURSE RX_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.c" "*.h")

add_library(rex STATIC ${RX_SOURCES})

target_include_directories(rex PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(rex SYSTEM PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include" FILES ${RX_SOURCES})
