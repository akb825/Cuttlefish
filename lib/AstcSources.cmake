set(astcSources
	${ASTC_DIR}/Source/astcenc.h
	${ASTC_DIR}/Source/astcenc_averages_and_directions.cpp
	${ASTC_DIR}/Source/astcenc_block_sizes.cpp
	${ASTC_DIR}/Source/astcenc_color_quantize.cpp
	${ASTC_DIR}/Source/astcenc_color_unquantize.cpp
	${ASTC_DIR}/Source/astcenc_compress_symbolic.cpp
	${ASTC_DIR}/Source/astcenc_compute_variance.cpp
	${ASTC_DIR}/Source/astcenc_decompress_symbolic.cpp
	${ASTC_DIR}/Source/astcenc_entry.cpp
	${ASTC_DIR}/Source/astcenc_find_best_partitioning.cpp
	${ASTC_DIR}/Source/astcenc_ideal_endpoints_and_weights.cpp
	${ASTC_DIR}/Source/astcenc_image.cpp
	${ASTC_DIR}/Source/astcenc_integer_sequence.cpp
	${ASTC_DIR}/Source/astcenc_internal.h
	${ASTC_DIR}/Source/astcenc_mathlib_softfloat.cpp
	${ASTC_DIR}/Source/astcenc_mathlib.cpp
	${ASTC_DIR}/Source/astcenc_mathlib.h
	${ASTC_DIR}/Source/astcenc_partition_tables.cpp
	${ASTC_DIR}/Source/astcenc_percentile_tables.cpp
	${ASTC_DIR}/Source/astcenc_pick_best_endpoint_format.cpp
	${ASTC_DIR}/Source/astcenc_quantization.cpp
	${ASTC_DIR}/Source/astcenc_symbolic_physical.cpp
	${ASTC_DIR}/Source/astcenc_weight_align.cpp
	${ASTC_DIR}/Source/astcenc_weight_quant_xfer_tables.cpp
)

set(ASTC_INCLUDE_DIRS ${ASTC_DIR}/Source)
