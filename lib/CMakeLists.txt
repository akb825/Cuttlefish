find_package(Threads)

set(FREEIMAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage)
set(NVTT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nvidia-texture-tools)
set(ETC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/etc2comp)
set(ASTC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/astc-encoder)
set(defines)
set(privateDefines _USE_MATH_DEFINES)

if (NOT CUTTLEFISH_FORCE_INTERNAL_FREEIMAGE)
	find_package(FreeImage QUIET)
endif()
if (NOT FreeImage_FOUND)
	message("FreeImage not found")
	if (NOT EXISTS ${FREEIMAGE_DIR})
		message(FATAL_ERROR
			"FreeImage not found. Run 'git submodule init' and "
			"'git submodule update' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/FreeImageSources.cmake)
	list(APPEND privateDefines DISABLE_PERF_MEASUREMENT=1)
endif()

if (CUTTLEFISH_BUILD_S3TC)
	if (NOT EXISTS ${NVTT_DIR})
		message(FATAL_ERROR
			"nvidia-texture-tools not found. Run 'git submodule init' and "
			"'git submodule update' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/NvttSources.cmake)
	list(APPEND defines CUTTLEFISH_HAS_S3TC=1)
endif()

if (CUTTLEFISH_BUILD_ETC)
	if (NOT EXISTS ${ETC_DIR})
		message(FATAL_ERROR
			"etc2comp not found. Run 'git submodule init' and "
			"'git submodule update' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/Etc2CompSources.cmake)
	list(APPEND defines CUTTLEFISH_HAS_ETC=1)
endif()

if (CUTTLEFISH_BUILD_ASTC)
	if (NOT EXISTS ${ASTC_DIR})
		message(FATAL_ERROR
			"astc-encoder not found. Run 'git submodule init' and "
			"'git submodule update' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/AstcSources.cmake)
	list(APPEND defines CUTTLEFISH_HAS_ASTC=1)
endif()

if (CUTTLEFISH_BUILD_PVRTC)
	find_package(PVRTexLib QUIET)
	if (PVRTEXLIB_FOUND)
		list(APPEND defines CUTTLEFISH_HAS_PVRTC=1)
	else()
		message("PVRTexLib not found")
		set(PVRTEXLIB_INCLUDE_DIRS)
		set(PVRTEXLIB_LIBRARIES)
	endif()
endif()

file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)
set(externalSources ${freeImageSources} ${ALL_NVTT_SOURCES} ${etcSources} ${astcSources})

# Don't care about warnings for external files.
if (MSVC)
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
else()
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS "-w -Wno-c++11-narrowing")
endif()

add_library(cuttlefish_lib ${CUTTLEFISH_LIB} ${sources} ${externalSources})
set_target_properties(cuttlefish_lib PROPERTIES OUTPUT_NAME cuttlefish)
target_include_directories(cuttlefish_lib
	PRIVATE ${FreeImage_INCLUDE_DIRS}
			${NVTT_INCLUDE_DIRS}
			${ETC_INCLUDE_DIRS}
			${ASTC_INCLUDE_DIRS}
			${PVRTEXLIB_INCLUDE_DIRS}
			glm
			src)
target_link_libraries(cuttlefish_lib
	PRIVATE ${FreeImage_LIBRARIES} ${PVRTEXLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(cuttlefish_lib PUBLIC ${defines} PRIVATE ${privateDefines})

if (CUTTLEFISH_BUILD_PVRTC AND PVRTEXLIB_FOUND)
	if (WIN32)
		get_filename_component(pvrTexLibDir ${PVRTEXLIB_LIB} DIRECTORY)
		set(pvrTexLibDll ${pvrTexLibDir}/PVRTexLib.dll)
		add_custom_command(TARGET cuttlefish_lib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
			${pvrTexLibDll} $<TARGET_FILE_DIR:cuttlefish_lib>)
	elseif (APPLE)
		add_custom_command(TARGET cuttlefish_lib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
			${PVRTEXLIB_LIB} $<TARGET_FILE_DIR:cuttlefish_lib>)
		if (CUTTLEFISH_SHARED)
			cfs_fixup_mac_dep(cuttlefish_lib "@executable_path/../lib/libPVRTexLib.dylib"
				"@rpath/libPVRTexLib.dylib")
		endif()
	endif()
endif()

cfs_set_folder(cuttlefish_lib libs)
cfs_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/cuttlefish
	FILES ${sources})
cfs_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} FILES ${externalSources})

cfs_install_library(TARGET cuttlefish_lib)
set(CUTTLEFISH_DOC_PROJECTS ${CUTTLEFISH_DOC_PROJECTS} lib PARENT_SCOPE)

if (CUTTLEFISH_INSTALL AND CUTTLEFISH_INSTALL_PVRTEXLIB AND CUTTLEFISH_BUILD_PVRTC AND
	PVRTEXLIB_FOUND)
	if (WIN32)
		install(FILES ${pvrTexLibDll} DESTINATION bin)
	else()
		install(FILES ${PVRTEXLIB_LIB} DESTINATION lib)
	endif()
endif()

add_subdirectory(test)
