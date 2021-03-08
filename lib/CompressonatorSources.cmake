set(compressonatorSources
	${COMPRESSONATOR_DIR}/cmp_core/shaders/bc4_encode_kernel.cpp
	${COMPRESSONATOR_DIR}/cmp_core/shaders/bc4_encode_kernel.h
	${COMPRESSONATOR_DIR}/cmp_core/shaders/bc5_encode_kernel.cpp
	${COMPRESSONATOR_DIR}/cmp_core/shaders/bc5_encode_kernel.h
	${COMPRESSONATOR_DIR}/cmp_core/shaders/bc6_encode_kernel.cpp
	${COMPRESSONATOR_DIR}/cmp_core/shaders/bc6_encode_kernel.h
)

set(COMPRESSONATOR_INCLUDE_DIRS ${COMPRESSONATOR_DIR}/cmp_core/source
	${COMPRESSONATOR_DIR}/cmp_core/shaders)
