#################################
# Setup our own cmake additions #
#################################
include(CompilerOptionsUtils)

# We need threads
find_package(Threads REQUIRED)

###################
# End-to-end test #
###################
add_executable(nova-test-end-to-end src/end_to_end_runner.cpp src/general_test_setup.hpp)
target_compile_definitions(nova-test-end-to-end PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_compile_options_if_supported(nova-test-end-to-end PRIVATE -Wno-unknown-pragmas)
target_link_libraries(nova-test-end-to-end PRIVATE nova-renderer Threads::Threads)
remove_permissive(nova-test-end-to-end)

##############
# Unit tests #
##############
add_executable(nova-test-unit unit_tests/loading/filesystem_test.cpp src/general_test_setup.hpp unit_tests/loading/shaderpack/shaderpack_validator_tests.cpp)
target_compile_definitions(nova-test-unit PRIVATE CMAKE_DEFINED_RESOURCES_PREFIX="${CMAKE_CURRENT_LIST_DIR}/resources/")
target_link_libraries(nova-test-unit nova-renderer gtest_main Threads::Threads)
target_compile_options_if_supported(nova-test-unit PRIVATE -Wno-unknown-pragmas)
remove_permissive(nova-test-unit)

# Reset shared libraries option if changed by us
if(DEFINED BUILD_SHARED_LIBS_ORIGINAL_NOVA)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_ORIGINAL_NOVA} CACHE BOOL "Reset BUILD_SHARED_LIBS value changed by nova to ${BUILD_SHARED_LIBS_ORIGINAL_NOVA}" FORCE)
endif()

if(MSVC)
    # TODO: Non-hacky solution when someone who knows what they're doing can do things
    string(REPLACE "/" "\\" WINDOWS_SAFE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET nova-test-unit
        POST_BUILD
        COMMAND IF EXIST "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Debug\\gtestd.dll"       copy /Y "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Debug\\gtestd.dll" "${WINDOWS_SAFE_BINARY_DIR}\\..\\Debug\\gtestd.dll"
        COMMAND IF EXIST "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Debug\\gtest_maind.dll"  copy /Y "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Debug\\gtest_maind.dll" "${WINDOWS_SAFE_BINARY_DIR}\\..\\Debug\\gtest_maind.dll"
        COMMAND IF EXIST "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Release\\gtest.dll"      copy /Y "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Release\\gtest.dll" "${WINDOWS_SAFE_BINARY_DIR}\\..\\Release\\gtest.dll"
        COMMAND IF EXIST "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Release\\gtest_main.dll" copy /Y "${WINDOWS_SAFE_BINARY_DIR}\\..\\bin\\Release\\gtest_main.dll" "${WINDOWS_SAFE_BINARY_DIR}\\..\\Release\\gtest_main.dll"
        )
endif()
