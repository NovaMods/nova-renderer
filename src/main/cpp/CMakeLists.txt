cmake_minimum_required(VERSION 3.4)
project(renderer)

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
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug) # This seems to be needed to actually change

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Setup 3rd party dependencies.
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)

find_package(OpenGL)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# need to compile GLFW and JSON
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/glfw")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/json")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/glm")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/easyloggingpp")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/miniz")

include_directories(SYSTEM
        "${3RD_PARTY_DIR}/glfw/include"
        "${3RD_PARTY_DIR}/easyloggingpp/src"
        "${3RD_PARTY_DIR}/glad/include"
        "${3RD_PARTY_DIR}/glm"
        "${3RD_PARTY_DIR}/miniz"
        "${3RD_PARTY_DIR}/json/src")

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} glfw ${OPENGL_LIBRARIES})

include_directories(${CMAKE_CURRENT_LIST_DIR})

# Setup the nova-core library.
set(NOVA_SOURCE

        view/gui/gui_renderer.cpp

        nova_renderer.cpp
        mc_interface/nova_facade.cpp
        model/texture_manager.cpp
        model/uniform_buffer_store.cpp
        view/gui/gui_renderer.cpp

        model/shaders/gl_shader_program.cpp
        model/gl/gl_uniform_buffer.cpp
        model/gl/gl_vertex_buffer.cpp
        model/texture2D.cpp

        view/windowing/glfw_gl_window.cpp

        interfaces/ivertex_buffer.cpp

        utils/utils.cpp

        model/settings.cpp
        model/loaders/shader_loading.cpp
        model/loaders/loaders.h
        model/loaders/shader_loading.h
        model/data_model.cpp
        model/data_model.h
        model/loaders/loader_utils.cpp
        model/loaders/loader_utils.h

        # Need to compile third party stuff, too...
        3rdparty/glad/src/glad.c
        3rdparty/miniz/miniz.c
        view/renderer.cpp
        view/renderer.h
        adapter/adapter_facade.cpp
        adapter/adapter_facade.h
        adapter/render_command/render_command.cpp
        adapter/render_command/render_command.h
        model/render_object.h
        model/shaders/shader_facade.cpp
        model/shaders/shader_facade.h
        )


set(NOVA_HEADERS

        view/gui/gui_renderer.h

        core/renderer/batch_builder.h
        core/renderer/model_renderer.h

        model/shaders/uniform_buffer_definitions.h

        mc_interface/nova.h
        nova_renderer.h
        model/texture_manager.h
        view/types.h

        model/shaders/gl_shader_program.h
        model/gl/gl_uniform_buffer.h
        model/gl/gl_vertex_buffer.h
        model/texture2D.h

        view/windowing/glfw_gl_window.h

        interfaces/igraphics_context.h
        interfaces/ivertex_buffer.h
        interfaces/iwindow.h

        mc_interface/mc_gui_objects.h
        mc_interface/mc_objects.h

        utils/utils.h
        model/settings.h
        )

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_HEADERS} ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer-obj OBJECT ${NOVA_SOURCE} ${GLAD_SOURCE} ${MINIZ_SOURCE})
set_target_properties(nova-renderer-obj PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_compile_definitions(nova-renderer-obj PRIVATE DLL_EXPORT)

target_compile_definitions(nova-renderer-obj PUBLIC DLL_EXPORT)

add_library(nova-renderer SHARED $<TARGET_OBJECTS:nova-renderer-obj>)
add_library(nova-renderer-static STATIC $<TARGET_OBJECTS:nova-renderer-obj>)

# This is needed on MSVC otherwise the build files get spread out all over the place.
# The only *problem* is that we wont be able to create multi-config builds. Personally
# I dont see this as a problem currently. This will make the location more predictable
# cross platform as well. Though i think object files are still cached across build
# types so this should be fine.
get_target_property(OUTPUT_DIR nova-renderer LIBRARY_OUTPUT_DIRECTORY)
nova_set_all_target_outputs(nova-renderer ${OUTPUT_DIR})
nova_set_all_target_outputs(nova-renderer-static ${OUTPUT_DIR})
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set_target_properties(nova-renderer PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()

#set_target_properties(nova-renderer-obj PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# Not sure if this is actually doing anything
if(GLFW_COMPILED)
        add_dependencies(nova-renderer glfw3)
endif()

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})
target_link_libraries(nova-renderer-static ${COMMON_LINK_LIBS})

# Setup the nova-test executable
set(TEST_SOURCE_FILES
        test/main.cpp
        )

set(TEST_HEADERS
        )

source_group("test" FILES ${TEST_SOURCE_FILES} ${TEST_HEADERS})

add_executable(nova-test ${TEST_SOURCE_FILES})
add_dependencies(nova-test nova-renderer-static)
target_compile_definitions(nova-test PUBLIC STATIC_LINKAGE)
target_link_libraries(nova-test nova-renderer-static ${COMMON_LINK_LIBS})
set_target_properties(nova-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")

# Needed for a similar reason as the libary
if (MSVC)
    nova_set_all_target_outputs(nova-test "run")
endif()
