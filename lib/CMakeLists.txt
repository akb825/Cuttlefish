find_package(Threads)

set(FREEIMAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage)
set(SQUISH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libsquish)
set(COMPRESSONATOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/compressonator)
set(BC7ENC_RDO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bc7enc_rdo)
set(ISPC_TEXCOMP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ISPCTextureCompressor)
set(ETC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/etc2comp)
set(ASTC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/astc-encoder)
set(ispcSources)
set(defines)
set(privateDefines)
set(externalSources)
set(extraLibraries)
set(extraIncludeDirs)

if (CUTTLEFISH_ISPC_PATH)
	set(ISPC ${CUTTLEFISH_ISPC_PATH} CACHE FILEPATH "Path to the ISPC compiler." FORCE)
	mark_as_advanced(ISPC)
else()
	if (WIN32)
		file(GLOB ispcInstallDir "C:/Program Files/ISPC/ispc-*-windows")
		set(ispcVersion)
		foreach (ispcDir ${ispcInstallDir})
			string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" curVersion ${ispcDir})
			if (curVersion VERSION_GREATER ispcVersion)
				set(ispcInstallPath ${ispcDir}/bin)
				set(ispcVersion ${curVersion})
			endif()
		endforeach()
	endif()
	find_program(ISPC ispc PATHS ${ispcInstallPath})
endif()

function(get_ispc_arch outArch outTargets outTargetNames arch singleTarget)
	# Supports both CUTTLEFISH_ARCH and CMAKE_OSX_ARCHITECTURES values.
	if (arch STREQUAL "x86" OR arch STREQUAL "i366")
		set(${outArch} x86 PARENT_SCOPE)
		if (singleTarget)
			set(${outTargets} sse4-i32x4 PARENT_SCOPE)
			set(${outTargetNames} sse4 PARENT_SCOPE)
		else()
			set(${outTargets} sse2-i32x4 sse4-i32x4 avx2-i32x4 PARENT_SCOPE)
			set(${outTargetNames} sse2 sse4 avx2 PARENT_SCOPE)
		endif()
	elseif (arch STREQUAL "x86_64")
		set(${outArch} x86-64 PARENT_SCOPE)
		if (singleTarget)
			set(${outTargets} sse4-i32x4 PARENT_SCOPE)
			set(${outTargetNames} sse4 PARENT_SCOPE)
		else()
			set(${outTargets} sse2-i32x4 sse4-i32x4 avx2-i32x4 PARENT_SCOPE)
			set(${outTargetNames} sse2 sse4 avx2 PARENT_SCOPE)
		endif()
	elseif (arch STREQUAL "arm")
		set(${outArch} arm PARENT_SCOPE)
		set(${outTargets} neon-i32x4 PARENT_SCOPE)
		set(${outTargetNames} neon PARENT_SCOPE)
	elseif (arch STREQUAL "arm64")
		set(${outArch} aarch64 PARENT_SCOPE)
		set(${outTargets} neon-i32x4 PARENT_SCOPE)
		set(${outTargetNames} neon PARENT_SCOPE)
	else()
		message(FATAL_ERROR "Unsupported CPU architecture ${arch}.")
	endif()
endfunction()

if (NOT CUTTLEFISH_FORCE_INTERNAL_FREEIMAGE)
	find_package(FreeImage QUIET)
endif()
if (CUTTLEFISH_FORCE_INTERNAL_FREEIMAGE OR NOT FreeImage_FOUND)
	message("Building FreeImage from submodule.")
	if (NOT EXISTS ${FREEIMAGE_DIR})
		message(FATAL_ERROR
			"FreeImage not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/FreeImageSources.cmake)
	list(APPEND externalSources ${freeImageSources})
	# NOTE: FreeImage doesn't include the libPNG Neon functions.
	list(APPEND privateDefines DISABLE_PERF_MEASUREMENT=1 PNG_ARM_NEON_OPT=0 FREEIMAGE_LIB
		_LARGEFILE64_SOURCE=1)
	if (WIN32)
		list(APPEND privateDefines OPJ_STATIC LIBRAW_NODLL)
	else()
		list(APPEND privateDefines __ANSI__)
	endif()
	list(APPEND extraIncludeDirs ${freeImageIncludeDirs})
else()
	list(APPEND extraLibraries ${FreeImage_LIBRARIES})
	list(APPEND extraIncludeDirs ${FreeImage_INCLUDE_DIRS})
endif()

if (CUTTLEFISH_BUILD_S3TC)
	if (NOT EXISTS ${SQUISH_DIR})
		message(FATAL_ERROR
			"squish not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/SquishSources.cmake)
	list(APPEND externalSources ${squishSources})
	list(APPEND extraIncludeDirs ${SQUISH_INCLUDE_DIRS})

	if (NOT EXISTS ${COMPRESSONATOR_DIR})
		message(FATAL_ERROR
			"compressonator not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/CompressonatorSources.cmake)
	list(APPEND externalSources ${compressonatorSources})
	list(APPEND extraIncludeDirs ${COMPRESSONATOR_INCLUDE_DIRS})

	if (NOT EXISTS ${BC7ENC_RDO_DIR})
		message(FATAL_ERROR
			"bc7enc_rdo not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/Bc7encRdoSources.cmake)
	list(APPEND externalSources ${bc7encRdoSources})
	list(APPEND extraIncludeDirs ${BC7ENC_RDO_INCLUDE_DIRS})

	if (ISPC)
		if (NOT EXISTS ${ISPC_TEXCOMP_DIR})
			message(FATAL_ERROR
				"ispc_texcomp not found. Run 'git submodule update --init' to pull the submodules.")
		endif()
		include(${CMAKE_CURRENT_LIST_DIR}/IspcTexcompSources.cmake)
		list(APPEND externalSources ${ispcTexcompSources})
		list(APPEND extraIncludeDirs ${ISPC_TEXCOMP_INCLUDE_DIRS})

		list(APPEND ispcSources ${BC7ENC_RDO_DIR}/bc7e.ispc
			${ISPC_TEXCOMP_DIR}/ispc_texcomp/kernel.ispc)
	else()
		message(WARNING "ISPC not found, falling back to lower quality BC6H and BC7 encoders.")
	endif()

	list(APPEND defines CUTTLEFISH_HAS_S3TC=1)
endif()

if (CUTTLEFISH_BUILD_ETC)
	if (NOT EXISTS ${ETC_DIR})
		message(FATAL_ERROR
			"etc2comp not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/Etc2CompSources.cmake)
	list(APPEND externalSources ${etcSources})
	list(APPEND extraIncludeDirs ${ETC_INCLUDE_DIRS})
	list(APPEND defines CUTTLEFISH_HAS_ETC=1)
endif()

if (CUTTLEFISH_BUILD_ASTC)
	if (NOT EXISTS ${ASTC_DIR})
		message(FATAL_ERROR
			"astc-encoder not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/AstcSources.cmake)
	list(APPEND externalSources ${astcSources})
	list(APPEND extraIncludeDirs ${ASTC_INCLUDE_DIRS})
	list(APPEND defines CUTTLEFISH_HAS_ASTC=1)
endif()

if (CUTTLEFISH_BUILD_PVRTC)
	find_package(PVRTexLib QUIET)
	if (PVRTEXLIB_FOUND)
		list(APPEND extraIncludeDirs ${PVRTEXLIB_INCLUDE_DIRS})
		list(APPEND extraLibraries ${PVRTEXLIB_LIBRARIES})
		list(APPEND defines CUTTLEFISH_HAS_PVRTC=1)
	else()
		message("PVRTexLib not found.")
	endif()
endif()

# Setup ISPC compilations.
if (ispcSources)
	set(ispcObjFiles)
	set(ispcTargetObjFiles)
	foreach (source ${ispcSources})
		get_filename_component(baseName ${source} NAME_WLE)
		set(headerPath ${CMAKE_CURRENT_BINARY_DIR}/${baseName}_ispc.h)
		if (APPLE AND CUTTLEFISH_ARCH STREQUAL "multi")
			if (CMAKE_VERSION VERSION_LESS "3.20.0")
				message(FATAL_ERROR
					"CMake 3.20 or higher required for universal binaries with BTPC support.")
			endif()
			if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
				message(FATAL_ERROR "Universal binaries can only be created with Xcode generator.")
			endif()

			list(APPEND ispcTargetObjFiles
				"${CMAKE_CURRENT_BINARY_DIR}/$(CURRENT_ARCH)/${baseName}.o")
			set(first ON)
			foreach (arch ${CMAKE_OSX_ARCHITECTURES})
				set(outputDir ${CMAKE_CURRENT_BINARY_DIR}/${arch})
				set(objPath ${outputDir}/${baseName}.o)

				# Only output the header for the first architecture.
				if (first)
					set(headerArgs -h ${headerPath})
					set(first OFF)
				else()
					set(headerPath)
					set(headerArgs)
				endif()

				get_ispc_arch(ispcArch ispcTarget ispcTargetName ${arch} ON)
				add_custom_command(OUTPUT ${objPath} ${headerPath}
					COMMAND ${CMAKE_COMMAND} -E make_directory ${outputDir}
					COMMAND ${ISPC} -O3 ${source} -o ${objPath} ${headerArgs} --arch=${ispcArch}
						--target=${ispcTarget} --opt=disable-assertions --pic -woff
					MAIN_DEPENDENCY ${source})
				list(APPEND ispcObjFiles ${objPath})
			endforeach()
		else()
			get_ispc_arch(ispcArch ispcTargets ispcTargetNames ${CUTTLEFISH_ARCH} OFF)
			string(REPLACE ";" "," ispcCommandTargets "${ispcTargets}")
			if (MSVC)
				set(objPath ${CMAKE_CURRENT_BINARY_DIR}/${baseName}.obj)
			else()
				set(objPath ${CMAKE_CURRENT_BINARY_DIR}/${baseName}.o)
			endif()
			set(objPaths ${objPath})
			list(LENGTH ispcTargetNames targetCount)
			if (targetCount GREATER 1)
				foreach (target ${ispcTargetNames})
					if (MSVC)
						list(APPEND objPaths ${CMAKE_CURRENT_BINARY_DIR}/${baseName}_${target}.obj)
					else()
						list(APPEND objPaths ${CMAKE_CURRENT_BINARY_DIR}/${baseName}_${target}.o)
					endif()
				endforeach()
			endif()
			add_custom_command(OUTPUT ${objPaths} ${headerPath}
				COMMAND ${ISPC} -O3 ${source} -o ${objPath} -h ${headerPath} --arch=${ispcArch}
					--target=${ispcCommandTargets} --opt=disable-assertions --pic -woff
				MAIN_DEPENDENCY ${source})
			list(APPEND ispcObjFiles ${objPaths})
		endif()
	endforeach()

	if (APPLE AND CUTTLEFISH_ARCH STREQUAL "multi")
		add_custom_target(cuttlefish_ispc_build DEPENDS ${ispcObjFiles})
		add_library(cuttlefish_ispc OBJECT IMPORTED)
		set_property(TARGET cuttlefish_ispc PROPERTY IMPORTED_OBJECTS ${ispcTargetObjFiles})
		add_dependencies(cuttlefish_ispc cuttlefish_ispc_build)
		list(APPEND extraLibraries cuttlefish_ispc)

		cfs_set_folder(cuttlefish_ispc_build libs/internal)
		cfs_set_folder(cuttlefish_ispc libs/internal)
	else()
		# Add sources directly to support older versions of CMake.
		list(APPEND externalSources ${ispcObjFiles})
	endif()

	list(APPEND privateDefines CUTTLEFISH_ISPC=1)
else()
	list(APPEND privateDefines CUTTLEFISH_ISPC=0)
endif()

file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)

# Don't care about warnings for external files.
if (MSVC)
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
else()
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS "-w -Wno-c++11-narrowing")
endif()

add_library(cuttlefish_lib ${CUTTLEFISH_LIB} ${sources} ${externalSources} ${ispcSources})
set_target_properties(cuttlefish_lib PROPERTIES OUTPUT_NAME cuttlefish)
target_include_directories(cuttlefish_lib
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
			glm
			src
			${extraIncludeDirs})
target_link_libraries(cuttlefish_lib PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${extraLibraries})
target_compile_definitions(cuttlefish_lib PUBLIC ${defines} PRIVATE ${privateDefines})

if (CUTTLEFISH_BUILD_PVRTC AND PVRTEXLIB_FOUND)
	# Make sure the PVRTexLib library is available for running in the build directory.
	if (WIN32)
		get_filename_component(pvrTexLibDir ${PVRTEXLIB_LIB} DIRECTORY)
		set(pvrTexLibDll ${pvrTexLibDir}/PVRTexLib.dll)
		add_custom_command(TARGET cuttlefish_lib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
			${pvrTexLibDll} $<TARGET_FILE_DIR:cuttlefish_lib>)
	elseif (APPLE)
		add_custom_command(TARGET cuttlefish_lib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
			${PVRTEXLIB_LIB} $<TARGET_FILE_DIR:cuttlefish_lib>)
	endif()

	# Install the PVRTexLib library when needed.
	if (CUTTLEFISH_INSTALL AND CUTTLEFISH_INSTALL_PVRTEXLIB)
		if (WIN32)
			install(PROGRAMS ${pvrTexLibDll} DESTINATION ${CMAKE_INSTALL_BINDIR})
		else()
			install(PROGRAMS ${PVRTEXLIB_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
		endif()
	endif()
endif()

cfs_set_folder(cuttlefish_lib)
cfs_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
	INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/cuttlefish
	FILES ${sources})
cfs_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} FILES ${externalSources})

cfs_install_library(cuttlefish_lib lib)
set(CUTTLEFISH_DOC_PROJECTS ${CUTTLEFISH_DOC_PROJECTS} lib PARENT_SCOPE)

add_subdirectory(test)
