# Helper file for searching for vulkan-1.dll for cross compiling
if(NOT Vulkan_LIBRARY)
	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/vulkan-dll-location.txt)
		file(READ ${CMAKE_CURRENT_LIST_DIR}/vulkan-dll-location.txt Vulkan_LIBRARY)
	else()
		# Comment this line and uncomment the line under it, replace <your/path> with a path to vulkan-1.dll (you can get it from the windows installer)
		message(FATAL_ERROR "Vulkan_LIBRARY not set! Edit toolchains/vulkan_library.cmake to contain the correct path to vulkan-1.dll!")
		# set(Vulkan_LIBRARY <your/path>) # Uncomment this line
	endif()
endif()
message("Vulkan_LIBRARY is ${Vulkan_LIBRARY}")
