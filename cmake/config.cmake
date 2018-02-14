# Copyright 2017 Aaron Barany
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

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (MSVC)
	add_compile_options(/W3 /WX /wd4200 /MP)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
else()
	add_compile_options(-Wall -Werror -Wconversion -Wno-sign-conversion -fno-strict-aliasing)
	# Behavior for VISIBILITY_PRESET variables are inconsistent between CMake versions.
	if (CUTTLEFISH_SHARED)
		add_compile_options(-fvisibility=hidden -fvisibility-inlines-hidden)
	endif()
endif()

enable_testing()

if (CUTTLEFISH_INSTALL_SET_RPATH)
	if (APPLE)
		set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../lib")
		set(MACOSX_RPATH ON)
	else()
		set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")
	endif()
endif()

function(cfs_set_folder target folderName)
	if (CUTTLEFISH_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${CUTTLEFISH_ROOT_FOLDER}/${folderName})
	elseif (NOT CUTTLEFISH_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${folderName})
	else()
		set_property(TARGET ${target} PROPERTY FOLDER ${CUTTLEFISH_ROOT_FOLDER})
	endif()
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

function (cfs_fixup_mac_dep target old new)
	if (NOT APPLE)
		return()
	endif()

	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change
		"${old}" "${new}" -add_rpath "$<TARGET_FILE_DIR:${target}>" "$<TARGET_FILE:${target}>")
endfunction()
