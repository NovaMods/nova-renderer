cmake_minimum_required(VERSION 3.8)
project(nova-renderer)

set(BUILD_SHARED_LIBS ON)

set(INSTALL_GTEST OFF)
add_subdirectory(3rdparty/googletest)
include(GoogleTest)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(nova-test-exe src/filesystem_test.cpp src/end_to_end_runner.cpp src/general_test_setup.hpp src/shaderpack_loading_test.cpp)
add_dependencies(nova-test-exe nova-renderer)
target_include_directories(nova-test-exe PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rdparty/googletest/googletest/include)
target_compile_definitions(nova-test-exe PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-exe nova-renderer gtest gtest_main)

gtest_discover_tests(nova-test-exe)
add_test(NAME nova-test COMMAND nova-test-exe)
set(CMAKE_CXX_FLAGS "${NOVA_CXX_FLAGS}")