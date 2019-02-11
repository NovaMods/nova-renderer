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
add_custom_target(reformat VERBATIM)

file(GLOB_RECURSE CLANG_FORMAT_PATHS CONFIGURE_DEPENDS "*.clang-format")
foreach(PATH ${CLANG_FORMAT_PATHS})
    message(STATUS "Found .clang-format at ${PATH}")
endforeach()

function(format TARGET EXTRA_SOURCES)
    list(APPEND TOUCH_PATHS)

    get_property(TARGET_SOURCES TARGET ${TARGET} PROPERTY SOURCES)
    get_property(TARGET_DIR TARGET ${TARGET} PROPERTY SOURCE_DIR)
    list(APPEND FULL_SOURCES_LIST ${TARGET_SOURCES} ${EXTRA_SOURCES})
    list(REMOVE_DUPLICATES FULL_SOURCES_LIST)
    foreach(SOURCE_FILE ${FULL_SOURCES_LIST})
        set(FULL_SOURCE_PATH "${TARGET_DIR}/${SOURCE_FILE}")
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}" "${FULL_SOURCE_PATH}")

        set(FULL_TOUCH_PATH "${CMAKE_BINARY_DIR}/${REL_PATH}.format.touch")
        get_filename_component(TOUCH_DIR "${FULL_TOUCH_PATH}" DIRECTORY)
        list(APPEND TOUCH_PATHS "${FULL_TOUCH_PATH}")

        file(MAKE_DIRECTORY "${TOUCH_DIR}")
        add_custom_command(
            OUTPUT "${FULL_TOUCH_PATH}"
            COMMAND "${CLANG_FORMAT_PROGRAM}" -i -style=file "${SOURCE_FILE}"
            COMMAND "${CMAKE_COMMAND}" -E touch "${FULL_TOUCH_PATH}"
            DEPENDS "${FULL_SOURCE_PATH}" ${CLANG_FORMAT_PATHS}
            COMMENT "${SOURCE_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endforeach()

    add_custom_target(${TARGET}-format VERBATIM DEPENDS ${TOUCH_PATHS})
    add_custom_target(
        ${TARGET}-reformat VERBATIM 
        COMMAND "${CMAKE_COMMAND}" -E remove ${TOUCH_PATHS}
        COMMENT "Clearing format dependencies"
    )

    add_dependencies(format ${TARGET}-format)
    add_dependencies(reformat ${TARGET}-reformat)
endfunction()
