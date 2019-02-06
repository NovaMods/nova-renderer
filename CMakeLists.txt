cmake_minimum_required(VERSION 3.10)
project(nova-renderer)

################################################################
# Check if nova is in a submodule (nova as a cmake dependency) #
################################################################
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(NOVA_IN_SUBMODULE False)
else()
    set(NOVA_IN_SUBMODULE True)
endif()

############################################################
# Fix build type because of possible capitalization issues #
############################################################
if(NOT NOVA_IN_SUBMODULE AND CMAKE_BUILD_TYPE)
    string(TOLOWER ${CMAKE_BUILD_TYPE} TMP_BUILD_TYPE)
    set(BUILD_TYPE_FIXED_BY_NOVA FALSE)
    if("${TMP_BUILD_TYPE}" STREQUAL "debug" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(ORIGINAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Specifies the build type on single-configuration generators. (Fixed to Debug from '${TMP_BUILD_TYPE}' by nova-renderer)" FORCE)
        set(BUILD_TYPE_FIXED_BY_NOVA TRUE)
    elseif(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(ORIGINAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Specifies the build type on single-configuration generators. (Fixed to Release from '${TMP_BUILD_TYPE}' by nova-renderer)" FORCE)
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
option(NOVA_TEST "Enable tests." OFF)
option(NOVA_ENABLE_EXPERIMENTAL "Enable experimental features, may be in code as well as in the CMake files" OFF)
option(NOVA_TREAT_WARNINGS_AS_ERRORS "Add -Werror flag or /WX for MSVC" OFF)
if(NOVA_ENABLE_EXPERIMENTAL)
    set(CMAKE_LINK_WHAT_YOU_USE TRUE) # Warn about unsued linked libraries
endif()

############################
# Initialize cmake options #
############################
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT ${NOVA_IN_SUBMODULE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

#################################
# Setup our own cmake additions #
#################################
include(ClangFormat)
include(RemovePermissive)
include(CompilerOptionsUtils)
include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/build)
set(BUILD_TESTING OFF CACHE BOOL "NO TESTING" FORCE)

#################################
# Setup 3rd party dependencies. #
#################################
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)
include(${3RD_PARTY_DIR}/3rdparty.cmake)

######################################
# Make dependencies' headers visible #
######################################
include_directories(SYSTEM
        $ENV{VULKAN_SDK}/include
        ${3RD_PARTY_DIR}/RenderDoc-Manager
        ${3RD_PARTY_DIR}/cpptoml/include
        ${3RD_PARTY_DIR}/glslang
        ${3RD_PARTY_DIR}/VulkanMemoryAllocator/src
        ${3RD_PARTY_DIR}/SPIRV-Cross
)

########################
# Add Nova source code #
########################
set(NOVA_SOURCE
        src/platform.hpp
        src/loading/zip_folder_accessor.hpp
        src/loading/regular_folder_accessor.hpp
        src/loading/folder_accessor.hpp
        src/nova_renderer.hpp
        src/nova_renderer.cpp
        src/loading/folder_accessor.cpp
        src/loading/regular_folder_accessor.cpp
        src/loading/zip_folder_accessor.cpp

        src/loading/shaderpack/shaderpack_loading.cpp
        src/loading/shaderpack/shaderpack_loading.hpp
        src/loading/loading_utils.cpp
        src/loading/loading_utils.hpp
        src/loading/shaderpack/shaderpack_data.hpp
        src/loading/shaderpack/shaderpack_data.cpp

        src/settings/nova_settings.cpp
        src/settings/nova_settings.hpp
        src/util/logger.cpp
        src/util/logger.hpp
        src/render_engine/render_engine.hpp
        src/render_engine/window.hpp
        src/loading/shaderpack/json_interop.cpp
        src/loading/shaderpack/json_interop.hpp
        src/loading/json_utils.hpp
        src/util/utils.hpp
        src/util/utils.cpp
        src/render_objects/render_object.cpp
        src/render_objects/render_object.hpp 
        src/loading/shaderpack/shaderpack_validator.cpp 
        src/loading/shaderpack/shaderpack_validator.hpp 
        src/loading/shaderpack/render_graph_builder.cpp 
        src/loading/shaderpack/render_graph_builder.hpp
        src/render_engine/vulkan/vulkan_render_engine.hpp
        src/render_engine/vulkan/vulkan_render_engine.cpp
        src/render_engine/vulkan/vulkan_utils.hpp
        src/render_engine/vulkan/vulkan_type_converters.hpp
        src/render_engine/vulkan/compacting_block_allocator.cpp 
        src/render_engine/vulkan/compacting_block_allocator.hpp 
        src/render_engine/vulkan/vulkan_utils.cpp 
        src/render_engine/vulkan/swapchain.cpp 
        src/render_engine/vulkan/swapchain.hpp 
        src/tasks/task_scheduler.cpp
        src/tasks/task_scheduler.hpp
        src/tasks/task_graph.hpp
        src/tasks/wait_free_queue.hpp
        src/tasks/condition_counter.cpp
        src/tasks/condition_counter.hpp
        src/debugging/renderdoc.cpp
        src/debugging/renderdoc.hpp
        )

##################################
# Select platform dependent code #
##################################
if(WIN32)
    set(NOVA_SOURCE
        ${NOVA_SOURCE}
        src/util/windows_utils.cpp
        src/util/windows_utils.hpp
        src/render_engine/dx12/pipeline_state_object.cpp
        src/render_engine/dx12/pipeline_state_object.hpp
        src/render_engine/dx12/vertex_attributes.hpp
        src/render_engine/dx12/dx12_texture.cpp
        src/render_engine/dx12/dx12_texture.hpp
        src/render_engine/dx12/dx12_utils.cpp
        src/render_engine/dx12/dx12_utils.hpp
        src/render_engine/dx12/win32_window.hpp
        src/render_engine/dx12/win32_window.cpp
        src/render_engine/dx12/dx12_render_engine.cpp
        src/render_engine/dx12/dx12_render_engine.hpp
        src/render_engine/dx12/d3dx12.h
        )
else()
    set(NOVA_SOURCE
        ${NOVA_SOURCE}
        src/render_engine/vulkan/x11_window.cpp
        src/render_engine/vulkan/x11_window.hpp
        src/util/linux_utils.cpp
        src/util/linux_utils.hpp
        )
endif()

##########################
# Setup 3rd party source #
##########################
set(3RD_PARTY_SOURCE
        ${3RD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.cpp)

##########################################
# Set platform dependent compile options #
##########################################
if (UNIX)
    add_compile_options_if_supported(-g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=always)
endif()

if(MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS=1)
endif()

if(NOVA_TREAT_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_compile_options(/WX)
    else()
        add_compile_options(-Werror)
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
source_group("nova" FILES ${NOVA_NO_COMPILE} ${3RD_PARTY_SOURCE} ${NOVA_SOURCE})

##########################
# Setup the nova library #
##########################
add_library(nova-renderer STATIC ${NOVA_SOURCE} ${3RD_PARTY_SOURCE})
# clang_format(nova-renderer NOVA_SOURCE) <= TODO: Uncomment when .clang-format is perfect
target_include_directories(nova-renderer INTERFACE ${JNI_INCLUDE_DIRS})
set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)

##################################
# Add extra warnings if possible #
##################################
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options_if_supported(nova-renderer PUBLIC -Wall -Wextra -Wno-long-long -pedantic -Wdefaulted-function-deleted)
endif()

add_coverage(nova-renderer)

#########################
# Set libraries to link #
#########################
set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS}
                     glm::glm
                     nlohmann::json
                     spirv::headers
                     spirv-cross-core
                     spirv-cross-glsl
                     spirv-cross-hlsl
                     spirv-cross-reflect
                     spirv-cross-util
                     nova-profiler
                     miniz
                     minitrace
                     glslang
                     SPIRV
                     ${SPIRV_SHARED_LIBRARIES}
                     ${Vulkan_LIBRARIES})

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
    target_compile_definitions(nova-renderer PUBLIC NOVA_STD_FILESYSTEM_EXPERIMENTAL=1)
else()
    target_compile_definitions(nova-renderer PUBLIC NOVA_STD_FILESYSTEM=1)
endif()

############################
# Platform dependent fixes #
############################
if(WIN32)
    find_package(WindowsSDK)
    get_windowssdk_include_dirs(${WINDOWSSDK_LATEST_DIR} WinSDK_DIRS)

    # MSVC 15.9 broke the Windows SDK by implementing two phase lookup. "/Zc:twoPhase-" disables it.
    add_compile_options_if_supported(/Zc:twoPhase- /permissive)

    set_target_properties(nova-renderer PROPERTIES PREFIX "")
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} d3d12 dxgi d3dcompiler)
    target_include_directories(nova-renderer PRIVATE ${WinSDK_DIRS})
    target_compile_definitions(nova-renderer PUBLIC _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING NOMINMAX)

    # Visual Studio is stupid. Is it really that hard to use filesystem directories to organize code like literally
    # everyone else?
    foreach(source IN LISTS NOVA_SOURCE)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
    
else()
    # GCC and Clang complain loudly about the #pragma region stuff. This shuts them up.
    target_compile_options_if_supported(nova-renderer PRIVATE -Wno-unknown-pragmas)
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} stdc++fs X11)
endif()

###############################
# Link all required libraries #
###############################
target_link_libraries(nova-renderer PUBLIC ${COMMON_LINK_LIBS})

#############################
# Fix issue with permissive #
#############################
remove_permissive(nova-renderer)

#########################################################
# Enable tests if not in submodule and debug or if MSVC #
#########################################################
if ((CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT NOVA_IN_SUBMODULE) OR MSVC)
    message(STATUS "Enabling nova tests by default because build type is Debug (wont overwrite if set manually)")
    set(NOVA_TEST ON)
endif()

##########################
# Add tests if requested #
##########################
if(NOVA_TEST)
    add_subdirectory(tests)
endif()
