cmake_minimum_required(VERSION 3.8)
project(nova-renderer)

option(NOVA_TEST "Enable tests." OFF)

# Initialize cmake options
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

# Setup 3rd party dependencies.
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)

find_package(Vulkan)

if(NOVA_TEST)
    enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(BUILD_DEMOS OFF)
set(BUILD_ICD OFF)

set(SHADERC_SKIP_TESTS TRUE)

set(EASY_PROFILER_NO_GUI ON)
set(BUILD_SHARED_LIBS OFF)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_subdirectory(${3RD_PARTY_DIR}/glfw)
    add_subdirectory(${3RD_PARTY_DIR}/SPIRV-Headers)
    add_subdirectory(${3RD_PARTY_DIR}/SPIRV-Tools)
    add_subdirectory(${3RD_PARTY_DIR}/glslang)
    add_subdirectory(${3RD_PARTY_DIR}/shaderc)
endif()

add_subdirectory(${3RD_PARTY_DIR}/glm)
add_subdirectory(${3RD_PARTY_DIR}/miniz)
add_subdirectory(${3RD_PARTY_DIR}/profiler)
add_subdirectory(${3RD_PARTY_DIR}/json)

include_directories(SYSTEM
        $ENV{VULKAN_SDK}/include
        ${3RD_PARTY_DIR}/glfw/include
        ${3RD_PARTY_DIR}/easyloggingpp/src
        ${3RD_PARTY_DIR}/glad/include
        ${3RD_PARTY_DIR}/glm
        ${3RD_PARTY_DIR}/miniz
        ${3RD_PARTY_DIR}/RenderDoc-Manager
        ${3RD_PARTY_DIR}/VulkanMemoryAllocator/src
        ${3RD_PARTY_DIR}/shaderc/libshaderc/include
        ${3RD_PARTY_DIR}/profiler/include
        ${3RD_PARTY_DIR}/json/single_include
        )

set(NOVA_SOURCE
        # Third party stuff that's too lame to have it's own CMakeLists.txt
        ${3RD_PARTY_DIR}/miniz/miniz.c
        ${3RD_PARTY_DIR}/miniz/miniz_tdef.c
        ${3RD_PARTY_DIR}/miniz/miniz_tinfl.c
        ${3RD_PARTY_DIR}/miniz/miniz_zip.c

        src/platform.hpp
        src/render_engine/vulkan/timestamp_query_pool.hpp
        src/render_engine/vulkan/per_thread_data.hpp
        src/loading/zip_folder_accessor.hpp
        src/loading/regular_folder_accessor.hpp
        src/loading/folder_accessor.hpp
        src/render_engine/vulkan/vulkan_render_device.hpp
        src/nova_renderer.hpp
        src/nova_renderer.cpp
        src/loading/folder_accessor.cpp
        src/loading/regular_folder_accessor.cpp
        src/loading/zip_folder_accessor.cpp
        src/render_engine/vulkan/per_thread_data.cpp
        src/render_engine/vulkan/timestamp_query_pool.cpp
        src/render_engine/vulkan/vulkan_render_device.cpp
        src/render_engine/vulkan/glfw_window.cpp
        src/render_engine/vulkan/glfw_window.hpp
        src/util/utils.cpp
        src/util/utils.hpp

        src/util/smart_enum.hpp
        src/loading/shaderpack_loading.cpp
        src/loading/shaderpack_loading.hpp
        src/loading/loading_utils.cpp
        src/loading/loading_utils.hpp
        src/loading/shaderpack_data.hpp
        src/loading/shaderpack-data.cpp

        src/render_engine/dx12/win32_window.hpp
        src/render_engine/dx12/win32_window.cpp
        src/settings/settings.cpp
        src/settings/settings.hpp
        src/util/logger.cpp
        src/util/logger.hpp
        src/render_engine/render_engine.hpp
        src/util/macros.hpp src/render_engine/dx12/dx_12_render_engine.cpp src/render_engine/dx12/dx_12_render_engine.hpp src/render_engine/dx12/d3dx12.h)

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
    set(NOVA_SOURCE ${NOVA_SOURCE} ${3RD_PARTY_DIR}/RenderDoc-Manager/RenderDocManager.cpp)
    # set(CMAKE_CXX_FLAGS "-static-libstdc++ -static-libgcc ${CMAKE_CXX_FLAGS}")
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_STACKTRACE_ON_CRASH -g -fno-omit-frame-pointer -fPIC")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer")
endif (UNIX)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -rdynamic")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -rdynamic")
    message("Enabling symbol output")
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

set(NOVA_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer STATIC ${NOVA_SOURCE} ${SPIRV_CROSS_SOURCE})
target_include_directories(nova-renderer INTERFACE ${JNI_INCLUDE_DIRS})
set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_compile_definitions(nova-renderer PUBLIC ELPP_THREAD_SAFE)
#if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)
#endif()

target_compile_definitions(nova-renderer PRIVATE DLL_EXPORT ELPP_THREAD_SAFE)
target_compile_definitions(nova-renderer PUBLIC DLL_EXPORT ELPP_THREAD_SAFE)

if (WIN32)
    set_target_properties(nova-renderer PROPERTIES PREFIX "")
endif (WIN32)

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} glfw ${GLFW_LIBRARIES} ${Vulkan_LIBRARIES} shaderc stdc++fs nova-profiler)
target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})

if(NOVA_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    add_executable(nova-test tests/src/filesystem_test.cpp tests/src/end_to_end_runner.cpp)
    add_dependencies(nova-test nova-renderer)
    target_link_libraries(nova-test nova-renderer ${GTEST_BOTH_LIBRARIES} ${COMMON_LINK_LIBS})
    add_test(NovaTests nova-test)
    set(CMAKE_CXX_FLAGS "${NOVA_CXX_FLAGS}")
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