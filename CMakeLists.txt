cmake_minimum_required(VERSION 3.12)
project(nova-renderer)

############################################################
# Fix build type because of possible capitalization issues #
############################################################
if(NOT NOVA_PACKAGE AND CMAKE_BUILD_TYPE)
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
option(NOVA_PACKAGE "Build only the library, nothing else." OFF)

option(NOVA_FORCE_DEBUGGING "Force compiling all the debugging and validation code" OFF)

if(NOVA_ENABLE_EXPERIMENTAL)
    set(CMAKE_LINK_WHAT_YOU_USE TRUE) # Warn about unsued linked libraries
endif()

#########################################################
# Enable tests if not in submodule and debug or if MSVC #
#########################################################
if(NOT NOVA_PACKAGE)
    message(STATUS "Enabling nova tests by default because Nova is not a package")
    set(NOVA_TEST ON)
endif()

############################
# Initialize cmake options #
############################
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/external)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools/cmake" "${3RD_PARTY_DIR}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT NOVA_PACKAGE)
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

# Only install the git hooks if we're not being included in someone else's project
if(NOT NOVA_PACKAGE)
    include(InstallGitHooks)
endif()

set(BUILD_TESTING OFF CACHE BOOL "NO TESTING" FORCE)

#################################
# Setup 3rd party dependencies. #
#################################
include(external)

########################
# Add Nova source code #
########################
set(NOVA_SOURCE
        include/nova_renderer/util/platform.hpp
        include/nova_renderer/util/result.hpp
        include/nova_renderer/util/utils.hpp
        include/nova_renderer/util/container_accessor.hpp
        include/nova_renderer/util/bytes.hpp

        include/nova_renderer/nova_renderer.hpp
        include/nova_renderer/nova_settings.hpp
        include/nova_renderer/renderables.hpp
        include/nova_renderer/renderdoc_app.h
        include/nova_renderer/renderpack_data.hpp
        include/nova_renderer/window.hpp
        include/nova_renderer/constants.hpp
        include/nova_renderer/frame_context.hpp
        include/nova_renderer/renderpack_data_conversions.hpp

        include/nova_renderer/filesystem/filesystem_helpers.hpp
        include/nova_renderer/filesystem/folder_accessor.hpp
        include/nova_renderer/filesystem/virtual_filesystem.hpp

        include/nova_renderer/loading/renderpack_loading.hpp
        include/nova_renderer/loading/shader_includer.hpp

        include/nova_renderer/rhi/forward_decls.hpp
        include/nova_renderer/rhi/command_list.hpp
        include/nova_renderer/rhi/swapchain.hpp
        include/nova_renderer/rhi/rhi_enums.hpp
        include/nova_renderer/rhi/rhi_types.hpp
        include/nova_renderer/rhi/resource_binder.hpp
        include/nova_renderer/rhi/pipeline_create_info.hpp
        include/nova_renderer/rhi/render_device.hpp

        include/nova_renderer/procedural_mesh.hpp
        include/nova_renderer/rendergraph.hpp
        include/nova_renderer/ui_renderer.hpp
        include/nova_renderer/resource_loader.hpp
        include/nova_renderer/camera.hpp
        include/nova_renderer/per_frame_device_array.hpp
        include/nova_renderer/material.hpp

        src/nova_renderer.cpp

        src/rhi/rhi_types.cpp
        src/rhi/render_device.cpp
        src/rhi/swapchain.cpp

        src/rhi/vulkan/vulkan_command_list.cpp
        src/rhi/vulkan/vulkan_command_list.hpp
        src/rhi/vulkan/vulkan_render_device.cpp
        src/rhi/vulkan/vulkan_render_device.hpp
        src/rhi/vulkan/vk_structs.hpp
        src/rhi/vulkan/vulkan_utils.hpp
        src/rhi/vulkan/vulkan_utils.cpp
        src/rhi/vulkan/vulkan_swapchain.cpp
        src/rhi/vulkan/vulkan_swapchain.hpp
        src/rhi/vulkan/vulkan_resource_binder.hpp
        src/rhi/vulkan/vulkan_resource_binder.cpp

        src/filesystem/zip_folder_accessor.hpp
        src/filesystem/regular_folder_accessor.hpp
        src/filesystem/folder_accessor.cpp
        src/filesystem/regular_folder_accessor.cpp
        src/filesystem/zip_folder_accessor.cpp
        src/filesystem/virtual_filesystem.cpp

        src/settings/nova_settings.cpp
		
        src/render_objects/procedural_mesh.cpp
        src/render_objects/uniform_structs.hpp
        src/render_objects/renderables.cpp

        src/renderer/rendergraph.cpp
        src/renderer/ui/ui_renderer.cpp
        src/renderer/builtin/backbuffer_output_pass.hpp
        src/renderer/builtin/backbuffer_output_pass.cpp
        src/renderer/resource_loader.cpp
        src/renderer/camera.cpp
        src/renderer/visibility_cache.hpp
        src/renderer/visibility_cache.cpp
        src/renderer/material_data_buffer.cpp
        src/renderer/material_data_buffer.hpp
        src/renderer/material.cpp
        src/renderer/pipeline_reflection.hpp
        src/renderer/pipeline_reflection.cpp

        src/util/utils.cpp
        src/util/result.cpp
        src/util/bytes.cpp

        src/loading/json_utils.hpp
        src/loading/renderpack/renderpack_loading.cpp
        src/loading/renderpack/shader_includer.cpp
        src/loading/renderpack/renderpack_data.cpp
        src/loading/renderpack/renderpack_validator.cpp
        src/loading/renderpack/renderpack_validator.hpp
        src/loading/renderpack/render_graph_builder.cpp
        src/loading/renderpack/render_graph_builder.hpp
        src/loading/renderpack/renderpack_data_conversions.cpp

        src/debugging/renderdoc.cpp
        src/debugging/renderdoc.hpp

        src/logging/console_log_stream.hpp
        src/logging/console_log_stream.cpp
        
        src/windowing/window.cpp
        )

#################################################################
# Add NTL source code - just until I get INTERFACE libs working #
#################################################################
set(NOVA_SOURCE ${NOVA_SOURCE} ${NTL_SOURCE})

####################
# Add Nova headers #
####################
set(NOVA_HEADERS
        include/nova_renderer/util/platform.hpp
        include/nova_renderer/nova_renderer.hpp
        include/nova_renderer/renderpack_data.hpp
        include/nova_renderer/nova_settings.hpp
        include/nova_renderer/rhi/render_device.hpp
        include/nova_renderer/window.hpp
        include/nova_renderer/util/utils.hpp
        include/nova_renderer/renderables.hpp
        include/nova_renderer/renderdoc_app.h
        include/nova_renderer/util/result.hpp
        )

##################################
# Select platform dependent code #
##################################
set(WINDOWS_NOVA_SOURCE
        include/nova_renderer/util/windows.hpp
        src/util/windows_utils.cpp
        src/util/windows_utils.hpp
        )

set(UNIX_NOVA_SOURCE
        include/nova_renderer/util/x11_but_good.hpp
        src/util/linux_utils.cpp
        src/util/linux_utils.hpp
        )

if(WIN32)
    set(NOVA_SOURCE ${NOVA_SOURCE} ${WINDOWS_NOVA_SOURCE})
    set(OTHER_NOVA_SOURCE ${UNIX_NOVA_SOURCE})
else()
    set(NOVA_SOURCE ${NOVA_SOURCE} ${UNIX_NOVA_SOURCE})
    set(OTHER_NOVA_SOURCE ${WINDOWS_NOVA_SOURCE})
endif()

##########################################
# Set platform dependent compile options #
##########################################
if (UNIX)
    add_compile_options_if_supported(-g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=always -Wpessimizing-move -Wredundant-move)
endif()

if(MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS=1)
endif()

if(NOVA_TREAT_WARNINGS_AS_ERRORS)
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
source_group("nova" FILES ${NOVA_NO_COMPILE} ${3RD_PARTY_SOURCE} ${NOVA_SOURCE} ${NOVA_HEADERS})

##########################
# Setup the nova library #
##########################
add_library(nova-renderer STATIC
        ${NOVA_HEADERS}
        ${NOVA_SOURCE}
        ${3RD_PARTY_SOURCE}
        )
nova_format(nova-renderer "${OTHER_NOVA_SOURCE}")

# Add VULKAN_SDK to the include path
# TODO: Only add if the user is compiling the Vulkan backend

message(STATUS "Miniz headers: ${INSTALL_HEADERS}")

set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)

if(NOVA_FORCE_DEBUGGING) 
    target_compile_definitions(nova-renderer PUBLIC NOVA_DEBUG)
else()
    target_compile_definitions(nova-renderer PUBLIC $<$<CONFIG:Debug>:NOVA_DEBUG>)
endif()

if(NOVA_DEBUG) 
    target_compile_definitions(nova-renderer PUBLIC RX_DEBUG)
endif()

target_include_directories(nova-renderer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $ENV{VULKAN_SDK}/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/miniz	# Miniz doesn't export a MINIZ_HEADERS variable?
        )
target_link_directories(nova-renderer PUBLIC $ENV{VULKAN_SDK}/Lib)

target_include_directories(nova-renderer PRIVATE Vulkan_INCLUDE_DIR)

include(GNUInstallDirs)

##################################
# Add extra warnings if possible #
##################################
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options_if_supported(nova-renderer PUBLIC 
                                        -Wall -Wextra -Wpedantic -Wconversion -Wdefaulted-function-deleted 
                                        -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++98-c++11-c++14-compat 
                                        -Wno-c++98-c++11-c++14-compat-pedantic -Wno-ctad-maybe-unsupported
                                        -Wno-global-constructors -Wno-zero-as-null-pointer-constant)
endif()

add_coverage(nova-renderer)

#########################
# Set libraries to link #
#########################
set(COMMON_LINK_LIBS
        ${CMAKE_DL_LIBS}
        ${Vulkan_LIBRARIES}
        glm::glm
        dxc-wrap 
        minitrace
        miniz
        vma::vma
        rex
        SPIRV-Tools
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-reflect
        spirv-cross-util)

# TODO: Figure out why we need to do this, and add a comment when we know
if(TARGET glfw)
    set(GLFW_TARGET_NAME glfw)
elseif(TARGET glfw3)
    set(GLFW_TARGET_NAME glfw3)
else()
    message(FATAL_ERROR "GLFW target name could not be found")
endif()
set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} ${GLFW_TARGET_NAME})
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/external/glfw/include)

############################
# Platform dependent fixes #
############################
if(WIN32)
    find_package(WindowsSDK)
    get_windowssdk_include_dirs(${WINDOWSSDK_LATEST_DIR} WinSDK_DIRS)

    # MSVC 15.9 broke the Windows SDK by implementing two phase lookup. "/Zc:twoPhase-" disables it.
    add_compile_options_if_supported(/Zc:twoPhase- /permissive /bigobj )

    set_target_properties(nova-renderer PROPERTIES PREFIX "")
    target_include_directories(nova-renderer PRIVATE ${WinSDK_DIRS})
    target_compile_definitions(nova-renderer PUBLIC _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING NOMINMAX)
    target_compile_definitions(nova-renderer PRIVATE NOVA_EXPORT)

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

if(MSVC)
    target_sources(nova-renderer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/tools/vs-visualizers/RexCore.natvis)
endif()

###############################
# Link all required libraries #
###############################
target_link_libraries(nova-renderer PUBLIC ${COMMON_LINK_LIBS})

#############################
# Fix issue with permissive #
#############################
remove_permissive(nova-renderer)

##########################
# Add tests if requested #
##########################
if(NOVA_TEST)
    add_subdirectory(tests)
endif()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()
