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

find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

include_target(spirv::headers "${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers")
include_target(vulkan::sdk "${VULKAN_INCLUDE}")

find_package(fmt CONFIG REQUIRED)
find_package(miniz CONFIG REQUIRED)

find_package(glslang CONFIG REQUIRED)

set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "Disable SPIRV-Tools tests" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "Enable error on warning SPIRV-Tools" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Tools)

set(ENABLE_EXPORTS ON CACHE BOOL "Enable linking SPIRV_Cross" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Cross)

target_includes_system(fmt::fmt)
target_includes_system(spirv-cross-core)
target_includes_system(spirv-cross-glsl)
target_includes_system(spirv-cross-hlsl)
target_includes_system(spirv-cross-reflect)
target_includes_system(spirv-cross-util)

#####################
# Setup google test #
#####################
if(NOVA_TEST)
	set(INSTALL_GTEST OFF)
	set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
	set(BUILD_GMOCK OFF CACHE BOOL "Build GMOCK shared library" FORCE)
	find_package(GTest MODULE REQUIRED)
	include(GoogleTest)
endif()

############################
# Manually built libraries #
############################
        
include(minitrace)

#####################################
# Hide unnecessary targets from all #
#####################################

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
