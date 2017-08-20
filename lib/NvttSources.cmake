set(CORE_DIR ${NVTT_DIR}/src/nvcore)
set(CORE_SRCS
    ${CORE_DIR}/nvcore.h
    ${CORE_DIR}/Array.h
    ${CORE_DIR}/Debug.h ${CORE_DIR}/Debug.cpp
    ${CORE_DIR}/DefsGnucDarwin.h
    ${CORE_DIR}/DefsGnucLinux.h
    ${CORE_DIR}/DefsGnucWin32.h
    ${CORE_DIR}/DefsVcWin32.h
    ${CORE_DIR}/FileSystem.h ${CORE_DIR}/FileSystem.cpp
    ${CORE_DIR}/ForEach.h
    ${CORE_DIR}/Memory.h ${CORE_DIR}/Memory.cpp
    ${CORE_DIR}/Ptr.h
    ${CORE_DIR}/RefCounted.h
    ${CORE_DIR}/StrLib.h ${CORE_DIR}/StrLib.cpp
    ${CORE_DIR}/Stream.h
    ${CORE_DIR}/StdStream.h
    ${CORE_DIR}/TextWriter.h ${CORE_DIR}/TextWriter.cpp
    ${CORE_DIR}/Timer.h ${CORE_DIR}/Timer.cpp
    ${CORE_DIR}/Utils.h
	${NVTT_DIR}/extern/poshlib/posh.c ${NVTT_DIR}/extern/poshlib/posh.h)

set(MATH_DIR ${NVTT_DIR}/src/nvmath)
set(MATH_SRCS
    ${MATH_DIR}/nvmath.h
    ${MATH_DIR}/Box.h ${MATH_DIR}/Box.inl
    ${MATH_DIR}/Color.h ${MATH_DIR}/Color.inl
    ${MATH_DIR}/Fitting.h ${MATH_DIR}/Fitting.cpp
    ${MATH_DIR}/Gamma.h ${MATH_DIR}/Gamma.cpp
    ${MATH_DIR}/Half.h ${MATH_DIR}/Half.cpp
    ${MATH_DIR}/Matrix.h
    ${MATH_DIR}/Plane.h ${MATH_DIR}/Plane.inl ${MATH_DIR}/Plane.cpp
    ${MATH_DIR}/SphericalHarmonic.h ${MATH_DIR}/SphericalHarmonic.cpp
    ${MATH_DIR}/SimdVector.h ${MATH_DIR}/SimdVector_SSE.h ${MATH_DIR}/SimdVector_VE.h
    ${MATH_DIR}/Vector.h ${MATH_DIR}/Vector.inl)

set(BC6H_DIR ${NVTT_DIR}/src/bc6h)
set(BC6H_SRCS
	${BC6H_DIR}/bits.h
	${BC6H_DIR}/shapes_two.h
	${BC6H_DIR}/tile.h
	${BC6H_DIR}/zoh_utils.cpp
	${BC6H_DIR}/zoh_utils.h
	${BC6H_DIR}/zoh.cpp
	${BC6H_DIR}/zoh.h
	${BC6H_DIR}/zohone.cpp
	${BC6H_DIR}/zohtwo.cpp)

set(BC7_DIR ${NVTT_DIR}/src/bc7)
set(BC7_SRCS
	${BC7_DIR}/avpcl.cpp
	${BC7_DIR}/avpcl.h
	${BC7_DIR}/avpcl_mode0.cpp
	${BC7_DIR}/avpcl_mode1.cpp
	${BC7_DIR}/avpcl_mode2.cpp
	${BC7_DIR}/avpcl_mode3.cpp
	${BC7_DIR}/avpcl_mode4.cpp
	${BC7_DIR}/avpcl_mode5.cpp
	${BC7_DIR}/avpcl_mode6.cpp
	${BC7_DIR}/avpcl_mode7.cpp
	${BC7_DIR}/bits.h
	${BC7_DIR}/endpts.h
	${BC7_DIR}/shapes_three.h
	${BC7_DIR}/shapes_two.h
	${BC7_DIR}/tile.h
	${BC7_DIR}/avpcl_utils.cpp
	${BC7_DIR}/avpcl_utils.h)

set(SQUISH_DIR ${NVTT_DIR}/src/nvtt/squish)
set(SQUISH_SRCS
	${SQUISH_DIR}/fastclusterfit.cpp
	${SQUISH_DIR}/fastclusterfit.h
	${SQUISH_DIR}/weightedclusterfit.cpp
	${SQUISH_DIR}/weightedclusterfit.h
	${SQUISH_DIR}/colourblock.cpp
	${SQUISH_DIR}/colourblock.h
	${SQUISH_DIR}/colourfit.cpp
	${SQUISH_DIR}/colourfit.h
	${SQUISH_DIR}/colourset.cpp
	${SQUISH_DIR}/colourset.h
	${SQUISH_DIR}/config.h
	${SQUISH_DIR}/maths.cpp
	${SQUISH_DIR}/maths.h
	${SQUISH_DIR}/simd.h
	${SQUISH_DIR}/simd_sse.h
	${SQUISH_DIR}/simd_ve.h)

set(IMAGE_DIR ${NVTT_DIR}/src/nvimage)
set(IMAGE_SRCS
	${IMAGE_DIR}/BlockDXT.cpp
	${IMAGE_DIR}/BlockDXT.h
	${IMAGE_DIR}/ColorBlock.cpp
	${IMAGE_DIR}/ColorBlock.h
	${IMAGE_DIR}/Image.cpp
	${IMAGE_DIR}/Image.h)

set(COMPRESS_DIR ${NVTT_DIR}/src/nvtt)
set(COMPRESS_SRCS
	${COMPRESS_DIR}/OptimalCompressDXT.cpp
	${COMPRESS_DIR}/OptimalCompressDXT.h
	${COMPRESS_DIR}/QuickCompressDXT.cpp
	${COMPRESS_DIR}/QuickCompressDXT.h
	${COMPRESS_DIR}/SingleColorLookup.cpp
	${COMPRESS_DIR}/SingleColorLookup.h)

# configuration file
INCLUDE(CheckIncludeFiles)

CHECK_INCLUDE_FILES("unistd.h" HAVE_UNISTD_H)
CHECK_INCLUDE_FILES("stdarg.h" HAVE_STDARG_H)
CHECK_INCLUDE_FILES("signal.h" HAVE_SIGNAL_H)
CHECK_INCLUDE_FILES("execinfo.h" HAVE_EXECINFO_H)
CHECK_INCLUDE_FILES("malloc.h" HAVE_MALLOC_H)
CHECK_INCLUDE_FILES("dispatch/dispatch.h" HAVE_DISPATCH_H)

CONFIGURE_FILE(${NVTT_DIR}/src/nvconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/nvconfig.h)

set(ALL_NVTT_SOURCES
	${CORE_SRCS}
	${MATH_SRCS}
	${BC6H_SRCS}
	${BC7_SRCS}
	${SQUISH_SRCS}
	${IMAGE_SRCS}
	${COMPRESS_SRCS})
set(NVTT_INCLUDE_DIRS ${NVTT_DIR}/src ${NVTT_DIR}/extern/poshlib)
