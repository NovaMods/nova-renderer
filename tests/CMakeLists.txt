set(BUILD_SHARED_LIBS ON)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF CACHE BOOL "Build GMOCK shared library" FORCE)

add_subdirectory(3rdparty/googletest)
include(GoogleTest)

find_package(Threads REQUIRED)

# End-to-end test
add_executable(nova-test-end-to-end src/end_to_end_runner.cpp src/general_test_setup.hpp)
target_compile_definitions(nova-test-end-to-end PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-end-to-end PRIVATE nova-renderer gtest gtest_main Threads::Threads)

remove_permissive(nova-test-end-to-end)

# Unit tests
add_executable(nova-test-unit unit_tests/loading/filesystem_test.cpp src/general_test_setup.hpp unit_tests/loading/shaderpack/shaderpack_validator_tests.cpp)
target_compile_definitions(nova-test-unit PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-unit nova-renderer gtest gtest_main Threads::Threads)

remove_permissive(nova-test-unit)

set(CMAKE_CXX_FLAGS "${NOVA_CXX_FLAGS}")

