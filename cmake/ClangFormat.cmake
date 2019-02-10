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
add_custom_target(format VERBATIM)

function(format TARGET)
    list(APPEND TOUCH_PATHS)

    get_property(TARGET_SOURCES TARGET ${TARGET} PROPERTY SOURCES)
    get_property(TARGET_DIR TARGET ${TARGET} PROPERTY SOURCE_DIR)
    foreach(SOURCE_FILE ${TARGET_SOURCES})
        set(FULL_SOURCE_PATH "${TARGET_DIR}/${SOURCE_FILE}")
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}" "${FULL_SOURCE_PATH}")

        set(FULL_TOUCH_PATH "${CMAKE_BINARY_DIR}/${REL_PATH}.touch")
        get_filename_component(TOUCH_DIR "${FULL_TOUCH_PATH}" DIRECTORY)
        list(APPEND TOUCH_PATHS "${FULL_TOUCH_PATH}")

        file(MAKE_DIRECTORY "${TOUCH_DIR}")
        message(STATUS "Formatting to ${FULL_TOUCH_PATH}")
        add_custom_command(
                OUTPUT "${FULL_TOUCH_PATH}"
                COMMAND "${CLANG_FORMAT_PROGRAM}" -i -style=file "${SOURCE_FILE}"
                COMMAND "${CMAKE_COMMAND}" -E touch "${FULL_TOUCH_PATH}"
                DEPENDS "${FULL_SOURCE_PATH}"
                COMMENT "${SOURCE_FILE}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endforeach()
    message(STATUS ${TOUCH_PATHS})

    add_custom_target(${TARGET}-format VERBATIM DEPENDS ${TOUCH_PATHS})

    add_dependencies(format ${TARGET}-format)
endfunction()
