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
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_util CONFIG REQUIRED)
find_package(SPIRV-Tools CONFIG REQUIRED)

include(minitrace)

#####################
# Setup google test #
#####################
if(NOVA_TEST)
	set(INSTALL_GTEST OFF)
	set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
	set(BUILD_GMOCK OFF CACHE BOOL "Build GMOCK shared library" FORCE)
	find_package(GTest MODULE REQUIRED)
endif()

