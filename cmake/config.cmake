# Copyright 2017-2022 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(GNUInstallDirs)

# Code should compile with C++11, but set to 14 for dependencies. Compiling on older targets will
# fall back to the the latest version.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (APPLE AND CMAKE_OSX_ARCHITECTURES)
	list(LENGTH CMAKE_OSX_ARCHITECTURES architectureCount)
	if (architectureCount EQUAL 1)
		set(CUTTLEFISH_ARCH ${CMAKE_OSX_ARCHITECTURES})
	else()
		set(CUTTLEFISH_ARCH multi)
	endif()
else()
	set(CUTTLEFISH_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

if (CUTTLEFISH_ARCH MATCHES "^x86" OR CUTTLEFISH_ARCH STREQUAL "amd64" OR
		CUTTLEFISH_ARCH STREQUAL "AMD64" OR CUTTLEFISH_ARCH STREQUAL "i386" OR
		CUTTLEFISH_ARCH STREQUAL "i686")
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CUTTLEFISH_ARCH x86_64)
	else()
		set(CUTTLEFISH_ARCH x86)
	endif()
elseif (CUTTLEFISH_ARCH MATCHES "^arm" OR CUTTLEFISH_ARCH STREQUAL "aarch64")
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CUTTLEFISH_ARCH arm64)
	else()
		set(CUTTLEFISH_ARCH arm)
	endif()
endif()

if (MSVC)
	add_compile_options(/W3 /WX /wd4200 /MP)
	if (CUTTLEFISH_ARCH STREQUAL "x86")
		add_compile_options(/arch:SSE2)
	endif()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
else()
	add_compile_options(-Wall -Werror -Wconversion -Wno-sign-conversion -fno-strict-aliasing)
	if (CUTTLEFISH_ARCH STREQUAL "x86")
		add_compile_options(-msse2)
	endif()
	if (CMAKE_C_COMPILER_ID MATCHES "GNU")
		add_compile_options(-Wno-comment)
		if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0)
			add_compile_options(-faligned-new)
		endif()
	elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" AND
			CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0)
		add_compile_options(-faligned-new)
	endif()
	# Behavior for VISIBILITY_PRESET variables are inconsistent between CMake versions.
	if (CUTTLEFISH_SHARED)
		add_compile_options(-fvisibility=hidden)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
	endif()
endif()

enable_testing()

if (CUTTLEFISH_INSTALL AND CUTTLEFISH_INSTALL_SET_RPATH)
	if (APPLE)
		set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../${CMAKE_INSTALL_LIBDIR}")
	else()
		set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
	endif()
endif()

function(cfs_set_folder target)
	set_property(TARGET ${target} PROPERTY FOLDER ${CUTTLEFISH_ROOT_FOLDER})
endfunction()

function(cfs_setup_filters)
	set(options)
	set(oneValueArgs SRC_DIR INCLUDE_DIR)
	set(multiValueArgs FILES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	foreach (fileName ${ARGS_FILES})
		# Get the directory name. Make sure there's a trailing /.
		get_filename_component(directoryName ${fileName} DIRECTORY)
		set(directoryName ${directoryName}/)

		set(filterName)
		string(REGEX MATCH ${ARGS_SRC_DIR}/.* matchSrc ${directoryName})
		string(REGEX MATCH ${ARGS_INCLUDE_DIR}/.* matchInclude ${directoryName})

		if (matchSrc)
			string(REPLACE ${ARGS_SRC_DIR}/ "" filterName ${directoryName})
			set(filterName src/${filterName})
		elseif (matchInclude)
			string(REPLACE ${ARGS_INCLUDE_DIR}/ "" filterName ${directoryName})
			set(filterName include/${filterName})
		endif()

		if (filterName)
			string(REPLACE "/" "\\" filterName ${filterName})
			source_group(${filterName} FILES ${fileName})
		endif()
	endforeach()
endfunction()
