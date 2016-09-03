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

# Dependency management. Download GLM, GLFW, and ZipLib
# Gonna just do ZipLib first, cause we already have a solution for the others

# Add ZipLib
#ExternalProject_Add(ziplib
#        GIT_REPOSITORY https://bitbucket.org/hsmith/ziplib/src/5b9fda317e6ba60d5be87c7511fcd35b2addefcd?at=master
#        INSTALL_DIR ${3RD_PARTY_DIR}/ziplib
#        SOURCE_DIR ${3RD_PARTY_DIR}/ziplib#
#
#        )

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(ENV{GLM_ROOT_DIR} "${3RD_PARTY_DIR}/glm")
        set(ENV{GLFW3_ROOT} "${3RD_PARTY_DIR}/glfw")
endif()

# First look for a pre compiled version if it doesnt find anything
# Try to compile it.
set(GLFW_COMPILED FALSE)
# this will still find the include directory for us even if we need to compile
find_package(GLFW3)
if(NOT GLFW3_FOUND OR NOT GLFW3_LIBRARY)
        set(GLFW_DIR "${3RD_PARTY_DIR}/glfw-src")
        set(GLFW3_INCLUDE_DIR "${3RD_PARTY_DIR}/glfw/include")
        if(EXISTS "${GLFW_DIR}" AND EXISTS "${GLFW_DIR}/src")
                set(GLFW_COMPILED TRUE)

                include(ExternalProject)
                ExternalProject_add(glfw3 DOWNLOAD_COMMAND ""
                        PREFIX ${BINARY_DIR}
                        INSTALL_DIR "${3RD_PARTY_DIR}/glfw"
                        CMAKE_ARGS CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DGLFW_BUILD_EXAMPLES:BOOL=OFF -DGLFW_BUILD_TESTS:BOOL=OFF -DGLFW_BUILD_DOCS:BOOL=OFF -DGLFW_INSTALL:BOOL=ON
                        SOURCE_DIR ${GLFW_DIR}
                )
                if(MSVC)
                    set(GLFW3_LIBRARY "${3RD_PARTY_DIR}/glfw/lib/glfw3.lib")
                else()
                    set(GLFW3_LIBRARY "${3RD_PARTY_DIR}/glfw/lib/libglfw3.a")
                endif()
                # This is a hack to make sure that the compilation order
                # is correct. The ExternalProject needs to be compiled before
                # nova-renderer so that the header files will be installed into
                # the correct location.
                add_library(GLFW-fake OBJECT "")
                add_dependencies(GLFW-fake glfw3)
        endif()
endif()


find_package(GLM REQUIRED)

# Not sure if this is actually needed or not..,
# I know some extension loaders and windowing libraries do
# but I'm not certain since i've never used GLFW or glad. Its really
# only here just in case it actually is needed. Seems to build fine on
# linux without though.
find_package(OpenGL REQUIRED)

set(GLAD_SOURCE ${3RD_PARTY_DIR}/glad/src/glad.c)


set(MINIZ_SOURCE "${3RD_PARTY_DIR}/miniz/src/miniz.c"
                 "${3RD_PARTY_DIR}/miniz/src/miniz_zip.c"
                 "${3RD_PARTY_DIR}/miniz/src/miniz_tinfl.c"
                 "${3RD_PARTY_DIR}/miniz/src/miniz_tdef.c")
include_directories(SYSTEM
        ${GLM_INCLUDE_DIRS}
        ${GLFW3_INCLUDE_DIR}
        "${3RD_PARTY_DIR}/easyloggingpp/include"
        "${3RD_PARTY_DIR}/glad/include"
        "${3RD_PARTY_DIR}/miniz/src"
        "${3RD_PARTY_DIR}/json/src")

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} ${GLFW3_LIBRARY} ${OPENGL_LIBRARIES} pthread)

include_directories(${CMAKE_CURRENT_LIST_DIR})

# Setup the nova-core library.
set(NOVA_SOURCE

        core/gui/gui_renderer.cpp

        core/nova_renderer.cpp
        core/nova_facade.cpp
        core/texture_manager.cpp
        core/uniform_buffer_store.cpp
        core/gui/gui_renderer.cpp

        gl/models/gl_shader_program.cpp
        gl/models/gl_uniform_buffer.cpp
        gl/models/gl_vertex_buffer.cpp
        gl/models/texture2D.cpp

        gl/windowing/glfw_gl_window.cpp

        interfaces/ivertex_buffer.cpp

        io/key_forwarder.cpp

        utils/utils.cpp

        data_sources/shaderpack.cpp
        config/config.cpp
        3rdparty/stb_image/stb_image.h)

# The following lists of files will only be contained in
# the source groups and will not be sent to the compiler directly.
set(NOVA_NO_COMPILE
        core/renderer/batch_builder.cpp
        core/renderer/model_renderer.cpp
        )

set(NOVA_HEADERS

        core/gui/gui_renderer.h

        core/renderer/batch_builder.h
        core/renderer/model_renderer.h

        core/shaders/uniform_buffer_definitions.h

        core/nova.h
        core/nova_renderer.h
        core/texture_manager.h
        core/types.h

        gl/models/gl_shader_program.h
        gl/models/gl_uniform_buffer.h
        gl/models/gl_vertex_buffer.h
        gl/models/texture2D.h

        gl/windowing/glfw_gl_window.h

        interfaces/igraphics_context.h
        interfaces/ivertex_buffer.h
        interfaces/iwindow.h

        io/key_forwarder.h

        mc/mc_gui_objects.h
        mc/mc_objects.h

        utils/utils.h
        data_sources/shaderpack.h
        config/config.h
        )

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_HEADERS} ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer-obj OBJECT ${NOVA_SOURCE} ${GLAD_SOURCE} ${MINIZ_SOURCE})
set_target_properties(nova-renderer-obj PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_compile_definitions(nova-renderer-obj PRIVATE DLL_EXPORT)

target_compile_definitions(nova-renderer-obj PUBLIC DLL_EXPORT)

if (GLFW_COMPILED)
    add_dependencies(nova-renderer-obj GLFW-fake)
endif()

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
        test/sanity.cpp
        test/shader_test.cpp
        test/test_utils.cpp
        test/config.cpp
        )

set(TEST_HEADERS
        test/sanity.h
        test/shader_test.h
        test/test_utils.h
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
