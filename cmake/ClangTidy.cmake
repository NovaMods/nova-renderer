if(NOT CLANG_TIDY_COMMAND)
    set(CLANG_TIDY_COMMAND "clang-tidy-9" "clang-tidy-8" "clang-tidy-7" "clang-tidy-6.0" "clang-tidy" CACHE STRING "Possible names of clang-tidy")
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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/lint.bash.in" "${CMAKE_CURRENT_BINARY_DIR}/tools/unexec/lint.bash" @ONLY NEWLINE_STYLE LF)
file(COPY "${CMAKE_CURRENT_BINARY_DIR}/tools/unexec/lint.bash"
	 DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/tools"
	 FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
	 GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/tools/unexec")

message(STATUS "Linting enabled through ./tools/lint.bash")

add_custom_target(lint "${CMAKE_CURRENT_BINARY_DIR}/tools/lint.bash" USES_TERMINAL)

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
