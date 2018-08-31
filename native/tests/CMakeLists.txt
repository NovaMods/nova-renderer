cmake_minimum_required(VERSION 3.8)
project(nova-renderer)

set(INSTALL_GTEST OFF)
set(BUILD_SHARED_LIBS ON)

add_subdirectory(3rdparty/googletest)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(nova-test src/filesystem_test.cpp src/end_to_end_runner.cpp)
add_dependencies(nova-test nova-renderer)
target_include_directories(nova-test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rdparty/googletest/googletest/include)
link_directories(SYSTEM "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.16299.0\\um\\x64")
target_link_libraries(nova-test nova-renderer gtest gtest_main d3d12 dxgi d3dcompiler ${COMMON_LINK_LIBS})
set(CMAKE_CXX_FLAGS "${NOVA_CXX_FLAGS}")