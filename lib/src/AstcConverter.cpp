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

#include "astcenc.h"
#include "astcenc_internal.h"

// Need to stub out these functions, since they don't work on all compilers. Return true for all,
// since it will only be called if support was compiled in.
int cpu_supports_sse41()
{
	return 1;
}

int cpu_supports_popcnt()
{
	return 1;
}

int cpu_supports_avx2()
{
	return 1;
}

namespace cuttlefish
{

static const unsigned int maxBlockSize = 12;

class AstcConverter::AstcThreadData : public Converter::ThreadData
{
public:
	explicit AstcThreadData(AstcConverter& converter)
	{
		unsigned int flags = 0;
		if (converter.m_alphaType == Texture::Alpha::Standard ||
			converter.m_alphaType == Texture::Alpha::PreMultiplied)
		{
			flags |= ASTCENC_FLG_USE_ALPHA_WEIGHT;
		}
		if (converter.m_colorSpace == ColorSpace::sRGB)
			flags |= ASTCENC_FLG_USE_PERCEPTUAL;

		astcenc_preset preset;
		switch (converter.m_quality)
		{
			case Texture::Quality::Lowest:
				preset = ASTCENC_PRE_FASTEST;
				break;
			case Texture::Quality::Low:
				preset = ASTCENC_PRE_FAST;
				break;
			case Texture::Quality::Normal:
				preset = ASTCENC_PRE_MEDIUM;
				break;
			case Texture::Quality::High:
				preset = ASTCENC_PRE_THOROUGH;
				break;
			case Texture::Quality::Highest:
				preset = ASTCENC_PRE_EXHAUSTIVE;
				break;
			default:
				assert(false);
				return;
		}

		astcenc_config config;
		astcenc_config_init(converter.m_hdr ? ASTCENC_PRF_HDR : ASTCENC_PRF_LDR, converter.m_blockX,
			converter.m_blockY, 1, preset, flags, config);

		astcenc_context_alloc(config, 1, &context);

		dummyImage.dim_x = converter.m_blockX;
		dummyImage.dim_y = converter.m_blockY;
		dummyImage.dim_z = 1;
		dummyImage.data_type = ASTCENC_TYPE_F32;
		dummyImage.data = nullptr;

		if (context->config.v_rgb_mean != 0.0f || context->config.v_rgb_stdev != 0.0f ||
			context->config.v_a_mean != 0.0f || context->config.v_a_stdev != 0.0f)
		{
			context->input_averages = m_inputAverages;
			context->input_variances = m_inputVariances;
			context->input_alpha_averages = m_inputAlphaAverages;

			astcenc_swizzle swizzle;
			swizzle.r = converter.m_colorMask.r ? ASTCENC_SWZ_R : ASTCENC_SWZ_0;
			swizzle.g = converter.m_colorMask.g ? ASTCENC_SWZ_G : ASTCENC_SWZ_0;
			swizzle.b = converter.m_colorMask.b ? ASTCENC_SWZ_B : ASTCENC_SWZ_0;
			if (converter.m_colorMask.a)
			{
				swizzle.a = converter.m_alphaType == Texture::Alpha::None ?
					ASTCENC_SWZ_1 : ASTCENC_SWZ_A;
			}
			else
				swizzle.a = ASTCENC_SWZ_0;
			init_compute_averages_and_variances(dummyImage,  context->config.v_rgb_power,
				context->config.v_a_power, context->config.v_rgba_radius,
				context->config.a_scale_radius, swizzle, context->arg, context->ag);
		}
	}

	~AstcThreadData()
	{
		astcenc_context_free(context);
	}

	astcenc_context* context;
	astcenc_image dummyImage;
	compress_symbolic_block_buffers tempBuffers;

private:
	float4 m_inputAverages[maxBlockSize*maxBlockSize];
	float4 m_inputVariances[maxBlockSize*maxBlockSize];
	float m_inputAlphaAverages[maxBlockSize*maxBlockSize];
};

static bool initialize()
{
	prepare_angular_tables();
	build_quantization_mode_table();
	return true;
}

AstcConverter::AstcConverter(const Texture& texture, const Image& image, unsigned int blockX,
	unsigned int blockY, Texture::Quality quality)
	: Converter(image), m_blockX(blockX), m_blockY(blockY),
	m_jobsX((image.width() + blockX - 1)/blockX), m_jobsY((image.height() + blockY - 1)/blockY),
	m_quality(quality), m_alphaType(texture.alphaType()), m_colorSpace(texture.colorSpace()),
	m_colorMask(texture.colorMask()), m_hdr(texture.type() == Texture::Type::UFloat)
{
	static bool initialized = initialize();
	(void)initialized;

	assert(texture.type() == Texture::Type::UNorm || texture.type() == Texture::Type::UFloat);
	data().resize(m_jobsX*m_jobsY*blockSize);
}

AstcConverter::~AstcConverter()
{
}

void AstcConverter::process(unsigned int x, unsigned int y, ThreadData* threadData)
{
	ColorRGBAf imageData[maxBlockSize*maxBlockSize];
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
			imageData[index] = scanline[scanlineIdx];
			astcBlock.data_r[index] = scanline[scanlineIdx].r;
			astcBlock.data_g[index] = scanline[scanlineIdx].g;
			astcBlock.data_b[index] = scanline[scanlineIdx].b;
			astcBlock.data_a[index] = scanline[scanlineIdx].a;
			astcBlock.rgb_lns[index] = m_hdr;
			astcBlock.alpha_lns[index] = m_hdr;
			astcBlock.nan_texel[index] = false;
		}
	}

	auto astcThreadData = static_cast<AstcThreadData*>(threadData);
	void* imageDataPtr = imageData;
	astcThreadData->dummyImage.data = &imageDataPtr;

	// Fill in the rest of the information.
	imageblock_initialize_work_from_orig(&astcBlock, m_blockX*m_blockY);
	update_imageblock_flags(&astcBlock, m_blockX, m_blockY, 1);

	astcenc_context* context = astcThreadData->context;
	if (astcThreadData->context->input_averages)
	{
		// This assumes that it's going to be a pool of thread jobs, so always make sure there's
		// exactly one job ready.
		context->manage_avg_var.reset();
		context->manage_avg_var.init(1);
		compute_averages_and_variances(*context, astcThreadData->context->ag);
	}

	symbolic_compressed_block symbolicBlock;
	auto block = reinterpret_cast<physical_compressed_block*>(
		data().data() + (y*m_jobsX + x)*blockSize);
	compress_block(*context, astcThreadData->dummyImage, &astcBlock, symbolicBlock, *block,
		&astcThreadData->tempBuffers);
}

std::unique_ptr<Converter::ThreadData> AstcConverter::createThreadData()
{
#if CUTTLEFISH_MSC
#pragma warning(push)
#pragma warning(disable: 4316)
#endif

	return std::unique_ptr<ThreadData>(new AstcThreadData(*this));

#if CUTTLEFISH_MSC
#pragma warning(pop)
#endif
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ASTC
