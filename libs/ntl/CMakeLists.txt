cmake_minimum_required(VERSION 3.12)
project(nova-renderer)


############################################################
# Fix build type because of possible capitalization issues #
############################################################
if(NOT NTL_PACKAGE AND CMAKE_BUILD_TYPE)
    string(TOLOWER ${CMAKE_BUILD_TYPE} TMP_BUILD_TYPE)
    set(BUILD_TYPE_FIXED_BY_NOVA FALSE)
    if("${TMP_BUILD_TYPE}" STREQUAL "debug" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(ORIGINAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Specifies the build type on single-configuration generators. (Fixed to Debug from '${TMP_BUILD_TYPE}' by ntl)" FORCE)
        set(BUILD_TYPE_FIXED_BY_NOVA TRUE)
    elseif(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(ORIGINAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Specifies the build type on single-configuration generators. (Fixed to Release from '${TMP_BUILD_TYPE}' by ntl)" FORCE)
        set(BUILD_TYPE_FIXED_BY_NOVA TRUE)
    endif()

    if(BUILD_TYPE_FIXED_BY_NOVA)
        message(WARNING "CMAKE_BUILD_TYPE build type has been fixed to ${CMAKE_BUILD_TYPE} from ${ORIGINAL_BUILD_TYPE}")
    endif()
elseif(NOT CMAKE_BUILD_TYPE AND NOT MSVC)
    set(CMAKE_BUILD_TYPE "Release")
endif()

#############################
# Options for nova's builds #
#############################
option(NTL_TEST "Enable tests." OFF)
option(NTL_ENABLE_EXPERIMENTAL "Enable experimental features, may be in code as well as in the CMake files" OFF)
option(NTL_TREAT_WARNINGS_AS_ERRORS "Add -Werror flag or /WX for MSVC" OFF)
option(NTL_PACKAGE "Build only the library, nothing else." OFF)

if(NTL_ENABLE_EXPERIMENTAL)
    set(CMAKE_LINK_WHAT_YOU_USE TRUE) # Warn about unsued linked libraries
endif()

#########################################################
# Enable tests if not in submodule and debug or if MSVC #
#########################################################
if(NOT NTL_PACKAGE)
    message(STATUS "Enabling nova tests by default because NTL is not a package")
    set(NTL_TEST ON)
endif()

############################
# Initialize cmake options #
############################
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../tools/cmake" "${3RD_PARTY_DIR}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT NTL_PACKAGE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

#################################
# Setup our own cmake additions #
#################################

include(ClangFormat)
include(ClangTidy)
include(RemovePermissive)
include(CompilerOptionsUtils)
include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)

set(BUILD_TESTING OFF CACHE BOOL "NO TESTING" FORCE)

########################
# Add Nova source code #
########################
set(NTL_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ntl/result.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/include/ntl/vector.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/include/ntl/stream.hpp
        PARENT_SCOPE
        )

##################################
# Select platform dependent code #
##################################

if(WIN32)
    set(NTL_SOURCE ${NTL_SOURCE} ${WINDOWS_NTL_SOURCE})
    set(OTHER_NTL_SOURCE ${UNIX_NTL_SOURCE})
else()
    set(NTL_SOURCE ${NTL_SOURCE} ${UNIX_NTL_SOURCE})
    set(OTHER_NTL_SOURCE ${WINDOWS_NTL_SOURCE})
endif()

##########################################
# Set platform dependent compile options #
##########################################
if (UNIX)
    add_compile_options_if_supported(-g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=always)
endif()

if(MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS=1)
endif()

if(NTL_TREAT_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_compile_options(/WX)
    else()
        # TODO: Consider removing the -Wno-error's when we are in a more complete state
        add_compile_options(-Werror -Wno-error=unused-variable -Wno-error=unused-but-set-variable
                -Wno-error=unused-parameter)
    endif()
endif()

macro(add_coverage target)
    if(NOVA_COVERAGE)
        if(MSVC)
            message(FATAL_ERROR "Coverage not supported on MSVC.")
        else()
            target_compile_options(${target} PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
            target_link_libraries(${target} PRIVATE -g --coverage)
        endif()
    endif()
endmacro()

##########################################
# Setup required options for backtracing #
##########################################
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_options_if_supported(-g -rdynamic)
    message(STATUS "Enabling symbol output")
endif()

#################
# Organize code #
#################
# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("ntl" FILES ${NTL_NO_COMPILE} ${3RD_PARTY_SOURCE} ${NTL_SOURCE})

##########################
# Setup the NTL library #
##########################
add_library(ntl INTERFACE)

nova_format(ntl "${OTHER_NTL_SOURCE}")

target_include_directories(ntl INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_sources(ntl INTERFACE "$<BUILD_INTERFACE:${NTL_SOURCE}>")

include(GNUInstallDirs)

install(
    TARGETS ntl
    EXPORT ntl-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
)

install(
    EXPORT ntl-config DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ntl
    NAMESPACE ntl::
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ntl
    FILES_MATCHING PATTERN "*.hpp*"
)

##################################
# Add extra warnings if possible #
##################################
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options_if_supported(ntl PUBLIC -Wall -Wextra -Wpedantic -Wconversion -Wdefaulted-function-deleted)
endif()

add_coverage(ntl)

##############################
# Find C++ filesystem header #
##############################
# On some toolchains its still <experimental/filesystem> while on others its already <filesystem>
check_include_file_cxx("filesystem" CXX_STD_FILESYSTEM)
if(NOT CXX_STD_FILESYSTEM)
    check_include_file_cxx("experimental/filesystem" CXX_STD_FILESYSTEM_EXPERIMENTAL)
    if(NOT CXX_STD_FILESYSTEM_EXPERIMENTAL)
        message(FATAL_ERROR "Failed to find a filesystem header for C++")
    endif()
    target_compile_definitions(ntl INTERFACE NTL_STD_FILESYSTEM_EXPERIMENTAL=1)
else()
    target_compile_definitions(ntl INTERFACE NTL_STD_FILESYSTEM=1)
endif()

############################
# Platform dependent fixes #
############################
if(WIN32)
    find_package(WindowsSDK)
    get_windowssdk_include_dirs(${WINDOWSSDK_LATEST_DIR} WinSDK_DIRS)

    # MSVC 15.9 broke the Windows SDK by implementing two phase lookup. "/Zc:twoPhase-" disables it.
    add_compile_options_if_supported(/Zc:twoPhase- /permissive /bigobj )

    target_include_directories(ntl INTERFACE ${WinSDK_DIRS})
    target_compile_definitions(ntl INTERFACE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING NOMINMAX)
	target_compile_definitions(ntl INTERFACE NTL_EXPORT)

    # Visual Studio is stupid. Is it really that hard to use filesystem directories to organize code like literally
    # everyone else?
    foreach(source IN LISTS NTL_SOURCE)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
else()
    # GCC and Clcomplain loudly about the #pragma region stuff. This shuts them up.
    target_compile_options_if_supported(ntl PRIVATE -Wno-unknown-pragmas)
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} stdc++fs X11)
endif()

##########################
# Add tests if requested #
##########################
if(NTL_TEST)
	# TODO: Write tests
    # add_subdirectory(tests)
endif()
