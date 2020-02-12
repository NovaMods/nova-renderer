# minitrace has no cmake :(
    set(MINITRACE_SOURCE ${CMAKE_CURRENT_LIST_DIR}/minitrace/minitrace.c
    ${CMAKE_CURRENT_LIST_DIR}/minitrace/minitrace.h)
add_library(minitrace STATIC ${MINITRACE_SOURCE})
target_include_directories(minitrace SYSTEM PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/minitrace>
        $<INSTALL_INTERFACE:minitrace>)
target_compile_definitions(minitrace PUBLIC MTR_ENABLED)
