if(NOT CLANG_FORMAT_COMMAND)
    set(CLANG_FORMAT_COMMAND "clang-format")
endif()

find_program(CLANG_FORMAT_PROGRAM ${CLANG_FORMAT_COMMAND})
if(CLANG_FORMAT_PROGRAM)
    message(STATUS "Found ClangFormat at ${CLANG_FORMAT_PROGRAM}")

    function(clang_format TARGET SOURCES)
        list(APPEND SOURCE_LIST)
        add_custom_target(${TARGET}-clang-format VERBATIM)
        foreach(SOURCE_FILE IN LISTS ${SOURCES})
            list(APPEND SOURCE_LIST ${SOURCE_FILE})
            add_custom_command(
                    COMMAND ${CLANG_FORMAT_PROGRAM} -i -style=file ${SOURCE_FILE}
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                    COMMENT "Formatting ${SOURCE_FILE} with ${CLANG_FORMAT_PROGRAM}"
                    TARGET ${TARGET}-clang-format
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
        endforeach()

        add_dependencies(${TARGET} ${TARGET}-clang-format)
    endfunction()
else()
    function(clang_format TARGET SOURCES)
        message(WARNING "Tried to call clang-format for sources but it wasn't found")
    endfunction()
endif()