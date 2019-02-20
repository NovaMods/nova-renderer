# minitrace has no cmake :(
set(MINITRACE_SOURCE ${CMAKE_CURRENT_LIST_DIR}/minitrace/minitrace.c
                     ${CMAKE_CURRENT_LIST_DIR}/minitrace/minitrace.h)
add_library(minitrace ${MINITRACE_SOURCE})
target_include_directories(minitrace SYSTEM PUBLIC ${CMAKE_CURRENT_LIST_DIR}/minitrace)
