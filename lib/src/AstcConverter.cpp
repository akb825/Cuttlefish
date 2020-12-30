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

// Need to stub out these functions, since they don't work on all compilers.
int cpu_supports_sse41()
{
	return 0;
}

int cpu_supports_popcnt()
{
	return 0;
}

int cpu_supports_avx2()
{
	return 0;
}

namespace cuttlefish
{

class AstcConverter::AstcThreadData : public Converter::ThreadData
{
public:
	compress_symbolic_block_buffers tempBuffers;
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
	m_jobsX((image.width() + blockX - 1)/blockX), m_jobsY((image.height() + blockY - 1)/blockY)
{
	static bool initialized = initialize();
	(void)initialized;

	data().resize(m_jobsX*m_jobsY*blockSize);

	assert(texture.type() == Texture::Type::UNorm || texture.type() == Texture::Type::UFloat);
	m_colorMask = texture.colorMask();
	m_hdr = texture.type() == Texture::Type::UFloat;

	// Just need the image for sizing information.
	m_dummyImage.reset(new astcenc_image);
	m_dummyImage->dim_x = image.width();
	m_dummyImage->dim_y = image.height();
	m_dummyImage->dim_z = 1;
	m_dummyImage->data_type = m_hdr ? ASTCENC_TYPE_F16 : ASTCENC_TYPE_U8;
	m_dummyImage->data = nullptr;

	unsigned int flags = 0;
	if (texture.alphaType() == Texture::Alpha::Standard ||
		texture.alphaType() == Texture::Alpha::PreMultiplied)
	{
		flags |= ASTCENC_FLG_USE_ALPHA_WEIGHT;
	}
	if (texture.colorSpace() == ColorSpace::sRGB)
		flags |= ASTCENC_FLG_USE_PERCEPTUAL;
	astcenc_config config;
	astcenc_config_init(m_hdr ? ASTCENC_PRF_HDR : ASTCENC_PRF_LDR, m_blockX, m_blockY, 1,
		static_cast<astcenc_preset>(quality), flags, config);

	astcenc_context_alloc(config, 1, &m_context);

	if (m_context->config.v_rgb_mean != 0.0f || m_context->config.v_rgb_stdev != 0.0f ||
	    m_context->config.v_a_mean != 0.0f || m_context->config.v_a_stdev != 0.0f)
	{
		unsigned int texelCount = image.width()*image.height();
		m_context->input_averages = new float4[texelCount];
		m_context->input_variances = new float4[texelCount];
		m_context->input_alpha_averages = new float[texelCount];

		astcenc_swizzle swizzle;
		swizzle.r = texture.colorMask().r ? ASTCENC_SWZ_R : ASTCENC_SWZ_0;
		swizzle.g = texture.colorMask().g ? ASTCENC_SWZ_G : ASTCENC_SWZ_0;
		swizzle.b = texture.colorMask().b ? ASTCENC_SWZ_B : ASTCENC_SWZ_0;
		if (texture.colorMask().a)
			swizzle.a = texture.alphaType() == Texture::Alpha::None ? ASTCENC_SWZ_1 : ASTCENC_SWZ_A;
		else
			swizzle.a = ASTCENC_SWZ_0;
		init_compute_averages_and_variances(*m_dummyImage,  m_context->config.v_rgb_power,
			m_context->config.v_a_power, m_context->config.v_rgba_radius,
			m_context->config.a_scale_radius, swizzle, m_context->arg, m_context->ag);
		compute_averages_and_variances(*m_context, m_context->ag);
	}
}

AstcConverter::~AstcConverter()
{
	delete[] m_context->input_averages;
	delete[] m_context->input_variances;
	delete[] m_context->input_alpha_averages;
	astcenc_context_free(m_context);
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
			astcBlock.data_r[index] = scanline[scanlineIdx].r;
			astcBlock.data_g[index] = scanline[scanlineIdx].g;
			astcBlock.data_b[index] = scanline[scanlineIdx].b;
			astcBlock.data_a[index] = scanline[scanlineIdx].a;
			astcBlock.rgb_lns[index] = m_hdr;
			astcBlock.alpha_lns[index] = m_hdr;
			astcBlock.nan_texel[index] = false;
		}
	}

	// Fill in the rest of the information.
	imageblock_initialize_work_from_orig(&astcBlock, m_blockX*m_blockY);
	update_imageblock_flags(&astcBlock, m_blockX, m_blockY, 1);

	symbolic_compressed_block symbolicBlock;
	auto block = reinterpret_cast<physical_compressed_block*>(
		data().data() + (y*m_jobsX + x)*blockSize);
	compress_block(*m_context, *m_dummyImage, &astcBlock, symbolicBlock, *block,
		&reinterpret_cast<AstcThreadData*>(threadData)->tempBuffers);
}

std::unique_ptr<Converter::ThreadData> AstcConverter::createThreadData()
{
	return std::unique_ptr<ThreadData>(new AstcThreadData);
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ASTC
