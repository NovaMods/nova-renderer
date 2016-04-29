cmake_minimum_required(VERSION 3.3)
project(renderer)

add_subdirectory(
        "C:/MinGW/glfw" "${CMAKE_CURRENT_BINARY_DIR}/glfw"
)

include_directories(
        "C:/win-builds/include"
        "C:/win-builds/lib/gcc/mingw32/4.8.1/include/c++"
        "C:/Program Files/Java/jdk1.8.0_25/include/win32"
        "C:/Program Files/Java/jdk1.8.0_25/include"
        "C:/VulkanSDK/1.0.5.0/Include"
)

link_directories(
        "C:/win-builds/lib"
)

set(SOURCE_FILES
        src/main.cpp
        src/gl/glad.c
        src/interfaces/iwindow.cpp src/interfaces/iwindow.h
        src/core/nova_renderer.cpp src/core/nova_renderer.h
        src/interfaces/igraphics_context.cpp src/interfaces/igraphics_context.h
        src/interfaces/ivertex_buffer.cpp src/interfaces/ivertex_buffer.h
        src/interfaces/itexture.cpp src/interfaces/itexture.h
        src/gl/windowing/glfw_gl_window.cpp src/gl/windowing/glfw_gl_window.h
        src/gl/core/opengl_wrapper.cpp src/gl/core/opengl_wrapper.h
        src/io/key_forwarder.cpp src/io/key_forwarder.h
        src/core/nova_facade.cpp
        src/nova_renderer.def
        )

add_library(nova-renderer SHARED ${SOURCE_FILES})

# configure the project
target_include_directories(nova-renderer SYSTEM PRIVATE)
target_link_libraries(nova-renderer pthread glfw ${GLFW_LIBRARIES})

set_property(TARGET nova-renderer PROPERTY CXX_STANDARD 14)
set_property(TARGET nova-renderer PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
