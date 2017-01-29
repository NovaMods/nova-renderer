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
add_subdirectory("${3RD_PARTY_DIR}/glfw")
add_subdirectory("${3RD_PARTY_DIR}/json")
add_subdirectory("${3RD_PARTY_DIR}/glm")
add_subdirectory("${3RD_PARTY_DIR}/easyloggingpp")
add_subdirectory("${3RD_PARTY_DIR}/miniz")
add_subdirectory("${3RD_PARTY_DIR}/googletest")
add_subdirectory("${3RD_PARTY_DIR}/optional")

include_directories(SYSTEM
        "${3RD_PARTY_DIR}/glfw/include"
        "${3RD_PARTY_DIR}/easyloggingpp/src"
        "${3RD_PARTY_DIR}/glad/include"
        "${3RD_PARTY_DIR}/glm"
        "${3RD_PARTY_DIR}/miniz"
        "${3RD_PARTY_DIR}/json/src"
        "${3RD_PARTY_DIR}/googletest/googletest/include"
        "${3RD_PARTY_DIR}/optional"
        "${3RD_PARTY_DIR}/renderdocapi"
        )

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} glfw ${OPENGL_LIBRARIES})

# Setup the nova-core library.
set(NOVA_HEADERS

        geometry_cache/builders/gui_geometry_builder.h

        render/objects/shaders/uniform_buffer_definitions.h

        mc_interface/nova.h
        render/nova_renderer.h
        render/objects/textures/texture_manager.h
        utils/types.h

        render/objects/shaders/gl_shader_program.h
        render/objects/gl_uniform_buffer.h
        render/objects/gl_mesh.h
        render/objects/textures/texture2D.h

        render/windowing/glfw_gl_window.h
		
		input/InputHandler.h
		
        interfaces/iwindow.h

        mc_interface/mc_gui_objects.h
        mc_interface/mc_objects.h

        utils/utils.h
        data_loading/settings.h
        data_loading/loaders/loaders.h
        data_loading/loaders/shader_loading.h
        data_loading/loaders/loader_utils.h
        geometry_cache/mesh_store.h
        render/render_pass.h
        render/objects/shaders/shader_facade.h
        render/objects/render_object.h

        data_loading/physics/AABB.h
        data_loading/loaders/shader_source_structs.h
        geometry_cache/mesh_definition.h
        )

set(NOVA_SOURCE
        # Need to compile third party stuff, too...
        3rdparty/glad/src/glad.c
        3rdparty/miniz/miniz.c
        3rdparty/renderdocapi/RenderDocManager.cpp

        geometry_cache/builders/gui_geometry_builder.cpp

        render/nova_renderer.cpp
        mc_interface/nova_facade.cpp
        render/objects/textures/texture_manager.cpp
        render/uniform_buffer_store.cpp
        geometry_cache/builders/gui_geometry_builder.cpp

		input/InputHandler.cpp

		
        render/objects/shaders/gl_shader_program.cpp
        render/objects/gl_mesh.cpp
        render/objects/textures/texture2D.cpp

        render/windowing/glfw_gl_window.cpp

        utils/utils.cpp

        data_loading/settings.cpp
        data_loading/loaders/shader_loading.cpp
        data_loading/loaders/loader_utils.cpp

        render/objects/shaders/shaderpack.cpp
        geometry_cache/mesh_store.cpp
        data_loading/physics/AABB.cpp
        geometry_cache/mesh_definition.cpp
        render/objects/shaders/geometry_filter.cpp

        render/objects/shaders/uniform_buffers_definitions.cpp
        )

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_HEADERS} ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer-obj OBJECT ${NOVA_SOURCE} ${GLAD_SOURCE} ${MINIZ_SOURCE})
set_target_properties(nova-renderer-obj PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_compile_definitions(nova-renderer-obj PRIVATE DLL_EXPORT)

target_compile_definitions(nova-renderer-obj PUBLIC DLL_EXPORT)

add_library(nova-renderer SHARED $<TARGET_OBJECTS:nova-renderer-obj>)
target_compile_definitions(nova-renderer PRIVATE ELPP_THREAD_SAFE)
target_compile_definitions(nova-renderer PUBLIC ELPP_THREAD_SAFE)

# This is needed on MSVC otherwise the build files get spread out all over the place.
# The only *problem* is that we wont be able to create multi-config builds. Personally
# I dont see this as a problem currently. This will make the location more predictable
# cross platform as well. Though i think object files are still cached across build
# types so this should be fine.
get_target_property(OUTPUT_DIR nova-renderer LIBRARY_OUTPUT_DIRECTORY)
nova_set_all_target_outputs(nova-renderer ${OUTPUT_DIR})
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set_target_properties(nova-renderer PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()

#set_target_properties(nova-renderer-obj PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# Not sure if this is actually doing anything
if(GLFW_COMPILED)
        add_dependencies(nova-renderer glfw3)
endif()

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})

# Setup the nova-test executable
set(TEST_SOURCE_FILES
        3rdparty/glad/src/glad.c

        test/main.cpp

        test/model/loaders/shader_loading_test.cpp
        test/model/shaders/shader_facade_test.cpp
        test/model/geometry_cache/builders/gui_geometry_builder_tests.cpp
        test/model/texture_manager_test.cpp
        test/model/shaders/gl_shader_program_test.cpp
        test/model/geometry_cache/mesh_store_test.cpp
        test/test_utils.cpp
        test/test_utils.h)

source_group("test" FILES ${TEST_SOURCE_FILES})

add_executable(nova-test ${TEST_SOURCE_FILES} ${NOVA_SOURCE})
target_compile_definitions(nova-test PUBLIC STATIC_LINKAGE)
target_link_libraries(nova-test gtest ${COMMON_LINK_LIBS})
set_target_properties(nova-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Needed for a similar reason as the libary
if (MSVC)
    nova_set_all_target_outputs(nova-test "run")
endif()
