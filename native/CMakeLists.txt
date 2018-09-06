cmake_minimum_required(VERSION 3.10)
project(nova-renderer)

option(NOVA_TEST "Enable tests." OFF)

include(ExternalProject)

# Initialize cmake options
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/build)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Wno-format-truncation")
endif()

# Setup 3rd party dependencies.
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)

set(BUILD_SHARED_LIBS OFF)

# Dependsncies that are common to all platforms
ExternalProject_Add(glm-proj
        PREFIX ${3RD_PARTY_DIR}/glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG cf6504b
        CMAKE_ARGS -DGLM_TEST_ENABLE=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

ExternalProject_Add(profiler-proj
        PREFIX ${3RD_PARTY_DIR}/profiler
        GIT_REPOSITORY https://github.com/NovaMods/profiler.git
        GIT_TAG 667d5de
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

ExternalProject_Add(json-proj
        PREFIX ${3RD_PARTY_DIR}/json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG eb30ff0
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DJSON_BuildTests=OFF)

# We use branch C++11
ExternalProject_Add(enkiTS-proj
        PREFIX ${3RD_PARTY_DIR}/enkiTS
        GIT_REPOSITORY https://github.com/NovaMods/enkiTS.git
        GIT_TAG 2225c13
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DENKITS_BUILD_C_INTERFACE=OFF -DENKITS_BUILD_EXAMPLES=OFF)

ExternalProject_Add(cpptoml-proj
        PREFIX ${3RD_PARTY_DIR}/cpptoml
        GIT_REPOSITORY https://github.com/skystrife/cpptoml.git
        GIT_TAG 13fb18e
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCPPTOML_BUILD_EXAMPLES=OFF)

# LINUX only
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    find_package(Vulkan)

    set(BUILD_DEMOS OFF)
    set(BUILD_ICD OFF)

    ExternalProject_Add(vma-proj
            PREFIX ${3RD_PARTY_DIR}/VulkanMemoryAllocator
            GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
            GIT_TAG c7d1b58
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

    ExternalProject_Add(SPIRV-Headers-proj
            PREFIX ${3RD_PARTY_DIR}/SPIRV-Headers
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
            GIT_TAG dcf23bd
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

    ExternalProject_Add(SPIRV-Tools-proj
            PREFIX ${3RD_PARTY_DIR}/SPIRV-Tools
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
            GIT_TAG 21bcb9d
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

    ExternalProject_Add(SPIRV-Cross-proj
            PREFIX ${3RD_PARTY_DIR}/SPIRV_Cross
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
            GIT_TAG 8c7282b
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

    ExternalProject_Add(glalsng-proj
            PREFIX ${3RD_PARTY_DIR}/glslang
            GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
            GIT_TAG 97068d8
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

    ExternalProject_Add(shaderc-proj
            PREFIX ${3RD_PARTY_DIR}/shaderc
            GIT_REPOSITORY https://github.com/google/shaderc.git
            GIT_TAG 30af9f9
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DSHADERC_SKIP_TESTS=ON)
endif()

# Windows only
if(WIN32)
    ExternalProject_Add(RenderDoc-API-proj
            PREFIX ${3RD_PARTY_DIR}/RenderDoc-Manager
            GIT_REPOSITORY https://github.com/NovaMods/RenderDoc-Manager.git
            GIT_TAG 1888256
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
endif()

include_directories(SYSTEM
        ${CMAKE_INSTALL_PREFIX}/include
        )

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
        src/util/utils.cpp
        src/util/utils.hpp

        src/util/smart_enum.hpp
        src/loading/shaderpack_loading.cpp
        src/loading/shaderpack_loading.hpp
        src/loading/loading_utils.cpp
        src/loading/loading_utils.hpp
        src/loading/shaderpack_data.hpp
        src/loading/shaderpack_data.cpp

        src/settings/settings.cpp
        src/settings/settings.hpp
        src/util/logger.cpp
        src/util/logger.hpp
        src/render_engine/render_engine.hpp
        src/util/macros.hpp
        src/render_engine/command_buffer_base.hpp
        src/render_engine/command_buffer_base.cpp
        src/render_engine/window.hpp
        src/render_engine/resource_barrier.hpp src/loading/utils.hpp)

if(UNIX)
    set(NOVA_SOURCE
            ${NOVA_SOURCE}
            src/render_engine/vulkan/vulkan_render_engine.hpp
            src/render_engine/vulkan/vulkan_render_engine.cpp
            src/render_engine/vulkan/vulkan_utils.hpp
            src/render_engine/vulkan/x11_window.cpp
            src/render_engine/vulkan/x11_window.hpp
            src/render_engine/vulkan/vulkan_command_buffer.hpp
            src/render_engine/vulkan/vulkan_command_buffer.cpp
            src/render_engine/vulkan/vulkan_resource_barrier_helpers.cpp
            src/render_engine/vulkan/vulkan_resource_barrier_helpers.hpp
            src/render_engine/vulkan/vulkan_opaque_types.hpp)
elseif(WIN32)
    set(NOVA_SOURCE
            ${NOVA_SOURCE}
            src/render_engine/dx12/win32_window.hpp
            src/render_engine/dx12/win32_window.cpp
            src/render_engine/dx12/dx12_render_engine.cpp
            src/render_engine/dx12/dx12_render_engine.hpp
            src/render_engine/dx12/d3dx12.h
            src/render_engine/dx12/dx12_command_buffer.hpp
            src/render_engine/dx12/dx12_resource_barrier_helpers.cpp
            src/render_engine/dx12/dx12_resource_barrier_helpers.hpp
            src/render_engine/dx12/dx12_command_buffer.cpp
            src/render_engine/dx12/dx12_opaque_types.hpp)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
# SPIRV-Cross doesn't like being an in-source build, meaning I can't easily add it as a dependency to Nova. The
# instructions are like "yeah just copy over whatever files you want cause that's totally the way modern software is
# built -_-
# So here we are,
set(SPIRV_CROSS_ROOT ${3RD_PARTY_DIR}/SPIRV-Cross)
set(SPIRV_CROSS_SOURCE
        ${SPIRV_CROSS_ROOT}/spirv_cfg.cpp
        ${SPIRV_CROSS_ROOT}/spirv_cross_util.cpp
        ${SPIRV_CROSS_ROOT}/spirv_cross.cpp
        ${SPIRV_CROSS_ROOT}/spirv_glsl.cpp
        )

set(SPIRV_CROSS_HEADERS
        ${SPIRV_CROSS_ROOT}/spirv_cross.hpp
        )

if (WIN32)
   # set(NOVA_SOURCE ${NOVA_SOURCE} ${3RD_PARTY_DIR}/RenderDoc-Manager/RenderDocManager.cpp)
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

add_library(nova-renderer STATIC ${NOVA_SOURCE}) # ${SPIRV_CROSS_SOURCE})
target_include_directories(nova-renderer INTERFACE ${JNI_INCLUDE_DIRS})
set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_compile_definitions(nova-renderer PUBLIC ELPP_THREAD_SAFE)
#if(CMAKE_BUILD_TYPE MATCHES "Debug")
target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)
#endif()

add_dependencies(nova-renderer
        glm-proj profiler-proj json-proj enkiTS-proj cpptoml-proj)

target_compile_definitions(nova-renderer PRIVATE DLL_EXPORT ELPP_THREAD_SAFE)
target_compile_definitions(nova-renderer PUBLIC DLL_EXPORT ELPP_THREAD_SAFE)

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} nova-profiler enkiTS cpptoml)

if (WIN32)
    set_target_properties(nova-renderer PROPERTIES PREFIX "")
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} d3d12 dxgi d3dcompiler)
    target_include_directories(nova-renderer PRIVATE
            "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.17134.0\\shared"
            "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.17134.0\\um")

    add_dependencies(nova-renderer
            RenderDoc-API-proj)

else(WIN32)
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} glfw ${GLFW_LIBRARIES} ${Vulkan_LIBRARIES} shaderc stdc++fs)

    add_dependencies(nova-renderer
            vma-proj SPIRV-Headers-proj SPIRV-Tools-proj SPIRV-Cross-proj glslang-proj shaderc-proj)
endif (WIN32)

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})

set(NOVA_TEST ON)

if(NOVA_TEST)
    add_subdirectory(tests)

    enable_testing()
    add_test(NovaTests nova-test)
endif()

####################################
# Game Specific variants from here #
####################################

# Minecraft is the only variant for now so enable it by default for now
option(NOVA_MINECRAFT "Build minecraft library." ON)
if(NOVA_MINECRAFT)
    set(JAVA_INCLUDE_PATH /usr/include/openjdk-8/)
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