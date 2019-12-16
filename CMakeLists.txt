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

option(NOVA_ENABLE_VULKAN_RHI "Compile the Vulkan RHI backend" ON)
option(NOVA_ENABLE_D3D12_RHI "Compile the D3D12 RHI backend" ON)
option(NOVA_ENABLE_OPENGL_RHI "Compile the OpenGL 3 backend" ON)

option(NOVA_FORCE_DEBUGGING "Force compiling all the debugging and validation code" OFF)

if(NOVA_ENABLE_D3D12_API AND NOT WIN32)
    message(WARNING "Nova can only compile its D3D12 RHI backend on Windows 10. You're now on Windows 10, so I'm disabling the D3D12 RHI backend")
    set(NOVA_ENABLE_D3D12_RHI_BACKEND OFF)
endif()

if(NOT NOVA_ENABLE_VULKAN_RHI AND NOT VULKAN_ENABLE_D3D12_RHI AND NOT NOVA_ENABLE_OPENGL_RHI)
    message(ERROR "You must enable at least one RHI backend")
endif()

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
        include/nova_renderer/util/filesystem.hpp
        include/nova_renderer/util/platform.hpp
        include/nova_renderer/util/result.hpp
        include/nova_renderer/util/utils.hpp
        include/nova_renderer/util/container_accessor.hpp

        include/nova_renderer/nova_renderer.hpp
        include/nova_renderer/nova_settings.hpp
        include/nova_renderer/renderables.hpp
        include/nova_renderer/renderdoc_app.h
        include/nova_renderer/shaderpack_data.hpp
        include/nova_renderer/window.hpp
        include/nova_renderer/bytes.hpp
        include/nova_renderer/allocation_strategy.hpp
        include/nova_renderer/allocation_structs.hpp
        include/nova_renderer/bytes.hpp
        include/nova_renderer/polyalloc.hpp
        include/nova_renderer/constants.hpp
        include/nova_renderer/frame_context.hpp
        include/nova_renderer/rhi/forward_decls.hpp
        include/nova_renderer/rhi/command_list.hpp
        include/nova_renderer/rhi/rhi_enums.hpp
        include/nova_renderer/rhi/rhi_types.hpp
        include/nova_renderer/rhi/render_engine.hpp
        include/nova_renderer/rhi/device_memory_resource.hpp
        include/nova_renderer/frontend/procedural_mesh.hpp
        include/nova_renderer/frontend/rendergraph.hpp
        include/nova_renderer/frontend/ui_renderer.hpp

        src/nova_renderer.cpp

        src/loading/zip_folder_accessor.hpp
        src/loading/regular_folder_accessor.hpp
        src/loading/folder_accessor.hpp
        src/loading/folder_accessor.cpp
        src/loading/regular_folder_accessor.cpp
        src/loading/zip_folder_accessor.cpp
        src/loading/loading_utils.cpp
        src/loading/loading_utils.hpp
        src/loading/json_utils.hpp

        src/settings/nova_settings.cpp
		
        src/render_objects/procedural_mesh.cpp
        src/render_objects/uniform_structs.hpp
        src/render_objects/renderables.cpp
        src/renderer/rendergraph.cpp
        src/renderer/ui/ui_renderer.cpp

        src/util/logger.cpp
        src/util/logger.hpp
        src/util/utils.cpp
        src/util/result.cpp

        src/loading/shaderpack/shaderpack_loading.cpp
        src/loading/shaderpack/shaderpack_loading.hpp
        src/loading/shaderpack/shaderpack_data.cpp
        src/loading/shaderpack/json_interop.cpp
        src/loading/shaderpack/json_interop.hpp
        src/loading/shaderpack/shaderpack_validator.cpp
        src/loading/shaderpack/shaderpack_validator.hpp
        src/loading/shaderpack/render_graph_builder.cpp
        src/loading/shaderpack/render_graph_builder.hpp

        src/render_engine/rhi_types.cpp
        src/render_engine/render_engine.cpp
        src/render_engine/swapchain.cpp

        src/tasks/task_scheduler.cpp
        src/tasks/task_scheduler.hpp
        src/tasks/task_graph.hpp
        src/tasks/wait_free_queue.hpp
        src/tasks/condition_counter.cpp
        src/tasks/condition_counter.hpp

        src/debugging/renderdoc.cpp
        src/debugging/renderdoc.hpp
        
        src/windowing/window.cpp

        src/memory/device_memory_resource.cpp
        src/memory/block_allocation_strategy.hpp
        src/memory/bump_point_allocation_strategy.hpp
        src/memory/mallocator.hpp
        src/memory/system_memory_allocator.hpp
        src/memory/block_allocation_strategy.cpp
        src/memory/bump_point_allocation_strategy.cpp
        src/memory/bytes.cpp
        src/memory/mallocator.cpp
        src/memory/system_memory_allocator.cpp
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
        include/nova_renderer/shaderpack_data.hpp
        include/nova_renderer/nova_settings.hpp
        include/nova_renderer/rhi/render_engine.hpp
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

#########################################
# Which RHI backends should we compile? #
#########################################
if(NOVA_ENABLE_VULKAN_RHI)
    set(NOVA_SOURCE
            ${NOVA_SOURCE}
            src/render_engine/vulkan/vulkan_command_list.cpp
            src/render_engine/vulkan/vulkan_command_list.hpp
            src/render_engine/vulkan/vulkan_render_engine.cpp
            src/render_engine/vulkan/vulkan_render_engine.hpp
            src/render_engine/vulkan/vk_structs.hpp
            src/render_engine/vulkan/vulkan_utils.hpp
            src/render_engine/vulkan/vulkan_utils.cpp
            src/render_engine/vulkan/vulkan_swapchain.cpp
            src/render_engine/vulkan/vulkan_swapchain.hpp
            )
endif()

if(NOVA_ENABLE_D3D12_RHI)
    set(WINDOWS_NOVA_SOURCE
            ${WINDOWS_NOVA_SOURCE}
            src/render_engine/dx12/dx12_utils.cpp
            src/render_engine/dx12/dx12_utils.hpp
            src/render_engine/dx12/d3dx12.h
            src/render_engine/dx12/dx12_command_list.cpp
            src/render_engine/dx12/dx12_command_list.hpp
            src/render_engine/dx12/dx12_structs.hpp
            src/render_engine/dx12/dx12_swapchain.cpp
            src/render_engine/dx12/dx12_swapchain.hpp
            src/render_engine/dx12/dx12_render_engine.cpp
            src/render_engine/dx12/dx12_render_engine.hpp
            )
endif()

if(NOVA_ENABLE_OPENGL_RHI)
    set(NOVA_SOURCE
            ${NOVA_SOURCE}
            src/render_engine/gl3/gl3_render_engine.cpp
            src/render_engine/gl3/gl3_render_engine.hpp
            src/render_engine/gl3/gl3_command_list.cpp
            src/render_engine/gl3/gl3_command_list.hpp
            src/render_engine/gl3/gl3_swapchain.cpp
            src/render_engine/gl3/gl3_swapchain.hpp
            src/render_engine/gl3/KHR/khrplatform.h
            src/render_engine/gl3/glad/glad.h
            src/render_engine/gl3/glad.c
            src/render_engine/gl3/gl3_structs.hpp
            )
endif()

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

message(STATUS "FMT headers: ${FMT_HEADERS}")
message(STATUS "Miniz headers: ${INSTALL_HEADERS}")

set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)
target_compile_definitions(nova-renderer PUBLIC MTR_ENABLED)
target_compile_definitions(nova-renderer PUBLIC FMT_STRING_ALIAS=1)

if(NOVA_FORCE_DEBUGGING) 
    target_compile_definitions(nova-renderer PUBLIC NOVA_DEBUG)
else()
    target_compile_definitions(nova-renderer PUBLIC $<$<CONFIG:Debug>:NOVA_DEBUG>)
endif()

target_include_directories(nova-renderer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $ENV{VULKAN_SDK}/Include
        ${FMT_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/external/miniz	# Miniz doesn't export a MINIZ_HEADERS variable?
        )
target_link_directories(nova-renderer PUBLIC $ENV{VULKAN_SDK}/Lib)

# Compilation definitions indicating enabled backends
if(NOVA_ENABLE_VULKAN_RHI)
    target_compile_definitions(nova-renderer PUBLIC NOVA_VULKAN_RHI)
    target_include_directories(nova-renderer PRIVATE Vulkan_INCLUDE_DIR)
endif()

if(NOVA_ENABLE_D3D12_RHI)
    target_compile_definitions(nova-renderer PUBLIC NOVA_D3D12_RHI)
endif()

if(NOVA_ENABLE_OPENGL_RHI)
    target_compile_definitions(nova-renderer PUBLIC NOVA_OPENGL_RHI)
endif()

include(GNUInstallDirs)

##################################
# Add extra warnings if possible #
##################################
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options_if_supported(nova-renderer PUBLIC -Wall -Wextra -Wpedantic -Wconversion -Wdefaulted-function-deleted)
endif()

add_coverage(nova-renderer)

#########################
# Set libraries to link #
#########################
set(COMMON_LINK_LIBS
        ${CMAKE_DL_LIBS}
        fmt
        glm::glm
        glslang
        OGLCompiler
        OSDependent
        SPIRV
        minitrace
        miniz
        nlohmann::json
        vma::vma
        SPIRV-Tools
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-reflect
        spirv-cross-util)

if(NOVA_ENABLE_VULKAN_RHI)
    if(WIN32)
        set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} vulkan-1)
    else()
        set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} vulkan)
    endif()
endif()

if(NOVA_ENABLE_OPENGL_RHI)
    if(TARGET glfw)
        set(GLFW_TARGET_NAME glfw)
    elseif(TARGET glfw3)
        set(GLFW_TARGET_NAME glfw3)
    else()
        message(FATAL_ERROR "GLFW target name could not be found")
    endif()
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} ${GLFW_TARGET_NAME})
    target_include_directories(nova-renderer PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external/glfw/include)
endif()

if(NOVA_ENABLE_D3D12_RHI)
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} spirv-cross-hlsl)
endif()

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
    add_compile_options_if_supported(/Zc:twoPhase- /permissive /bigobj )

    set_target_properties(nova-renderer PROPERTIES PREFIX "")
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} d3d12 dxgi d3dcompiler spirv-cross-hlsl)
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
