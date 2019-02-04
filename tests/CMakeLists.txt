##########################################
# Enable shared libraries for nova tests #
##########################################
if(DEFINED BUILD_SHARED_LIBS AND NOT BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Temporarly enable BUILD_SHARED_LIBS" FORCE)
    set(BUILD_SHARED_LIBS_ORIGINAL_NOVA ${BUILD_SHARED_LIBS})
endif()

#####################
# Setup google test #
#####################
set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF CACHE BOOL "Build GMOCK shared library" FORCE)
add_subdirectory(3rdparty/googletest)
include(GoogleTest)

# We need threads
find_package(Threads REQUIRED)

###################
# End-to-end test #
###################
add_executable(nova-test-end-to-end src/end_to_end_runner.cpp src/general_test_setup.hpp)
target_compile_definitions(nova-test-end-to-end PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-end-to-end PRIVATE nova-renderer gtest gtest_main Threads::Threads)
remove_permissive(nova-test-end-to-end)

##############
# Unit tests #
##############
add_executable(nova-test-unit unit_tests/loading/filesystem_test.cpp src/general_test_setup.hpp unit_tests/loading/shaderpack/shaderpack_validator_tests.cpp)
target_compile_definitions(nova-test-unit PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-unit nova-renderer gtest gtest_main Threads::Threads)
remove_permissive(nova-test-unit)

# Reset shared libraries option if changed by us
if(DEFINED BUILD_SHARED_LIBS_ORIGINAL_NOVA)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_ORIGINAL_NOVA} CACHE BOOL "Reset BUILD_SHARED_LIBS value changed by nova to ${BUILD_SHARED_LIBS_ORIGINAL_NOVA}" FORCE)
endif()