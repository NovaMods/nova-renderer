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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Wno-format-truncation")
endif()

set(BUILD_TESTING OFF CACHE BOOL "NO TESTING" FORCE)

# Setup 3rd party dependencies.
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)

######################################
# Add dependencies as subdirectories #
######################################

set(BUILD_DEMOS OFF CACHE BOOL "Disable demos" FORCE)
set(BUILD_ICD OFF CACHE BOOL "Disable ICD" FORCE)

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(${3RD_PARTY_DIR}/glm)
add_subdirectory(${3RD_PARTY_DIR}/profiler)
		
set(JSON_BuildTests OFF CACHE BOOL "JSON tests go away" FORCE)
add_subdirectory(${3RD_PARTY_DIR}/json)

set(FTL_BUILD_TESTS OFF CACHE BOOL "Disable FTL tests" FORCE)
set(FTL_BUILD_BENCHMARKS OFF CACHE BOOL "Disable FTL benchmarks" FORCE)
add_subdirectory(${3RD_PARTY_DIR}/FiberTaskingLib)

add_subdirectory(${3RD_PARTY_DIR}/miniz)

if(UNIX)
    find_package(Vulkan)

    add_subdirectory(${3RD_PARTY_DIR}/SPIRV-Headers)
    add_subdirectory(${3RD_PARTY_DIR}/SPIRV-Tools)
    add_subdirectory(${3RD_PARTY_DIR}/glslang)

    set(SHADERC_SKIP_TESTS TRUE CACHE BOOL "Shaderc tests go away" FORCE)
    add_subdirectory(${3RD_PARTY_DIR}/shaderc)
endif()

######################################
# Make dependencies' headers visible #
######################################

include_directories(SYSTEM
        $ENV{VULKAN_SDK}/include
        ${3RD_PARTY_DIR}/glm
        ${3RD_PARTY_DIR}/RenderDoc-Manager
        ${3RD_PARTY_DIR}/profiler/include
        ${3RD_PARTY_DIR}/json/single_include
        ${3RD_PARTY_DIR}/FiberTaskingLib/include
        ${3RD_PARTY_DIR}/cpptoml/include
        ${3RD_PARTY_DIR}/miniz
        )

if(UNIX)
    include_directories(SYSTEM
            ${3RD_PARTY_DIR}/VulkanMemoryAllocator/src
            ${3RD_PARTY_DIR}/shaderc/libshaderc/include
            )
endif()

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
        src/util/macros.hpp
        src/render_engine/command_buffer_base.hpp
        src/render_engine/command_buffer_base.cpp
        src/render_engine/window.hpp
        src/render_engine/resource_barrier.hpp
        src/loading/shaderpack/json_interop.cpp
        src/loading/shaderpack/json_interop.hpp
        src/loading/utils.hpp
        src/util/utils.hpp
        src/util/utils.cpp
		src/render_objects/render_object.cpp
		src/render_objects/render_object.hpp 
		src/loading/shaderpack/shaderpack_validator.cpp 
		src/loading/shaderpack/shaderpack_validator.hpp)

if(UNIX)
    set(SPIRV_CROSS_ROOT ${3RD_PARTY_DIR}/SPIRV-Cross)

    set(NOVA_SOURCE
            ${NOVA_SOURCE}
            src/render_engine/vulkan/vulkan_render_engine.hpp
            src/render_engine/vulkan/vulkan_render_engine.cpp
            src/render_engine/vulkan/vulkan_utils.hpp
            src/render_engine/vulkan/x11_window.cpp
            src/render_engine/vulkan/x11_window.hpp
            src/render_engine/vulkan/vulkan_resource_barrier_helpers.cpp
            src/render_engine/vulkan/vulkan_resource_barrier_helpers.hpp
            src/render_engine/vulkan/vulkan_opaque_types.hpp

            # SPIRV-Cross doesn't like being an in-source build, meaning I can't easily add it as a dependency to Nova.
            # The instructions are like "yeah just copy over whatever files you want cause that's totally the way modern
            # software is built -_-
            # So here we are,
            ${SPIRV_CROSS_ROOT}/spirv_cfg.cpp
            ${SPIRV_CROSS_ROOT}/spirv_cross_util.cpp
            ${SPIRV_CROSS_ROOT}/spirv_cross.cpp
            ${SPIRV_CROSS_ROOT}/spirv_glsl.cpp src/render_engine/vulkan/vulkan_type_converters.hpp)

elseif(WIN32)
    set(NOVA_SOURCE
            ${NOVA_SOURCE}
            src/render_engine/dx12/win32_window.hpp
            src/render_engine/dx12/win32_window.cpp
            src/render_engine/dx12/dx12_render_engine.cpp
            src/render_engine/dx12/dx12_render_engine.hpp
            src/render_engine/dx12/d3dx12.h
            src/render_engine/dx12/dx12_resource_barrier_helpers.cpp
            src/render_engine/dx12/dx12_resource_barrier_helpers.hpp
            src/render_engine/dx12/dx12_opaque_types.hpp)

else()
    message(FATAL_ERROR "Unsupported platform")

endif()

if (WIN32)
   set(NOVA_SOURCE ${NOVA_SOURCE} ${3RD_PARTY_DIR}/RenderDoc-Manager/RenderDocManager.cpp)
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer -fPIC")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer")
endif (UNIX)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -rdynamic")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -rdynamic")
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
target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)
#endif()

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} nova-profiler ftl miniz)

if (WIN32)
    set_target_properties(nova-renderer PROPERTIES PREFIX "")
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} d3d12 dxgi d3dcompiler)
    target_include_directories(nova-renderer PRIVATE
            "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.17134.0\\shared"
            "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.17134.0\\um")

else(WIN32)
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} ${Vulkan_LIBRARIES} shaderc stdc++fs X11)
endif (WIN32)

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})

set(NOVA_TEST ON)

if(NOVA_TEST)
    add_subdirectory(tests)
endif()

####################################
# Game Specific variants from here #
####################################

# Minecraft is the only variant for now so enable it by default for now
option(NOVA_MINECRAFT "Build minecraft library." ON)
if(NOVA_MINECRAFT)
    find_package(JNI REQUIRED)
    set(NOVA_MC_SOURCE
            mc_interface/jni/com_continuum_nova_system_NovaNative.h
            mc_interface/nova_mc_interface.cpp
            )
    add_library(nova-minecraft ${NOVA_MC_SOURCE})
    add_dependencies(nova-minecraft nova-renderer)
    target_include_directories(nova-minecraft PUBLIC ${JNI_INCLUDE_DIRS})
    target_link_libraries(nova-minecraft nova-render ${COMMON_LINK_LIBS} ${JNI_LIBRARIES})
endif()
