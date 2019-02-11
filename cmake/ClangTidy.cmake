if(NOT CLANG_TIDY_COMMAND)
    set(CLANG_TIDY_COMMAND
        "clang-tidy-9"
        "clang-tidy-8"
        "clang-tidy-7"
        "clang-tidy-6.0"
        "clang-tidy"
        CACHE STRING "Possible names of clang-tidy")
endif()

if(NOT CLANG_APPLY_REPLACEMENTS_COMMAND)
    set(CLANG_APPLY_REPLACEMENTS_COMMAND 
        "clang-apply-replacements-9"
        "clang-apply-replacements-8"
        "clang-apply-replacements-7"
        "clang-apply-replacements-6.0"
        "clang-apply-replacements"
        CACHE STRING "Possible names of clang-apply-replacements")
endif()

find_program(CLANG_TIDY_PROGRAM NAMES ${CLANG_TIDY_COMMAND})
if(NOT CLANG_TIDY_PROGRAM)
    message(STATUS "clang-tidy not found")
    # function(lint)
    #     # no-op
    # endfunction()
    return()
endif()

message(STATUS "Found clang-tidy at ${CLANG_TIDY_PROGRAM}")
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

find_package(Python3)

if(NOT Python3_FOUND)
    return()
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/lint.bash.in" "${CMAKE_CURRENT_BINARY_DIR}/tools/unexec/lint.bash" @ONLY NEWLINE_STYLE LF)
file(COPY "${CMAKE_CURRENT_BINARY_DIR}/tools/unexec/lint.bash"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/tools"
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
     GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/tools/unexec")

message(STATUS "Linting enabled through ./tools/lint.bash")

add_custom_target(lint "${CMAKE_CURRENT_BINARY_DIR}/tools/lint.bash" "--sort" "file" USES_TERMINAL)
add_custom_target(lint-by-file "${CMAKE_CURRENT_BINARY_DIR}/tools/lint.bash" "--sort" "file" USES_TERMINAL)
add_custom_target(lint-by-diagnostic "${CMAKE_CURRENT_BINARY_DIR}/tools/lint.bash" "--sort" "diagnostic"  USES_TERMINAL)

find_program(CLANG_APPLY_REPLACEMENTS_PROGRAM NAMES ${CLANG_APPLY_REPLACEMENTS_COMMAND})
if(CLANG_APPLY_REPLACEMENTS_PROGRAM AND CLANG_FORMAT_PROGRAM)
    message(STATUS "Found clang-apply-replacements at ${CLANG_APPLY_REPLACEMENTS_PROGRAM}")
    add_custom_target(
        lint-fix 
        COMMAND "${CMAKE_CURRENT_BINARY_DIR}/tools/lint.bash" "--sort" "file" "--fix" "--clang-apply-replacements" "${CLANG_APPLY_REPLACEMENTS_PROGRAM}" 
        COMMAND "${CMAKE_COMMAND}" "--build" "." "--target" "format"
        USES_TERMINAL
    )
else()
    message(STATUS "clang-apply-replacements not found")
endif()
# function(lint TARGET)
#     add_custom_target(${TARGET}-lint VERBATIM)
    
#     list(APPEND SOURCE_LIST)

#     get_property(TARGET_SOURCES TARGET ${TARGET} PROPERTY SOURCES)
#     foreach(SOURCE_FILE ${TARGET_SOURCES})
#     	get_filename_component(EXTANT ${SOURCE_FILE} EXT)

#     	if(EXTANT MATCHES [[.*\.[cC].*]])
#         	list(APPEND SOURCE_LIST ${SOURCE_FILE})
#         endif()
#     endforeach()

#     set_property(GLOBAL PROPERTY CLANG_TIDY_FILES ${CLANG_TIDY_FILES})
# endfunction()

# function(enable_linting RUN_CLANG_TIDY)
# 	get_property(CLANG_TIDY_FILES GLOBAL PROPERTY CLANG_TIDY_FILES)

# 	add_custom_target(lint 
# 		COMMAND ${run_clang_tidy} )
# endfunction()
