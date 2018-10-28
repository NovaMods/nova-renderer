cmake_minimum_required(VERSION 3.8)
project(nova-renderer)

set(BUILD_SHARED_LIBS ON)

set(INSTALL_GTEST OFF)
add_subdirectory(3rdparty/googletest)
include(GoogleTest)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# End-to-end test
add_executable(nova-test-end-to-end src/end_to_end_runner.cpp src/general_test_setup.hpp)
add_dependencies(nova-test-end-to-end nova-renderer)
target_include_directories(nova-test-end-to-end PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rdparty/googletest/googletest/include)
target_compile_definitions(nova-test-end-to-end PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-end-to-end nova-renderer gtest gtest_main)

add_test(NAME nova-end-to-end-test COMMAND nova-test-end-to-end)

# Unit tests
add_executable(nova-test-unit unit_tests/loading/filesystem_test.cpp src/general_test_setup.hpp unit_tests/loading/shaderpack/shaderpack_validator_tests.cpp)
add_dependencies(nova-test-unit nova-renderer)
target_include_directories(nova-test-unit PRIVATE ${CMAKE_CURRENT_LIST_DIR}/3rdparty/googletest/googletest/include)
target_compile_definitions(nova-test-unit PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-unit nova-renderer gtest gtest_main)

gtest_discover_tests(nova-test-unit)
add_test(NAME nova-unit-tests COMMAND nova-test-unit)

set(CMAKE_CXX_FLAGS "${NOVA_CXX_FLAGS}")