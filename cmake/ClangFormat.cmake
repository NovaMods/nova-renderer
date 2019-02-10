if(NOT CLANG_FORMAT_COMMAND)
    set(CLANG_FORMAT_COMMAND "clang-format-9" "clang-format-8" "clang-format-7" "clang-format-6.0" "clang-format")
endif()

find_program(CLANG_FORMAT_PROGRAM NAMES ${CLANG_FORMAT_COMMAND})
if(NOT CLANG_FORMAT_PROGRAM)
    message(STATUS "clang-format not found")
    function(format)
        # no-op
    endfunction()
    return()
endif()

message(STATUS "Found clang-format at ${CLANG_FORMAT_PROGRAM}")

function(format TARGET SOURCES)
    list(APPEND SOURCE_LIST)
    add_custom_target(${TARGET}-format VERBATIM)
    foreach(SOURCE_FILE IN LISTS ${SOURCES})
        list(APPEND SOURCE_LIST ${SOURCE_FILE})
        add_custom_command(
                COMMAND ${CLANG_FORMAT_PROGRAM} -i -style=file ${SOURCE_FILE}
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                COMMENT "Formatting ${SOURCE_FILE} with ${CLANG_FORMAT_PROGRAM}"
                TARGET ${TARGET}-format
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()

    add_dependencies(${TARGET} ${TARGET}-format)
endfunction()
