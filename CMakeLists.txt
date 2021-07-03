cmake_minimum_required(VERSION 3.1)
project(Cuttlefish)

# Build options
set(CUTTLEFISH_SHARED ON CACHE BOOL "Build Cuttlefish using shared libraries.")

# Options for disabling portions of the build.
set(CUTTLEFISH_BUILD_TESTS ON CACHE BOOL "Build unit tests.")
set(CUTTLEFISH_BUILD_DOCS ON CACHE BOOL "Build documentation.")
set(CUTTLEFISH_BUILD_TOOL ON CACHE BOOL "Build the tool.")
set(CUTTLEFISH_FORCE_INTERNAL_FREEIMAGE OFF CACHE BOOL "Force building FreeImage internally.")
set(CUTTLEFISH_BUILD_S3TC ON CACHE BOOL "Build support for S3TC texture compression.")
set(CUTTLEFISH_BUILD_ETC ON CACHE BOOL "Build support for ETC texture compression.")
set(CUTTLEFISH_BUILD_ASTC ON CACHE BOOL "Build support for ASTC texture compression.")
set(CUTTLEFISH_BUILD_PVRTC ON CACHE BOOL "Build support for PVRTC texture compression.")

# Misc options.
set(CUTTLEFISH_ISPC_PATH "" CACHE PATH "Path to the ISPC tool.")
set(CUTTLEFISH_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output CACHE PATH
	"Folder for placing the build output.")
set(CUTTLEFISH_EXPORTS_DIR ${CMAKE_BINARY_DIR}/cmake CACHE PATH
	"Folder for placing the cmake exports while building. Useful when embedding in other projects.")
set(CUTTLEFISH_ROOT_FOLDER Cuttlefish CACHE STRING
	"Root folder for the Cuttlefish projects. Usefull when embedding in other projects.")
set(CUTTLEFISH_INSTALL ON CACHE BOOL "Allow installation for Cuttlefish components.")
set(CUTTLEFISH_INSTALL_PVRTEXLIB ON CACHE BOOL
	"Include the PVRTexTool library with the installation.")
set(CUTTLEFISH_INSTALL_SET_RPATH ON CACHE BOOL "Set rpath for library and tool on installation.")

if (CUTTLEFISH_BUILD_TESTS)
	find_package(GTest QUIET)
	if (NOT GTEST_FOUND)
		message("GTest not installed. Skipping tests.")
	endif()
endif()

if (CUTTLEFISH_BUILD_DOCS)
	find_package(Doxygen QUIET)
	if (NOT DOXYGEN_FOUND)
		message("Doxygon not installed. Skipping documentation.")
	endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CUTTLEFISH_SHARED)
	set(CUTTLEFISH_LIB SHARED)
else()
	set(CUTTLEFISH_LIB STATIC)
endif()

if (CUTTLEFISH_OUTPUT_DIR)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CUTTLEFISH_OUTPUT_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CUTTLEFISH_OUTPUT_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CUTTLEFISH_OUTPUT_DIR})
endif()

set(CUTTLEFISH_MAJOR_VERSION 2)
set(CUTTLEFISH_MINOR_VERSION 5)
set(CUTTLEFISH_PATCH_VERSION 3)
set(CUTTLEFISH_VERSION ${CUTTLEFISH_MAJOR_VERSION}.${CUTTLEFISH_MINOR_VERSION}.${CUTTLEFISH_PATCH_VERSION})

set(CUTTLEFISH_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/config.cmake)
include(cmake/install.cmake)

add_subdirectory(lib)
if (CUTTLEFISH_BUILD_TOOL)
	add_subdirectory(tool)
endif()

# Documentation. (populated by built libraries)
add_subdirectory(doc)
