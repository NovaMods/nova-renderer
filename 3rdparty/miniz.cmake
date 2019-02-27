# Miniz's cmake sucks, so doing it here
set(MINIZ_SOURCE ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz.c 
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_zip.c
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_tinfl.c
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_tdef.c)
add_library(miniz ${MINIZ_SOURCE})
target_include_directories(miniz SYSTEM PUBLIC ${CMAKE_CURRENT_LIST_DIR}/miniz)
