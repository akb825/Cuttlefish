file(GLOB_RECURSE sources *.cpp *.h)
add_executable(cuttlefish ${sources})

target_link_libraries(cuttlefish PRIVATE cuttlefish_lib)

cfs_set_folder(cuttlefish tools)

find_package(PVRTexLib QUIET)
if (APPLE AND CUTTLEFISH_BUILD_PVRTC AND PVRTEXLIB_FOUND AND NOT CUTTLEFISH_SHARED)
	cfs_fixup_mac_dep(cuttlefish "@executable_path/../lib/libPVRTexLib.dylib"
		"@rpath/libPVRTexLib.dylib")
endif()

if (CUTTLEFISH_INSTALL)
	install(TARGETS cuttlefish RUNTIME DESTINATION bin COMPONENT tool)
endif()

set(testPath ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(cuttlefishPath $<TARGET_FILE:cuttlefish>)
if (WIN32)
	set(commandPath ${testPath}/run-test.bat)
	set(nullFile NUL)
else()
	set(commandPath ${testPath}/run-test.sh)
	set(nullFile /dev/null)
endif()

# NOTE: There will be some weird capiltalization in these tests. This is intentional to make sure
# some of the options (ones not prefixed with - or --) are case-insensitive.
add_test(NAME CuttlefishNoArgs
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "" 1)
add_test(NAME CuttlefishHelp
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-h" 1)
add_test(NAME CuttlefishSaveDds
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -o ${nullFile} --file-format dds" 0)
add_test(NAME CuttlefishSaveKtx
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f r8g8b8a8 -o ${nullFile} --file-format ktx" 0)
add_test(NAME CuttlefishSavePvr
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveUnknownFileType
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -o ${nullFile} --file-format asdf" 1)
add_test(NAME CuttlefishSaveMissingFileTypeParam
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -o ${nullFile} --file-format" 1)
add_test(NAME CuttlefishMissingOutput
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --file-format pvr" 1)
add_test(NAME CuttlefishMultipleOutput
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -o ${nullFile} -o ${nullFile} --file-format pvr" 1)

if (CUTTLEFISH_BUILD_S3TC)
	add_test(NAME CuttlefishSaveBC3
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f BC3 -o ${nullFile} --file-format PVR" 0)
	add_test(NAME CuttlefishSaveBC3sRGB
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f BC3 --srgb -o ${nullFile} --file-format pvr" 0)
	add_test(NAME CuttlefishSaveBC6H
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f BC6H -o ${nullFile} --file-format pvr" 0)
	add_test(NAME CuttlefishSaveBC6HUNorm
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f BC6H -t UNorm -o ${nullFile} --file-format pvr" 1)
	add_test(NAME CuttlefishSaveBC6HUFloat
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f BC6H -t UFloat -o ${nullFile} --file-format pvr" 0)
	add_test(NAME CuttlefishSaveBC6HFloat
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f BC6H -t Float -o ${nullFile} --file-format pvr" 0)
endif()

if (CUTTLEFISH_BUILD_ETC)
	add_test(NAME CuttlefishSaveETC2
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f etc2_R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
	add_test(NAME CuttlefishSaveETC2sRGB
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ETC2_r8g8b8a8 --srgb -o ${nullFile} --file-format pvr" 0)
endif()

if (CUTTLEFISH_BUILD_ASTC)
	add_test(NAME CuttlefishSaveASTC
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ASTC_5x5 -o ${nullFile} --file-format pvr" 0)
	add_test(NAME CuttlefishSaveASTCHDR
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ASTC_5x5 -t UFLOAT -o ${nullFile} --file-format pvr" 0)
	add_test(NAME CuttlefishSaveASTCsRGB
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ASTC_5x5 --srgb -o ${nullFile} --file-format pvr" 0)
	add_test(NAME CuttlefishSaveASTCLowest
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ASTC_5x5 -o ${nullFile} -Q lowest --file-format pvr" 0)
	add_test(NAME CuttlefishSaveASTCLow
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ASTC_5x5 -o ${nullFile} -Q low --file-format pvr" 0)
	add_test(NAME CuttlefishSaveASTCHigh
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ASTC_5x5 -o ${nullFile} -Q high --file-format pvr" 0)
	add_test(NAME CuttlefishSaveASTCHightest
		WORKING_DIRECTORY ${testPath}
		COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f ASTC_5x5 -o ${nullFile} -Q highest --file-format pvr" 0)
endif()

if (CUTTLEFISH_BUILD_PVRTC)
	if (PVRTEXLIB_FOUND)
		add_test(NAME CuttlefishSavePVRTC
			WORKING_DIRECTORY ${testPath}
			COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f PVRTC1_RGB_4BPP -o ${nullFile} --file-format pvr" 0)
		add_test(NAME CuttlefishSavePVRTCsRGB
			WORKING_DIRECTORY ${testPath}
			COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f PVRTC1_RGB_4BPP --srgb -o ${nullFile} --file-format pvr" 0)
	endif()
endif()

add_test(NAME CuttlefishSaveB10G11R11_UFloat
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f B10G11R11_UFloat -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveE5B9G9R9_UFloat
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f E5B9G9R9_UFloat -o ${nullFile} --file-format pvr" 0)

add_test(NAME CuttlefishSavesRGB
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --srgb -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveUNorm
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R16G16B16A16 -t unorm -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveSNorm
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R16G16B16A16 -t snorm -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveUInt
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R16G16B16A16 -t uint -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveInt
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R16G16B16A16 -t int -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveUFloat
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R16G16B16A16 -t ufloat -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishSaveFloat
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R16G16B16A16 -t float -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveUnknownType
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R16G16B16A16 -t asdf -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishSaveUnknownFormat
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f asdf -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishSaveMissingFormat
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png  -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishSaveMissingInput
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishSaveInputNotFound
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i asdf.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 2)

add_test(NAME CuttlefishSaveArray
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-a array0.png -a 1 array1.png --array array2.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSaveArrayDuplicateIndex
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-a array0.png -a 0 array1.png --array array2.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishSaveArrayMissingIndex
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-a array0.png -a array1.png --array 3 array2.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishSave3D
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-a array0.png -a 1 array1.png --array array2.png -d 3 -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)

add_test(NAME CuttlefishCube
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-c +X posx.png -c +Y posy.png -c +Z posz.png -c -x negx.png -c -y negy.png --cube -z negz.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishCubeMissingFace
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-c +y posy.png -c +z posz.png -c -x negx.png -c -y negy.png --cube -z negz.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishCubeDuplicateFace
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-c +x posx.png -c +x posx.png -c +y posy.png -c +z posz.png -c -x negx.png -c -y negy.png --cube -z negz.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishCubeArray
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-C 0 +x posx.png -C 0 +y posy.png -C 0 +z posz.png -C 0 -X negx.png -C 0 -Y negy.png --cube-array 0 -Z negz.png -C 1 +x posx.png -C 1 +y posy.png -C 1 +z posz.png -C 1 -x negx.png -C 1 -y negy.png --cube-array 1 -z negz.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishCubeArrayMissingFace
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-C 0 +y posy.png -C 0 +z posz.png -C 0 -x negx.png -C 0 -y negy.png --cube-array 0 -z negz.png -C 1 +x posx.png -C 1 +y posy.png -C 1 +z posz.png -C 1 -x negx.png -C 1 -y negy.png --cube-array 1 -z negz.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishCubeArrayDuplicateFace
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-C 0 +x posx.png -C 0 +x posx.png -C 0 +y posy.png -C 0 +z posz.png -C 0 -x negx.png -C 0 -y negy.png --cube-array 0 -z negz.png -C 1 +x posx.png -C 1 +y posy.png -C 1 +z posz.png -C 1 -x negx.png -C 1 -y negy.png --cube-array 1 -z negz.png -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishImageList
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I image image.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishImageListNotFound
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I image asdf.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishImageListMultipleImages
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I image array.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishArrayList
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "--input-list array array.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)

add_test(NAME CuttlefishCubeList
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I cube cube.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishInvalidCubeList
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I cube array.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishCubeArrayList
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I cube-array cube-array.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishInvalidCubeArrayList
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I cube-array array.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishUnknownList
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-I asdf array.txt -f R8G8B8A8 -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishResize
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --resize 31 42 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishResizeBox
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -r 31 42 Box -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishResizeLinear
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -r 31 42 lInear -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishResizeCubic
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -r 31 42 cuBic -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishResizeCatmullRom
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -r 31 42 catmull-Rom -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishResizePO2
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -r nextPO2 Nearestpo2 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishResizeInvalidX
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -r asdf nearestPO2 -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishResizeInvalidY
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -r nextpo2 asdf -o ${nullFile} --file-format pvr" 1)

add_test(NAME CuttlefishMipmap
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -m -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishMipmapLevels
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --mipmap 1 -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishMipmapFilter
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -m Box -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishMipmapLevelsFilter
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -m 1 box -o ${nullFile} --file-format pvr" 0)

add_test(NAME CuttlefishNormalimap
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -n -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishNormalimapHeight
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --normalmap 2.3 -o ${nullFile} --file-format pvr" 0)

add_test(NAME CuttlefishAlphaNone
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --alpha nonE -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishAlphaStandard
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --alpha Standard -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishAlphaPreMultiplied
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --alpha pre-muLtiplied -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishEncoded
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 --alpha encodeD -o ${nullFile} --file-format pvr" 0)

add_test(NAME CuttlefishSwizzle
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -s abgr -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSwizzleAlternate
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -s RgXb -o ${nullFile} --file-format pvr" 0)
add_test(NAME CuttlefishSwizzleInvalidNumber
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -s rgb -o ${nullFile} --file-format pvr" 1)
add_test(NAME CuttlefishSwizzleInvalidChannel
	WORKING_DIRECTORY ${testPath}
	COMMAND ${commandPath} ${cuttlefishPath} "-i texture.png -f R8G8B8A8 -s abcd -o ${nullFile} --file-format pvr" 1)
