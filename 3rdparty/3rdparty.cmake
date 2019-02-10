#############################
# Overriding default values #
#############################

set(BUILD_DEMOS OFF CACHE BOOL "Disable demos" FORCE)
set(BUILD_ICD OFF CACHE BOOL "Disable ICD" FORCE)

set(BUILD_SHARED_LIBS OFF)

include(IncludeTarget)
include(TargetIncludesSystem)

#########################
# External dependencies #
#########################

find_package(Vulkan)
if (DEFINED ENV{VULKAN_SDK})
    message(STATUS "Vulkan environment variable: $ENV{VULKAN_SDK}")
    set(VULKAN_INCLUDE "$ENV{VULKAN_SDK}/include")
else()
    message(STATUS "Vulkan environment variable: undefined")
    set(VULKAN_INCLUDE "")
endif()

#########################
# Header only libraries #
#########################

include_target(glm::glm "${CMAKE_CURRENT_LIST_DIR}/glm")
set_target_properties(glm::glm PROPERTIES INTERFACE_COMPILE_DEFINITIONS "GLM_ENABLE_EXPERIMENTAL")

include_target(nlohmann::json "${CMAKE_CURRENT_LIST_DIR}/json/single_include")
include_target(spirv::headers "${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers")
include_target(vma::vma "${3RD_PARTY_DIR}/VulkanMemoryAllocator/src")
include_target(vulkan::sdk "${VULKAN_INCLUDE}")

#####################
# Setup google test #
#####################
set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF CACHE BOOL "Build GMOCK shared library" FORCE)
add_subdirectory(3rdparty/googletest)
include(GoogleTest)

#######################
# Submodule libraries #
#######################

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/glslang)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/profiler)

set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "Disable SPIRV-Tools tests" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "Enable error on warning SPIRV-Tools" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Tools)

set(ENABLE_EXPORTS ON CACHE BOOL "Enable linking SPIRV_Cross" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Cross)

target_includes_system(SPIRV)
target_includes_system(spirv-cross-core)
target_includes_system(spirv-cross-glsl)
target_includes_system(spirv-cross-hlsl)
target_includes_system(spirv-cross-reflect)
target_includes_system(spirv-cross-util)
target_includes_system(glslang)
target_includes_system(gtest)
target_includes_system(gtest_main)

############################
# Manually built libraries #
############################
        
include(miniz)
include(minitrace)

#####################################
# Hide unnecessary targets from all #
#####################################

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
