#############################
# Overriding default values #
#############################

set(BUILD_DEMOS OFF CACHE BOOL "Disable demos" FORCE)
set(BUILD_ICD OFF CACHE BOOL "Disable ICD" FORCE)

include(IncludeTarget)
include(TargetIncludesSystem)

#########################
# External dependencies #
#########################

find_package(Vulkan)
if (DEFINED ENV{VULKAN_SDK})
    message(STATUS "Vulkan environment variable: $ENV{VULKAN_SDK}")
    set(Vulkan_INCLUDE_DIR "$ENV{VULKAN_SDK}/include")
else()
    message(ERROR "Vulkan environment variable: undefined")
    set(Vulkan_INCLUDE_DIR "")
endif()

# Dependencies and specific options

# Compile things as shared libraries
set(BUILD_STATIC_LIBS OFF CACHE BOOL "Compile everything as a static lib" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Don't compile anything as a shared lib" FORCE)

set(ENABLE_EXPORTS ON CACHE BOOL "Enable linking SPIRV_Cross" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Don't build the CLI" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Tests are for library developers, not library consumers" FORCE)
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "Nova doesn't use MSL" FORCE)	# Need to remove this is we add a Metal RHI backend
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "Nova doesn't use C++ shaders, but one of its debugging tools might" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "No stop install bad" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Cross)

# Compile things as static libraries
set(BUILD_STATIC_LIBS ON CACHE BOOL "Compile everything as a static lib" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Don't compile anything as a shared lib" FORCE)

set(SPIRV-Headers_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers" CACHE FILEPATH "Use our submodule SPIRV-Headers" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "Disable SPIRV-Tools tests" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "Enable error on warning SPIRV-Tools" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SPIRV-Tools)

set(GLM_TEST_ENABLE_CXX_17 ON)
set(GLM_TEST_ENABLE OFF)
set(GLM_TEST_ENABLE_SIMD_AVX2 ON)	# TODO: determine minimum CPU for Nova and use the right instruction set
include_target(glm::glm "${CMAKE_CURRENT_LIST_DIR}/glm")

include_target(spirv::headers "${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers")
include_target(vma::vma "${3RD_PARTY_DIR}/VulkanMemoryAllocator/src")
include_target(vulkan::sdk "${VULKAN_INCLUDE}")

# GLFW
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/glfw)
        
include(minitrace)

set(BUILD_EXAMPLES OFF CACHE BOOL "Disable Miniz examples" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/miniz)

# Rex Core lib
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/rex_core)

# VMA
# Apparently they don't have an `include/` directory, you just include the src folder? Thanks AMD :|
include_directories(${CMAKE_CURRENT_LIST_DIR}/VulkanMemoryAllocator/src)

# DMA
# Something something check for Windows
include_directories(${CMAKE_CURRENT_LIST_DIR}/D3D12MemoryAllocator/src)

# DXC
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dxc-wrapper/extern/dxc)

#####################
# Test dependencies #
#####################
if(NOVA_TEST)
    set(BUILD_STATIC_LIBS OFF CACHE BOOL "Compile everything as a static lib" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Don't compile anything as a shared lib" FORCE)
	set(BUILD_GMOCK OFF)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/googletest)
endif()