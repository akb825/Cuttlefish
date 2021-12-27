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
			set(${outTargets} avx1-i32x8 PARENT_SCOPE)
			set(${outTargetNames} avx PARENT_SCOPE)
		else()
			set(${outTargets} sse2-i32x4 sse4-i32x4 avx1-i32x8 avx2-i32x8 PARENT_SCOPE)
			set(${outTargetNames} sse2 sse4 avx avx2 PARENT_SCOPE)
		endif()
	elseif (arch STREQUAL "x86_64")
		set(${outArch} x86-64 PARENT_SCOPE)
		if (singleTarget)
			set(${outTargets} avx1-i32x8 PARENT_SCOPE)
			set(${outTargetNames} avx PARENT_SCOPE)
		else()
			set(${outTargets} sse2-i32x4 sse4-i32x4 avx1-i32x8 avx2-i32x8 PARENT_SCOPE)
			set(${outTargetNames} sse2 sse4 avx avx2 PARENT_SCOPE)
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
if (NOT FreeImage_FOUND)
	message("FreeImage not found")
	if (NOT EXISTS ${FREEIMAGE_DIR})
		message(FATAL_ERROR
			"FreeImage not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/FreeImageSources.cmake)
	# NOTE: FreeImage doesn't include the libPNG Neon functions.
	list(APPEND privateDefines DISABLE_PERF_MEASUREMENT=1 PNG_ARM_NEON_OPT=0)
endif()

if (CUTTLEFISH_BUILD_S3TC)
	if (NOT EXISTS ${SQUISH_DIR})
		message(FATAL_ERROR
			"squish not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/SquishSources.cmake)

	if (NOT EXISTS ${COMPRESSONATOR_DIR})
		message(FATAL_ERROR
			"compressonator not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/CompressonatorSources.cmake)

	if (NOT EXISTS ${BC7ENC_RDO_DIR})
		message(FATAL_ERROR
			"bc7enc_rdo not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/Bc7encRdoSources.cmake)

	if (ISPC)
		if (NOT EXISTS ${ISPC_TEXCOMP_DIR})
			message(FATAL_ERROR
				"ispc_texcomp not found. Run 'git submodule update --init' to pull the submodules.")
		endif()
		include(${CMAKE_CURRENT_LIST_DIR}/IspcTexcompSources.cmake)

		list(APPEND ispcSources ${BC7ENC_RDO_DIR}/bc7e.ispc
			${ISPC_TEXCOMP_DIR}/ispc_texcomp/kernel.ispc)
	else()
		message(WARNING "ISPC not found, falling back to lower quality BC6H and BC7 encoders.")
	endif()

	if (CUTTLEFISH_ARCH MATCHES "^x86")
		list(APPEND privateDefines SQUISH_USE_SSE=2)
	elseif (CUTTLEFISH_ARCH MATCHES "^arm")
		list(APPEND privateDefines SQUISH_USE_SSE=0)
	endif()

	list(APPEND defines CUTTLEFISH_HAS_S3TC=1)
endif()

if (CUTTLEFISH_BUILD_ETC)
	if (NOT EXISTS ${ETC_DIR})
		message(FATAL_ERROR
			"etc2comp not found. Run 'git submodule update --init' to pull the submodules.")
	endif()
	include(${CMAKE_CURRENT_LIST_DIR}/Etc2CompSources.cmake)
	list(APPEND defines CUTTLEFISH_HAS_ETC=1)
endif()

if (CUTTLEFISH_BUILD_ASTC)
	if (NOT EXISTS ${ASTC_DIR})
		message(FATAL_ERROR
			"astc-encoder not found. Run 'git submodule update --init' to pull the submodules.")
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
	endif()
endif()

# Setup ISPC compilations.
if (ispcSources)
	set(ispcObjFiles)
	set(ispcTargetObjFiles)
	foreach (source ${ispcSources})
		get_filename_component(baseName ${source} NAME_WLE)
		set(headerPath ${CMAKE_CURRENT_BINARY_DIR}/${baseName}_ispc.h)
		if (APPLE AND CTTULEFISH_ARCH STREQUAL "multi")
			if (CMAKE_VERSION VERSION_LESS "3.20.0")
				message(FATAL_ERROR
					"CMake 3.20 or higher required for universal binaries with BTPC support.")
			endif()

			list(APPEND ispcTargetObjFiles
				"${CMAKE_CURRENT_BINARY_DIR}/$(CURRENT_ARCH)/${baseName}.o")
			foreach (arch ${CMAKE_OSX_ARCHITECTURES})
				set(objPath ${CMAKE_CURRENT_BINARY_DIR}/${arch}/${baseName}.o)
				get_ispc_arch(ispcArch ispcTarget ispcTargetName ${arch} ON)
				add_custom_command(OUTPUT ${objPath} ${headerPath}
					COMMAND ${ISPC} -O3 ${source} -o ${objPath} -h ${headerPath} --arch=${ispcArch}
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
			list(APPEND ispcTargetObjFiles ${objPaths})
		endif()
	endforeach()

	if (APPLE AND CTTULEFISH_ARCH STREQUAL "multi")
		add_custom_target(cuttlefish_ispc_build DEPENDS ${ispcObjFiles})
		add_library(cuttlefish_ispc OBJECT IMPORTED)
		set_property(TARGET cuttlefish_ispc IMPORTED_OBJECTS ${ispcCommandTargets})

		cfs_set_folder(cuttlefish_ispc_build libs/internal)
		cfs_set_folder(cuttlefish_ispc libs/internal)
	else()
		# Add sources directly to support older versions of CMake.
		set(ispcObjSources ${ispcObjFiles})
	endif()

	list(APPEND privateDefines CUTTLEFISH_ISPC=1)
else()
	list(APPEND privateDefines CUTTLEFISH_ISPC=0)
endif()

file(GLOB_RECURSE sources src/*.cpp src/*.h include/*.h)
set(externalSources ${freeImageSources} ${squishSources} ${compressonatorSources}
	${bc7encRdoSources} ${ispcTexcompSources} ${ispcObjSources} ${etcSources} ${astcSources}
	${ispcSources})

# Don't care about warnings for external files.
if (MSVC)
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
else()
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS "-w -Wno-c++11-narrowing")
endif()

add_library(cuttlefish_lib ${CUTTLEFISH_LIB} ${sources} ${externalSources})
set_target_properties(cuttlefish_lib PROPERTIES OUTPUT_NAME cuttlefish)
target_include_directories(cuttlefish_lib
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
			${FreeImage_INCLUDE_DIRS}
			${SQUISH_INCLUDE_DIRS}
			${COMPRESSONATOR_INCLUDE_DIRS}
			${BC7ENC_RDO_INCLUDE_DIRS}
			${ISPC_TEXCOMP_INCLUDE_DIRS}
			${ETC_INCLUDE_DIRS}
			${ASTC_INCLUDE_DIRS}
			glm
			src)
target_link_libraries(cuttlefish_lib PRIVATE ${FreeImage_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(cuttlefish_lib PUBLIC ${defines} PRIVATE ${privateDefines})

# Special per-architecture defines for XCode.
if (CUTTLEFISH_ARCH STREQUAL "multi")
	set_target_properties(cuttlefish_lib PROPERTIES
		XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$(CUTTLEFISH_FLAGS_$(CURRENT_ARCH))"
		XCODE_ATTRIBUTE_CUTTLEFISH_FLAGS_i386 "SQUISH_USE_SSE=2"
		XCODE_ATTRIBUTE_CUTTLEFISH_FLAGS_x86_64 "SQUISH_USE_SSE=2"
		XCODE_ATTRIBUTE_CUTTLEFISH_FLAGS_arm "SQUISH_USE_SSE=0"
		XCODE_ATTRIBUTE_CUTTLEFISH_FLAGS_arm64 "SQUISH_USE_SSE=0")
endif()

if (CUTTLEFISH_BUILD_PVRTC AND PVRTEXLIB_FOUND)
	# Delay linking to PVRTexLib to here in case CUTTLEFISH_BUILD_PVRTC was toggled and the include
	# dirs and library variables are still set.
	target_include_directories(cuttlefish_lib PRIVATE ${PVRTEXLIB_INCLUDE_DIRS})
	target_link_libraries(cuttlefish_lib PRIVATE ${PVRTEXLIB_LIBRARIES})

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
