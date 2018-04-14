/*
 * Copyright 2017 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AstcConverter.h"
#include <cuttlefish/Color.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#if CUTTLEFISH_HAS_ASTC

#if CUTTLEFISH_GCC || CUTTLEFISH_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include "astc_codec_internals.h"
#if CUTTLEFISH_GCC || CUTTLEFISH_CLANG
#pragma GCC diagnostic pop
#endif

// astc-encoder uses global variables to control its operation.
int print_diagnostics;
int print_tile_errors;
int print_statistics;

int perform_srgb_transform;
int rgb_force_use_of_hdr;
int alpha_force_use_of_hdr;

// Stub out functions.
int store_tga_image(const astc_codec_image*, const char*, int)
{
	return 0;
}

astc_codec_image* load_tga_image(const char*, int , int*)
{
	return nullptr;
}

astc_codec_image* load_image_with_stb(const char*, int, int*)
{
	return nullptr;
}

int astc_codec_unlink(const char*)
{
	return 0;
}

void astc_codec_internal_error(const char *filename, int linenum)
{
	std::printf("Internal error: File=%s Line=%d\n", filename, linenum);
	std::exit(1);
}

namespace cuttlefish
{

class AstcConverter::AstcThreadData : public Converter::ThreadData
{
public:
	AstcThreadData()
	{
		m_planes.ei1 = &m_ei1;
		m_planes.ei2 = &m_ei2;
		m_planes.eix1 = m_eix1;
		m_planes.eix2 = m_eix2;
		m_planes.decimated_quantized_weights = m_decimated_quantized_weights;
		m_planes.decimated_weights = m_decimated_weights;
		m_planes.flt_quantized_decimated_quantized_weights =
			m_flt_quantized_decimated_quantized_weights;
		m_planes.u8_quantized_decimated_quantized_weights =
			m_u8_quantized_decimated_quantized_weights;

		tempBuffers.ewb = &m_ewb;
		tempBuffers.ewbo = &m_ewbo;
		tempBuffers.tempblocks = m_tempblocks;
		tempBuffers.temp = &m_temp;
		// Buffers for plane1 is smaller than planes2, so can re-use between them.
		tempBuffers.plane1 = &m_planes;
		tempBuffers.planes2 = &m_planes;
	}

	compress_symbolic_block_buffers tempBuffers;

private:
	endpoints_and_weights m_ei1;
	endpoints_and_weights m_ei2;
	endpoints_and_weights m_eix1[MAX_DECIMATION_MODES];
	endpoints_and_weights m_eix2[MAX_DECIMATION_MODES];
	float m_decimated_quantized_weights[2*MAX_DECIMATION_MODES*MAX_WEIGHTS_PER_BLOCK];
	float m_decimated_weights[2*MAX_DECIMATION_MODES*MAX_WEIGHTS_PER_BLOCK];
	float m_flt_quantized_decimated_quantized_weights[2*MAX_WEIGHT_MODES*MAX_WEIGHTS_PER_BLOCK];
	uint8_t m_u8_quantized_decimated_quantized_weights[2*MAX_WEIGHT_MODES*MAX_WEIGHTS_PER_BLOCK];
	compress_fixed_partition_buffers m_planes;

	error_weight_block m_ewb;
	error_weight_block_orig m_ewbo;
	symbolic_compressed_block m_tempblocks[4];
	imageblock m_temp;
};

std::mutex AstcConverter::m_mutex;

static bool initialize()
{
	prepare_angular_tables();
	build_quantization_mode_table();
	return true;
}

AstcConverter::AstcConverter(const Texture& texture, const Image& image, unsigned int blockX,
	unsigned int blockY, Texture::Quality quality)
	: Converter(image), m_blockX(blockX), m_blockY(blockY),
	m_jobsX((image.width() + blockX - 1)/blockX), m_jobsY((image.height() + blockY - 1)/blockY)
{
	static bool initialized = initialize();
	(void)initialized;

	data().resize(m_jobsX*m_jobsY*blockSize);

	assert(texture.type() == Texture::Type::UNorm || texture.type() == Texture::Type::UFloat);
	m_colorMask = texture.colorMask();
	m_hdr = texture.type() == Texture::Type::UFloat;
	float log10Size = std::log10(static_cast<float>(m_blockX*m_blockY));
	m_alphaWeight = texture.alphaType() == Texture::Alpha::Standard ||
		texture.alphaType() == Texture::Alpha::PreMultiplied;
	switch (quality)
	{
		case Texture::Quality::Lowest:
			m_partitionsToTest = 2;
			m_oplimit = 1.0f;
			m_mincorrel = 0.5f;
			m_averageErrorLimit = std::max(75 - 35*log10Size, 53 - 19*log10Size);
			m_blockModeCutoff = 0.25f;
			m_maxIters = 1;
			break;
		case Texture::Quality::Low:
			m_partitionsToTest = 4;
			m_oplimit = 1.0f;
			m_mincorrel = 0.5f;
			m_averageErrorLimit = std::max(85 - 35*log10Size, 63 - 19*log10Size);
			m_blockModeCutoff = 0.5f;
			m_maxIters = 1;
			break;
		case Texture::Quality::Normal:
			m_partitionsToTest = 25;
			m_oplimit = 1.2f;
			m_mincorrel = 0.75f;
			m_averageErrorLimit = std::max(95 - 35*log10Size, 70 - 19*log10Size);
			m_blockModeCutoff = 0.75f;
			m_maxIters = 2;
			break;
		case Texture::Quality::High:
			m_partitionsToTest = 100;
			m_oplimit = 2.5f;
			m_mincorrel = 0.95f;
			m_averageErrorLimit = std::max(105 - 35*log10Size, 77 - 19*log10Size);
			m_blockModeCutoff = 0.95f;
			m_maxIters = 4;
			break;
		case Texture::Quality::Highest:
			m_partitionsToTest = PARTITION_COUNT;
			m_oplimit = 1000.0f;
			m_mincorrel = 0.99f;
			m_averageErrorLimit = 999.0f;
			m_blockModeCutoff = 1.0f;
			m_maxIters = 4;
			break;
		default:
			assert(false);
			break;
	}

	if (m_hdr)
		m_averageErrorLimit = 0.0f;
	else
		m_averageErrorLimit = std::pow(0.1f, m_averageErrorLimit*0.1f)*65535.0f*65535.0f;

	// ASTC encoder library uses global variables for control.
	m_mutex.lock();
	rgb_force_use_of_hdr = m_hdr;
	alpha_force_use_of_hdr = m_hdr;
}

AstcConverter::~AstcConverter()
{
	m_mutex.unlock();
}

void AstcConverter::process(unsigned int x, unsigned int y, ThreadData* threadData)
{
	imageblock astcBlock;
	astcBlock.xpos = x*m_blockX;
	astcBlock.ypos = y*m_blockY;
	astcBlock.zpos = 0;
	for (unsigned int j = 0, index = 0; j < m_blockY; ++j)
	{
		auto scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(
			std::min(y*m_blockY + j, image().height() - 1)));
		for (unsigned int i = 0; i < m_blockX; ++i, ++index)
		{
			unsigned int scanlineIdx = std::min(x*m_blockX + i, image().width() - 1);
			astcBlock.orig_data[index*4] = scanline[scanlineIdx].r;
			astcBlock.orig_data[index*4 + 1] = scanline[scanlineIdx].g;
			astcBlock.orig_data[index*4 + 2] = scanline[scanlineIdx].b;
			astcBlock.orig_data[index*4 + 3] = scanline[scanlineIdx].a;
			astcBlock.rgb_lns[index] = m_hdr;
			astcBlock.alpha_lns[index] = m_hdr;
			astcBlock.nan_texel[index] = false;
		}
	}

	// Fill in the rest of the information.
	imageblock_initialize_work_from_orig(&astcBlock, m_blockX*m_blockY);
	update_imageblock_flags(&astcBlock, m_blockX, m_blockY, 1);

	// Just need the image for sizing information.
	astc_codec_image dummyImage;
	dummyImage.imagedata8 = nullptr;
	dummyImage.imagedata16 = nullptr;
	dummyImage.xsize = image().width();
	dummyImage.ysize = image().height();
	dummyImage.zsize = 1;
	dummyImage.padding = 0;

	error_weighting_params errorParams;
	errorParams.rgb_power = 1.0f;
	errorParams.alpha_power = 1.0f;
	errorParams.rgb_base_weight = 1.0f;
	errorParams.alpha_base_weight = 1.0f;
	errorParams.rgb_mean_weight = 0.0f;
	errorParams.rgb_stdev_weight = 0.0f;
	errorParams.alpha_mean_weight = 0.0f;
	errorParams.alpha_stdev_weight = 0.0f;

	errorParams.rgb_mean_and_stdev_mixing = 0.0f;
	errorParams.mean_stdev_radius = 0;
	errorParams.enable_rgb_scale_with_alpha = m_alphaWeight;
	errorParams.alpha_radius = 0;

	errorParams.block_artifact_suppression = 0.0f;
	// NOTE: Cannot have a weight of 0.
	errorParams.rgba_weights[0] = m_colorMask.r ? 1.0f : 1e-4f;
	errorParams.rgba_weights[1] = m_colorMask.g ? 1.0f : 1e-4f;
	errorParams.rgba_weights[2] = m_colorMask.b ? 1.0f : 1e-4f;
	errorParams.rgba_weights[3] = m_colorMask.a ? 1.0f : 1e-4f;
	errorParams.ra_normal_angular_scale = 0;
	expand_block_artifact_suppression(m_blockX, m_blockY, 1, &errorParams);

	errorParams.partition_search_limit = m_partitionsToTest;
	errorParams.block_mode_cutoff = m_blockModeCutoff;
	errorParams.texel_avg_error_limit = m_averageErrorLimit;
	errorParams.partition_1_to_2_limit = m_oplimit;
	errorParams.lowest_correlation_cutoff = m_mincorrel;
	errorParams.max_refinement_iters = m_maxIters;

	symbolic_compressed_block symbolicBlock;
	compress_symbolic_block(&dummyImage, m_hdr ? DECODE_HDR : DECODE_LDR, m_blockX, m_blockY, 1,
		&errorParams, &astcBlock, &symbolicBlock,
		&reinterpret_cast<AstcThreadData*>(threadData)->tempBuffers);

	auto block = reinterpret_cast<physical_compressed_block*>(
		data().data() + (y*m_jobsX + x)*blockSize);
	*block = symbolic_to_physical(m_blockX, m_blockY, 1, &symbolicBlock);
}

std::unique_ptr<Converter::ThreadData> AstcConverter::createThreadData()
{
	return std::unique_ptr<ThreadData>(new AstcThreadData);
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ASTC
