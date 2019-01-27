cmake_minimum_required(VERSION 3.10)
project(nova-renderer)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    message(STATUS "Enabling CCache at ${CCACHE_PROGRAM}")
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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -rdynamic -fdiagnostic-color=always")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -rdynamic -fdiagnostic-color=always")
    message("Enabling symbol output")
endif()

set(NOVA_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer STATIC ${NOVA_SOURCE})
target_include_directories(nova-renderer INTERFACE ${JNI_INCLUDE_DIRS})
set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

#if(CMAKE_BUILD_TYPE MATCHES "Debug")
target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS NOMINMAX)
#endif()

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
	                 glslang
	                 SPIRV
	                 ${SPIRV_SHARED_LIBRARIES}
	                 ${Vulkan_LIBRARIES}
	                 ${GLFW_LIBRARY_NAME})
    set_target_properties(nova-renderer PROPERTIES PREFIX "")
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} d3d12 dxgi d3dcompiler)
    target_include_directories(nova-renderer PRIVATE
            "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.17134.0\\shared"
            "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.17134.0\\um")
    target_compile_definitions(nova-renderer PUBLIC _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING NOMINMAX)

    # Visual Studio is stupid. Is it really that hard to use filesystem directories to organize code like literally
    # everyone else?
    foreach(source IN LISTS NOVA_SOURCE)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
	
else(WIN32)
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} stdc++fs X11)
endif (WIN32)

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})

set(NOVA_TEST ON)

if(NOVA_TEST)
    add_subdirectory(tests)
endif()
