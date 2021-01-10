include(FetchContent)
include(IncludeTarget)
include(TargetIncludesSystem)

#############################
# Overriding default values #
#############################

set(BUILD_DEMOS OFF CACHE BOOL "Disable demos" FORCE)
set(BUILD_ICD OFF CACHE BOOL "Disable ICD" FORCE)

#########################
# External dependencies #
#########################

# VCPKG dependencies

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

find_package(Vulkan)
if (DEFINED ENV{VULKAN_SDK})
    message(STATUS "Vulkan environment variable: $ENV{VULKAN_SDK}")
    set(Vulkan_INCLUDE_DIR "$ENV{VULKAN_SDK}/include")
else()
    message(STATUS "Vulkan environment variable: undefined")
    set(Vulkan_INCLUDE_DIR "")
endif()

# Submodule dependencies

# Compile things as static libraries
set(BUILD_STATIC_LIBS ON CACHE BOOL "Compile everything as a static lib" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Don't compile anything as a shared lib" FORCE)

set(GLM_TEST_ENABLE_CXX_17 ON)
set(GLM_TEST_ENABLE OFF)
set(GLM_TEST_ENABLE_SIMD_AVX2 ON)	# TODO: determine minimum CPU for Nova and use the right instruction set
include_target(glm::glm "${CMAKE_CURRENT_LIST_DIR}/glm")

include_target(vma::vma "${3RD_PARTY_DIR}/VulkanMemoryAllocator/src")
include_target(vulkan::sdk "${VULKAN_INCLUDE}")

# Tracy
include_directories("${CMAKE_CURRENT_LIST_DIR}/tracy")

# VMA
# Apparently they don't have an `include/` directory, you just include the src folder? Thanks AMD :|
include_directories(${CMAKE_CURRENT_LIST_DIR}/VulkanMemoryAllocator/src)

# Rando dependencies that no one likes
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        master 
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
