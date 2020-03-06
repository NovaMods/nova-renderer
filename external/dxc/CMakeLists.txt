cmake_minimum_required(VERSION 3.8)
project(DXCmake)

file(GLOB_RECURSE SOURCES "lib/*.h")

add_library(dxc-wrap STATIC ${SOURCES} "dummy.cc")

target_include_directories(dxc-wrap PUBLIC include/)

if (MSVC)
    target_link_libraries(dxc-wrap
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/dxcompiler.lib"
    )

    # copy the dll to the runtime output folder post-build
    set(DXC_COPY_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/bin/dxcompiler.dll" "${CMAKE_CURRENT_SOURCE_DIR}/bin/dxil.dll")

    # foreach(file_i ${DXC_COPY_DLLS})
    #     add_custom_command(
    #         TARGET dxc-wrap POST_BUILD
    #         COMMAND ${CMAKE_COMMAND} -E copy_if_different "${file_i}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    #     )
    # endforeach()

else()
    target_link_libraries(dxc-wrap
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/lib_linux/libdxcompiler.so.3.7"
    )
endif()
