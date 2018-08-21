cmake_minimum_required(VERSION 3.8)
project(nova-renderer)

# Functions
function(nova_set_all_target_outputs target output_dir)
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${output_dir})
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${output_dir})
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${output_dir})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${output_dir})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${output_dir})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${output_dir})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_dir})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${output_dir})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${output_dir})
endfunction()

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

find_package(JNI REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(BUILD_DEMOS OFF)
set(BUILD_ICD OFF)

set(SHADERC_SKIP_TESTS TRUE)

set(BUILD_SHARED_LIBS TRUE)

set(EASY_PROFILER_NO_GUI ON)
set(BUILD_SHARED_LIBS OFF)

# need to compile GLFW and JSON
add_subdirectory(${3RD_PARTY_DIR}/glfw)
add_subdirectory(${3RD_PARTY_DIR}/json)
add_subdirectory(${3RD_PARTY_DIR}/glm)
add_subdirectory(${3RD_PARTY_DIR}/easyloggingpp)
add_subdirectory(${3RD_PARTY_DIR}/miniz)
add_subdirectory(${3RD_PARTY_DIR}/SPIRV-Headers)
add_subdirectory(${3RD_PARTY_DIR}/SPIRV-Tools)
add_subdirectory(${3RD_PARTY_DIR}/glslang)
add_subdirectory(${3RD_PARTY_DIR}/shaderc)
add_subdirectory(${3RD_PARTY_DIR}/profiler)

include_directories(SYSTEM
        $ENV{VULKAN_SDK}/include
        ${JNI_INCLUDE_DIRS}
        ${3RD_PARTY_DIR}/glfw/include
        ${3RD_PARTY_DIR}/easyloggingpp/src
        ${3RD_PARTY_DIR}/glad/include
        ${3RD_PARTY_DIR}/glm
        ${3RD_PARTY_DIR}/miniz
        ${3RD_PARTY_DIR}/json/include
        ${3RD_PARTY_DIR}/googletest/googletest/include
        ${3RD_PARTY_DIR}/Optional
        ${3RD_PARTY_DIR}/RenderDoc-Manager
        ${3RD_PARTY_DIR}/VulkanMemoryAllocator/src
        ${3RD_PARTY_DIR}/shaderc/libshaderc/include
        ${3RD_PARTY_DIR}/profiler/include
        )

# Setup the nova-core library.
set(NOVA_HEADERS
        src/mc_interface/jni/com_continuum_nova_system_NovaNative.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_GeometryType.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_key_char_event.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_key_press_event.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_mc_atlas_texture.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_mc_gui_buffer.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_mc_settings.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_mc_texture_atlas_location.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_mouse_button_event.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_mouse_position_event.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_mouse_scroll_event.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_NativeBoolean.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_NovaVertexFormat.h
        src/mc_interface/jni/com_continuum_nova_system_NovaNative_window_size.h

        src/platform.hpp
        src/vulkan/timestamp_query_pool.hpp
        src/vulkan/per_thread_data.hpp
        src/fs_access/zip_folder_accessor.hpp
        src/fs_access/regular_folder_accessor.hpp
        src/fs_access/folder_accessor.hpp
        src/vulkan/vulkan_render_device.hpp
        src/nova_renderer.hpp
        )

set(NOVA_SOURCE
        # Third party stuff that's too lame to have it's own CMakeLists.txt
        ${3RD_PARTY_DIR}/miniz/miniz.c
        ${3RD_PARTY_DIR}/miniz/miniz_tdef.c
        ${3RD_PARTY_DIR}/miniz/miniz_tinfl.c
        ${3RD_PARTY_DIR}/miniz/miniz_zip.c
        ${3RD_PARTY_DIR}/easyloggingpp/src/easylogging++.cc

        src/nova_renderer.cpp
        src/fs_access/folder_accessor.cpp
        src/fs_access/regular_folder_accessor.cpp
        src/fs_access/zip_folder_accessor.cpp
        src/vulkan/per_thread_data.cpp
        src/vulkan/timestamp_query_pool.cpp
        src/vulkan/glfw_window.cpp src/vulkan/glfw_window.hpp src/util/util.cpp src/util/util.hpp src/input/input_handler.cpp src/input/input_handler.hpp)


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
    set(CMAKE_CXX_FLAGS "-static-libstdc++ -static-libgcc ${CMAKE_CXX_FLAGS}")
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

if(MSVC)
    message(FATAL_ERROR "MSVC is not supported, use MinGW with gcc")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer SHARED ${NOVA_SOURCE} ${SPIRV_CROSS_SOURCE})
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

# This is needed on MSVC otherwise the build files get spread out all over the place.
# The only *problem* is that we wont be able to create multi-config builds. Personally
# I dont see this as a problem currently. This will make the location more predictablelajsdhfnvfsadk;jh
# cross platform as well. Though i think object files are still cached across build
# types so this should be fine.
get_target_property(OUTPUT_DIR nova-renderer LIBRARY_OUTPUT_DIRECTORY)
nova_set_all_target_outputs(nova-renderer ${OUTPUT_DIR})
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set_target_properties(nova-renderer PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} glfw ${GLFW_LIBRARIES} ${Vulkan_LIBRARIES} shaderc stdc++fs nova-profiler ${JNI_LIBRARIES})

add_executable(nova-test ${NOVA_SOURCE} ${SPIRV_CROSS_SOURCE})
target_link_libraries(nova-test ${COMMON_LINK_LIBS})
target_compile_definitions(nova-test PUBLIC STATIC_LINKAGE)
set_target_properties(nova-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})