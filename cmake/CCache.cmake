if(NOT CCACHE_COMMAND)
    set(CCACHE_COMMAND "ccache")
endif()

find_program(CCACHE_PROGRAM ${CCACHE_COMMAND})
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    message(STATUS "Enabling CCache at ${CCACHE_PROGRAM}")
endif()
