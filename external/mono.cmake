if (WIN32)
	set(MONO_LIB_NAME libmono-static-sgen)
else() 
	set(MONO_LIB_NAME monosgen-2.0)
endif()
if (MONO_ROOT)
	find_library(
		MONO_LIB_LOCATION 
		NAMES ${MONO_LIB_NAME}
		PATHS "${MONO_ROOT}"
		PATH_SUFFIXES lib
		NO_DEFAULT_PATH 
		DOC "Location of mono library")
	find_path(
		MONO_INCLUDE_DIR 
		NAMES mono/jit/jit.h
		PATHS "${MONO_ROOT}"
		PATH_SUFFIXES include include/mono-2.0 mono-2.0
		NO_DEFAULT_PATH 
		DOC "Location of mono include files")
else()
	find_library(
		MONO_LIB_LOCATION 
		NAMES ${MONO_LIB_NAME}
		HINTS "C:/Program Files/Mono"
		PATHS ENV MONO_ROOT
		PATH_SUFFIXES lib
		DOC "Location of mono library")
	find_path(
		MONO_INCLUDE_DIR 
		NAMES mono/jit/jit.h
		HINTS "C:/Program Files/Mono"
		PATHS ENV MONO_ROOT
		PATH_SUFFIXES include include/mono-2.0 mono-2.0
		DOC "Location of mono include files")
endif()
if(MONO_LIB_LOCATION)
	message(STATUS "Mono library found: ${MONO_LIB_LOCATION}")
endif()
if(MONO_INCLUDE_DIR)
	message(STATUS "Mono include dir found: ${MONO_INCLUDE_DIR}")
endif()
if(NOT MONO_LIB_LOCATION)
	message(FATAL_ERROR "Mono library not found. Pass MONO_ROOT to root of mono install or set MONO_LIB_LOCATION to the path to libmono-static-sgen manually.")
endif()
if(NOT MONO_INCLUDE_DIR)
	message(FATAL_ERROR "Mono include files not found. Pass MONO_ROOT to root of mono install or set MONO_INCLUDE_DIR to the include directory with mono/jit/jit.h manually.")
endif()

add_library(mono::mono INTERFACE IMPORTED)
set_target_properties(mono::mono PROPERTIES
	INTERFACE_LINK_LIBRARIES "${MONO_LIB_LOCATION}"
	INTERFACE_INCLUDE_DIRECTORIES "${MONO_INCLUDE_DIR}"
	INTERFACE_COMPILE_DEFINITIONS "_REENTRANT"
)
