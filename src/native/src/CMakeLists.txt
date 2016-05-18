cmake_minimum_required(VERSION 3.3)
project(renderer)

add_subdirectory(
        "C:/MinGW/glfw" "${CMAKE_CURRENT_BINARY_DIR}/glfw"
)

include_directories(
        "C:/win-builds/lib/gcc/mingw32/4.8.1/include/c++"
        "C:/win-builds/include"
)

link_directories(
        "C:/win-builds/lib"
)

set(SOURCE_FILES
        gl/glad.c
        interfaces/iwindow.cpp interfaces/iwindow.h
        core/nova_renderer.cpp core/nova_renderer.h
        interfaces/igraphics_context.cpp interfaces/igraphics_context.h
        interfaces/ivertex_buffer.h
        interfaces/itexture.h
        gl/windowing/glfw_gl_window.cpp gl/windowing/glfw_gl_window.h
        io/key_forwarder.cpp io/key_forwarder.h
        core/nova_facade.cpp
        core/texture_manager.cpp core/texture_manager.h
        gl/objects/texture2D.cpp gl/objects/texture2D.h
        )

add_library(nova-renderer SHARED ${SOURCE_FILES})

# configure the project
target_include_directories(nova-renderer SYSTEM PRIVATE)
target_link_libraries(nova-renderer glfw ${GLFW_LIBRARIES})

set_property(TARGET nova-renderer PROPERTY CXX_STANDARD 14)
set_property(TARGET nova-renderer PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
