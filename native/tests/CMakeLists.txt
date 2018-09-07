cmake_minimum_required(VERSION 3.8)
project(nova-renderer)

set(BUILD_SHARED_LIBS ON)

set(INSTALL_GTEST OFF)
add_subdirectory(3rdparty/googletest)
include(GoogleTest)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(nova-test-exe src/filesystem_test.cpp src/end_to_end_runner.cpp src/general_test_setup.hpp)
add_dependencies(nova-test-exe nova-renderer)
target_include_directories(nova-test-exe PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rdparty/googletest/googletest/include)

if(WIN32)
    link_directories(SYSTEM "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.16299.0\\um\\x64")
    set(API_LIBS d3d12 dxgi d3dcompiler)
endif()

target_link_libraries(nova-test-exe nova-renderer gtest gtest_main ${API_LIBS} ${COMMON_LINK_LIBS})

gtest_discover_tests(nova-test-exe)
add_test(NAME nova-test COMMAND nova-test-exe)
set(CMAKE_CXX_FLAGS "${NOVA_CXX_FLAGS}")