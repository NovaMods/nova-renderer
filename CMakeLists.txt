cmake_minimum_required(VERSION 3.10)
project(nova-renderer)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    message(STATUS "Enabling CCache at ${CCACHE_PROGRAM}")
endif()

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(NOVA_IN_SUBMODULE False)
else()
    set(NOVA_IN_SUBMODULE True)
endif()

option(NOVA_TEST "Enable tests." OFF)
option(ENABLE_EXPERIMENTAL "Enable experimental features, may be in code as well as in the CMake files" OFF)
if(ENABLE_EXPERIMENTAL)
    set(CMAKE_LINK_WHAT_YOU_USE TRUE) # Warn about unsued linked libraries
endif()

include(ExternalProject)

# Initialize cmake options
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT ${NOVA_IN_SUBMODULE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/build)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

set(BUILD_TESTING OFF CACHE BOOL "NO TESTING" FORCE)

# Setup 3rd party dependencies.
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)

include(${3RD_PARTY_DIR}/3rdparty.cmake)

######################################
# Make dependencies' headers visible #
######################################

include(cmake/RemovePermissive.cmake)

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

set(SPIRV_CROSS_ROOT ${3RD_PARTY_DIR}/SPIRV-Cross)

set(NOVA_SOURCE
        ${3RD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.cpp

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

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer -fPIC")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer")
endif (UNIX)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -rdynamic")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -rdynamic")
    message("Enabling symbol output")
endif()

if(NOT WIN32)
    add_compile_options("-fdiagnostics-color=always")
endif()

set(NOVA_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer STATIC ${NOVA_SOURCE})
target_include_directories(nova-renderer INTERFACE ${JNI_INCLUDE_DIRS})
set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_compile_definitions(nova-renderer PUBLIC NOMINMAX)

target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)

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
include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)
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

if(WIN32)
    find_package(WindowsSDK)
    get_windowssdk_include_dirs(${WINDOWSSDK_LATEST_DIR} WinSDK_DIRS)

    # MSVC 15.9 broke the Windows SDK by implementing two phase lookup. "/Zc:twoPhase-" disables it.

    check_cxx_compiler_flag(/Zc:twoPhase- TWO_PHASE_LOOKUP)
    if (TWO_PHASE_LOOKUP)
        add_compile_options(/Zc:twoPhase-)
    endif()

    CHECK_CXX_COMPILER_FLAG(/permissive PERMISSIVE)
    if (PERMISSIVE)
        add_compile_options(/permissive)
    endif()

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
    check_cxx_compiler_flag(-Wno-unknown-pragmas Wno_unknown_pragmas)
    if (Wno_unknown_pragmas)
        add_compile_options(-Wno-unknown-pragmas)
    endif()

    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} stdc++fs X11)
endif()

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})

remove_permissive(nova-renderer)

set(NOVA_TEST ON)

if(NOVA_TEST)
    add_subdirectory(tests)
endif()
