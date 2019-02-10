add_library(glsl-standalone STATIC ${CMAKE_CURRENT_LIST_DIR}/glslang/StandAlone/ResourceLimits.cpp)
target_include_directories(glsl-standalone PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glslang)
