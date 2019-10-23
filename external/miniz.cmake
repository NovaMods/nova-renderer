# Miniz's cmake sucks, so doing it here
set(MINIZ_SOURCE ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz.c 
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_zip.c
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_tinfl.c
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_tdef.c)
add_library(miniz STATIC ${MINIZ_SOURCE})
include(GenerateExportHeader)
generate_export_header(miniz)
target_include_directories(miniz SYSTEM PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/miniz>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:miniz>)