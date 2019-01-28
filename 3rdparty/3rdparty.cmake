######################################
# Add dependencies as subdirectories #
######################################

set(BUILD_DEMOS OFF CACHE BOOL "Disable demos" FORCE)
set(BUILD_ICD OFF CACHE BOOL "Disable ICD" FORCE)

set(BUILD_SHARED_LIBS OFF)

add_library(glm::glm INTERFACE IMPORTED)
set_target_properties(glm::glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/glm"
    INTERFACE_COMPILE_FEATURES cxx_std_17
    INTERFACE_COMPILE_DEFINITIONS "GLM_ENABLE_EXPERIMENTAL"
)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/profiler)
        
add_library(nlohmann::json INTERFACE IMPORTED)
set_target_properties(nlohmann::json PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/json/single_include"
    INTERFACE_COMPILE_FEATURES cxx_std_17
)

# Miniz's cmake sucks, so doing it here
set(MINIZ_SOURCE ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz.c 
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_zip.c
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_tinfl.c
                 ${CMAKE_CURRENT_LIST_DIR}/miniz/miniz_tdef.c)
add_library(miniz ${MINIZ_SOURCE})
target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_LIST_DIR}/miniz)

set(SPIRV_SKIP_TESTS ON CACHE BOOL "Disable SPIRV-Tools tests" FORCE)
add_library(spirv::headers INTERFACE IMPORTED)
set_target_properties(spirv::headers PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers"
    INTERFACE_COMPILE_FEATURES cxx_std_17
)
set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers)
set(SPIRV_WERROR OFF CACHE BOOL "Enable error on warning SPIRV-Tools" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Tools)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/glslang)

set(ENABLE_EXPORTS ON CACHE BOOL "Enable linking SPIRV_Cross" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Cross)

find_package(Vulkan)

set_property(TARGET glslang PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET glslang-default-resource-limits PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET OGLCompiler PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET OSDependent PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET SPIRV PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET SPVRemapper PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET HLSL PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET glslangValidator PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-remap PROPERTY EXCLUDE_FROM_ALL True)

set_property(TARGET spirv-tools-build-version PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-debuginfo PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-header-DebugInfo PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-spv-amd-gcn-shader PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-spv-amd-shader-ballot PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-spv-amd-shader-explicit-vertex-parameter PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-spv-amd-shader-trinary-minmax PROPERTY EXCLUDE_FROM_ALL True)

set_property(TARGET spirv-tools-cpp-example PROPERTY EXCLUDE_FROM_ALL True)

set_property(TARGET spirv-as PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cfg PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-dis PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-link PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-opt PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-reduce PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-stats PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-val PROPERTY EXCLUDE_FROM_ALL True)

set_property(TARGET SPIRV-Tools PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET SPIRV-Tools-link PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET SPIRV-Tools-opt PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET SPIRV-Tools-reduce PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET SPIRV-Tools-shared PROPERTY EXCLUDE_FROM_ALL True)

set_property(TARGET spirv-tools-vimsyntax PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-pkg-config PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-tools-shared-pkg-config PROPERTY EXCLUDE_FROM_ALL True)

set_property(TARGET spirv-cross PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cross-core PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cross-cpp PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cross-glsl PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cross-hlsl PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cross-msl PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cross-reflect PROPERTY EXCLUDE_FROM_ALL True)
set_property(TARGET spirv-cross-util PROPERTY EXCLUDE_FROM_ALL True)