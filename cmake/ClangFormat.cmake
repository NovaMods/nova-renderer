if(NOT CLANG_FORMAT_COMMAND)
    set(CLANG_FORMAT_COMMAND "clang-format-9" "clang-format-8" "clang-format-7" "clang-format")
endif()

find_program(CLANG_FORMAT_PROGRAM NAMES ${CLANG_FORMAT_COMMAND})

execute_process(COMMAND "${CLANG_FORMAT_PROGRAM}" "--version"
                OUTPUT_VARIABLE CLANG_TIDY_VERSION_FULL_STRING)
string(REGEX MATCH [=[[0-9]\.[0-9].[0-9]]=] VERSION "${CLANG_TIDY_VERSION_FULL_STRING}")
if(VERSION VERSION_LESS "7.0.0")
    message(STATUS "clang-format version ${VERSION} is not greater or equal to 7.0.0")
    set(CLANG_FORMAT_TERMINATE On)
endif()
if(NOT CLANG_FORMAT_PROGRAM)
    message(STATUS "clang-format not found")
    set(CLANG_FORMAT_TERMINATE On)
endif()
if(CLANG_FORMAT_TERMINATE)
    message(STATUS "Disabling formatting.")
    function(format)
        # no-op
    endfunction()
    function(nova_format)
        # no-op
    endfunction()
    return()
endif()

message(STATUS "Found clang-format at ${CLANG_FORMAT_PROGRAM}")

if(NOT TARGET format)
    add_custom_target(format VERBATIM)
endif()
if(NOT TARGET reformat)
    add_custom_target(reformat VERBATIM)
endif()

file(GLOB_RECURSE CLANG_FORMAT_PATHS CONFIGURE_DEPENDS "*.clang-format")
foreach(PATH ${CLANG_FORMAT_PATHS})
    message(STATUS "Found .clang-format at ${PATH}")
endforeach()

function(format TARGET)
    list(APPEND TOUCH_PATHS)

    get_property(TARGET_SOURCES TARGET ${TARGET} PROPERTY SOURCES)
    get_property(TARGET_DIR TARGET ${TARGET} PROPERTY SOURCE_DIR)
    list(APPEND FULL_SOURCES_LIST ${TARGET_SOURCES} ${ARGV1})
    list(REMOVE_DUPLICATES FULL_SOURCES_LIST)
    foreach(SOURCE_FILE ${FULL_SOURCES_LIST})
        if(SOURCE_FILE MATCHES [=[^/.*$]=])
            set(FULL_SOURCE_PATH "${SOURCE_FILE}")
        else()
            set(FULL_SOURCE_PATH "${TARGET_DIR}/${SOURCE_FILE}")
        endif()
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

function(nova_format)
    if (NOT NOVA_IN_SUBMODULE)
        format(${ARGV})
    endif()
endfunction()
