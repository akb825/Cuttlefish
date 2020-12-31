/*
 * Copyright 2017-2020 Aaron Barany
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

#include "S3tcConverter.h"
#include "Shared.h"
#include <cuttlefish/Color.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <limits>

#if CUTTLEFISH_HAS_S3TC

#if CUTTLEFISH_CLANG || CUTTLEFISH_GCC
#pragma GCC diagnostic push

#if CUTTLEFISH_CLANG
// NOTE: Older versions of clang don't have -Wexpansion-to-defined. This include's Apple's version,
// which doesn't follow clang's version numbering scheme. Since they may add it later, disable
// the warning for unknown pragmas.
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#endif

#if (CUTTLEFISH_GCC && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)))
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

#include "bc7enc.h"
#include "bc6h/zoh.h"
#include "bc7/avpcl.h"
#include "nvimage/BlockDXT.h"
#include "nvimage/ColorBlock.h"
#include "nvimage/ImageIO.h"
#include "nvmath/Half.h"
#include "nvmath/Vector.inl"
#include "nvtt/squish/colourset.h"
#include "nvtt/squish/weightedclusterfit.h"
#include "nvtt/OptimalCompressDXT.h"
#include "nvtt/QuickCompressDXT.h"
#include "nvtt/icbc.h"

#if CUTTLEFISH_CLANG || CUTTLEFISH_GCC
#pragma GCC diagnostic pop
#endif

// Need to #include this after the NVidia Texture Tools includes since it #undef's assert.
#include <cassert>

// Stub out functions.
namespace nv
{

Image* ImageIO::load(const char*)
{
	return nullptr;
}

} // namespace nv

namespace cuttlefish
{

static nv::ColorBlock toColorBlock(const ColorRGBAf* blockColors)
{
	nv::ColorBlock colorBlock;
	for (unsigned int i = 0; i < S3tcConverter::blockDim*S3tcConverter::blockDim; ++i)
	{
		nv::Color32& color = colorBlock.color(i);
		color.r = static_cast<std::uint8_t>(std::round(clamp(blockColors[i].r, 0.0f, 1.0f)*0xFF));
		color.g = static_cast<std::uint8_t>(std::round(clamp(blockColors[i].g, 0.0f, 1.0f)*0xFF));
		color.b = static_cast<std::uint8_t>(std::round(clamp(blockColors[i].b, 0.0f, 1.0f)*0xFF));
		color.a = static_cast<std::uint8_t>(std::round(clamp(blockColors[i].a, 0.0f, 1.0f)*0xFF));
	}

	return colorBlock;
}

static void toBc7ColorBlock(uint8_t outBlock[16][4], const ColorRGBAf* blockColors)
{
	for (unsigned int i = 0; i < S3tcConverter::blockDim*S3tcConverter::blockDim; ++i)
	{
		outBlock[i][0] =
			static_cast<std::uint8_t>(std::round(clamp(blockColors[i].r, 0.0f, 1.0f)*0xFF));
		outBlock[i][1] =
			static_cast<std::uint8_t>(std::round(clamp(blockColors[i].g, 0.0f, 1.0f)*0xFF));
		outBlock[i][2] =
			static_cast<std::uint8_t>(std::round(clamp(blockColors[i].b, 0.0f, 1.0f)*0xFF));
		outBlock[i][3] =
			static_cast<std::uint8_t>(std::round(clamp(blockColors[i].a, 0.0f, 1.0f)*0xFF));
	}
}

static nv::AlphaBlock4x4 toAlphaBlock(const ColorRGBAf* blockColors, unsigned int channel,
	bool keepSign)
{
	nv::AlphaBlock4x4 alphaBlock;
	if (keepSign)
	{
		for (unsigned int i = 0; i < S3tcConverter::blockDim*S3tcConverter::blockDim; ++i)
		{
			float value = (reinterpret_cast<const float*>(blockColors + i)[channel] + 0.5f) + 0.5f;
			alphaBlock.alpha[i] =
				static_cast<std::uint8_t>(std::round(clamp(value, 0.0f, 1.0f)*0xFF));
			alphaBlock.weights[i] = 1.0f;
		}
	}
	else
	{
		for (unsigned int i = 0; i < S3tcConverter::blockDim*S3tcConverter::blockDim; ++i)
		{
			float value = (reinterpret_cast<const float*>(blockColors + i)[channel]);
			alphaBlock.alpha[i] =
				static_cast<std::uint8_t>(std::round(clamp(value, 0.0f, 1.0f)*0xFF));
			alphaBlock.weights[i] = 1.0f;
		}
	}

	return alphaBlock;
}

static void clampColors(ColorRGBAf* blockColors)
{
	for (unsigned int i = 0; i < S3tcConverter::blockDim*S3tcConverter::blockDim; ++i)
	{
		blockColors[i].r = clamp(blockColors[i].r, 0.0f, 1.0f);
		blockColors[i].g = clamp(blockColors[i].g, 0.0f, 1.0f);
		blockColors[i].b = clamp(blockColors[i].b, 0.0f, 1.0f);
		blockColors[i].a = clamp(blockColors[i].a, 0.0f, 1.0f);
	}
}

static void createWeights(float* weights, const ColorRGBAf* blockColors, bool weight)
{
	if (weight)
	{
		for (unsigned int i = 0; i < S3tcConverter::blockDim*S3tcConverter::blockDim; ++i)
			weights[i] = blockColors[i].a;
	}
	else
	{
		for (unsigned int i = 0; i < S3tcConverter::blockDim*S3tcConverter::blockDim; ++i)
			weights[i] = 1.0f;
	}
}

static nv::Vector3 createChannelWeights(Texture::ColorMask colorMask)
{
	return nv::Vector3(colorMask.r ? 1.0f : 0.0f, colorMask.g ? 1.0f : 0.0f,
		colorMask.b ? 1.0f : 0.0f);
}

static void setChannelWeights(nvsquish::WeightedClusterFit& fit, Texture::ColorMask colorMask)
{
	fit.SetMetric(colorMask.r ? 1.0f : 0.0f, colorMask.g ? 1.0f : 0.0f, colorMask.b ? 1.0f : 0.0f);
}

static bc7enc_compress_block_params createBc7BlockParams(const S3tcConverter& converter)
{
	bc7enc_compress_block_params params;

	switch (converter.quality())
	{
		case Texture::Quality::Lowest:
			params.m_max_partitions_mode = 0;
			params.m_uber_level = 0;
			params.m_try_least_squares = false;
			params.m_mode_partition_estimation_filterbank = true;
			bc7enc_compress_block_params_init_linear_weights(&params);
			break;
		case Texture::Quality::Low:
			params.m_max_partitions_mode = 16;
			params.m_uber_level = 0;
			params.m_try_least_squares = true;
			params.m_mode_partition_estimation_filterbank = true;
			bc7enc_compress_block_params_init_linear_weights(&params);
			break;
		case Texture::Quality::Normal:
			params.m_max_partitions_mode = BC7ENC_MAX_PARTITIONS1;
			params.m_uber_level = 1;
			params.m_try_least_squares = true;
			params.m_mode_partition_estimation_filterbank = false;
			if (converter.image().colorSpace() == ColorSpace::sRGB)
				bc7enc_compress_block_params_init_perceptual_weights(&params);
			else
				bc7enc_compress_block_params_init_linear_weights(&params);
			break;
		case Texture::Quality::High:
		case Texture::Quality::Highest:
			params.m_max_partitions_mode = BC7ENC_MAX_PARTITIONS1;
			params.m_uber_level = 4;
			params.m_try_least_squares = true;
			params.m_mode_partition_estimation_filterbank = false;
			if (converter.image().colorSpace() == ColorSpace::sRGB)
				bc7enc_compress_block_params_init_perceptual_weights(&params);
			else
				bc7enc_compress_block_params_init_linear_weights(&params);
			break;
		default:
			assert(false);
			break;
	}

	if (!converter.colorMask().r)
		params.m_weights[0] = 0;
	if (!converter.colorMask().g)
		params.m_weights[1] = 0;
	if (!converter.colorMask().b)
		params.m_weights[2] = 0;
	if (!converter.colorMask().a)
		params.m_weights[3] = 0;

	return params;
}

S3tcConverter::S3tcConverter(const Texture& texture, const Image& image, unsigned int blockSize,
	Texture::Quality quality)
	: Converter(image), m_blockSize(blockSize),
	m_jobsX((image.width() + blockDim - 1)/blockDim),
	m_jobsY((image.height() + blockDim - 1)/blockDim),
	m_quality(quality), m_colorMask(texture.colorMask()),
	m_weightAlpha(texture.alphaType() == Texture::Alpha::Standard ||
		texture.alphaType() == Texture::Alpha::PreMultiplied)
{
	data().resize(m_jobsX*m_jobsY*m_blockSize);
}

void S3tcConverter::process(unsigned int x, unsigned int y, ThreadData*)
{
	void* block = data().data() + (y*m_jobsX + x)*m_blockSize;
	ColorRGBAf blockColors[blockDim][blockDim];
	for (unsigned int j = 0; j < blockDim; ++j)
	{
		auto scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(
			std::min(y*blockDim + j, image().height() - 1)));
		for (unsigned int i = 0; i < blockDim; ++i)
			blockColors[j][i] = scanline[std::min(x*blockDim + i, image().width() - 1)];
	}

	compressBlock(block, reinterpret_cast<ColorRGBAf*>(blockColors));
}

Bc1Converter::Bc1Converter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 8, quality)
{
	icbc::init_dxt1();
}

void Bc1Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	auto dxtBlock = reinterpret_cast<nv::BlockDXT1*>(block);
	if (quality() == Texture::Quality::Lowest || quality() == Texture::Quality::Low)
	{
		nv::QuickCompress::compressDXT1(toColorBlock(blockColors), dxtBlock);
		return;
	}

	clampColors(blockColors);
	float weights[blockDim*blockDim];
	createWeights(weights, blockColors, weightAlpha());
	nv::Vector3 channelWeights = createChannelWeights(colorMask());

	icbc::Quality icbcQuality = icbc::Quality_Default;
	if ( quality() == Texture::Quality::Highest)
		icbcQuality = icbc::Quality_Max;
	icbc::compress_dxt1(icbcQuality, reinterpret_cast<const float*>(blockColors), weights,
		reinterpret_cast<const float*>(&channelWeights), true, quality() >= Texture::Quality::High,
		dxtBlock);
}

Bc1AConverter::Bc1AConverter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 8, quality)
{
}

void Bc1AConverter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	auto dxtBlock = reinterpret_cast<nv::BlockDXT1*>(block);
	nv::ColorBlock colorBlock = toColorBlock(blockColors);
	if (quality() == Texture::Quality::Lowest || quality() == Texture::Quality::Low)
	{
		nv::QuickCompress::compressDXT1a(colorBlock, dxtBlock);
		return;
	}

	// Same implementation as nv::CompressorDXT1a::compressBlock()
	std::uint32_t alphaMask = 0;
	for (unsigned int i = 0; i < S3tcConverter::blockDim; i++)
	{
		if (colorBlock.color(i).a == 0)
			alphaMask |= (3 << (i * 2)); // Set two bits for each color.
	}

	if (colorBlock.isSingleColor())
	{
		nv::OptimalCompress::compressDXT1a(colorBlock.color(0), alphaMask, dxtBlock);
		return;
	}

	nvsquish::WeightedClusterFit fit;
	setChannelWeights(fit, colorMask());

	int flags = nvsquish::kDxt1;
	if (weightAlpha())
		flags |= nvsquish::kWeightColourByAlpha;

	nvsquish::ColourSet colors(reinterpret_cast<const std::uint8_t*>(colorBlock.colors()), flags);
	fit.SetColourSet(&colors, nvsquish::kDxt1);
	fit.Compress(dxtBlock);
}

Bc2Converter::Bc2Converter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 16, quality)
{
}

void Bc2Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	nv::BlockDXT3* dxtBlock = reinterpret_cast<nv::BlockDXT3*>(block);
	nv::ColorBlock colorBlock = toColorBlock(blockColors);
	if (quality() == Texture::Quality::Lowest || quality() == Texture::Quality::Low)
	{
		nv::QuickCompress::compressDXT3(colorBlock, dxtBlock);
		return;
	}

	// Same implementation as nv::CompressorDXT3::compressBlock()

	// Compress explicit alpha.
	nv::OptimalCompress::compressDXT3A(colorBlock, &dxtBlock->alpha);

	// Compress color.
	if (colorBlock.isSingleColor())
	{
		nv::OptimalCompress::compressDXT1(colorBlock.color(0), &dxtBlock->color);
		return;
	}

	nvsquish::WeightedClusterFit fit;
	setChannelWeights(fit, colorMask());

	int flags = 0;
	if (weightAlpha())
		flags |= nvsquish::kWeightColourByAlpha;

	nvsquish::ColourSet colors(reinterpret_cast<const std::uint8_t*>(colorBlock.colors()), flags);
	fit.SetColourSet(&colors, 0);
	fit.Compress(&dxtBlock->color);
}

Bc3Converter::Bc3Converter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 16, quality)
{
}

void Bc3Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	nv::BlockDXT5* dxtBlock = reinterpret_cast<nv::BlockDXT5*>(block);
	nv::ColorBlock colorBlock = toColorBlock(blockColors);
	if (quality() == Texture::Quality::Lowest || quality() == Texture::Quality::Low)
	{
		nv::QuickCompress::compressDXT5(colorBlock, dxtBlock);
		return;
	}

	// Same implementation as nv::CompressorDXT5::compressBlock()

	// Compress explicit alpha.
	if (quality() == Texture::Quality::High || quality() == Texture::Quality::Highest)
		nv::OptimalCompress::compressDXT5A(colorBlock, &dxtBlock->alpha);
	else
		nv::QuickCompress::compressDXT5A(colorBlock, &dxtBlock->alpha);

	// Compress color.
	if (colorBlock.isSingleColor())
	{
		nv::OptimalCompress::compressDXT1(colorBlock.color(0), &dxtBlock->color);
		return;
	}

	nvsquish::WeightedClusterFit fit;
	setChannelWeights(fit, colorMask());

	int flags = 0;
	if (weightAlpha())
		flags |= nvsquish::kWeightColourByAlpha;

	nvsquish::ColourSet colors(reinterpret_cast<const std::uint8_t*>(colorBlock.colors()), flags);
	fit.SetColourSet(&colors, 0);
	fit.Compress(&dxtBlock->color);
}

Bc4Converter::Bc4Converter(const Texture& texture, const Image& image, Texture::Quality quality,
	bool keepSign)
	: S3tcConverter(texture, image, 8, quality)
	, m_signed(keepSign)
{
}

void Bc4Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	auto dxtBlock = reinterpret_cast<nv::BlockATI1*>(block);
	nv::AlphaBlock4x4 alphaBlock = toAlphaBlock(blockColors, 0, m_signed);
	if (quality() == Texture::Quality::Lowest || quality() == Texture::Quality::Low)
		nv::QuickCompress::compressDXT5A(alphaBlock, &dxtBlock->alpha);
	else
		nv::OptimalCompress::compressDXT5A(alphaBlock, &dxtBlock->alpha);
}

Bc5Converter::Bc5Converter(const Texture& texture, const Image& image, Texture::Quality quality,
	bool keepSign)
	: S3tcConverter(texture, image, 16, quality)
	, m_signed(keepSign)
{
}

void Bc5Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	auto dxtBlock = reinterpret_cast<nv::BlockATI2*>(block);
	nv::AlphaBlock4x4 xBlock = toAlphaBlock(blockColors, 0, m_signed);
	nv::AlphaBlock4x4 yBlock = toAlphaBlock(blockColors, 1, m_signed);
	if (quality() == Texture::Quality::Lowest || quality() == Texture::Quality::Low)
	{
		nv::QuickCompress::compressDXT5A(xBlock, &dxtBlock->x);
		nv::QuickCompress::compressDXT5A(yBlock, &dxtBlock->y);
	}
	else
	{
		nv::OptimalCompress::compressDXT5A(xBlock, &dxtBlock->x);
		nv::OptimalCompress::compressDXT5A(yBlock, &dxtBlock->y);
	}
}

std::mutex Bc6HConverter::m_mutex;

Bc6HConverter::Bc6HConverter(const Texture& texture, const Image& image, Texture::Quality quality,
	bool keepSign)
	: S3tcConverter(texture, image, 16, quality)
	, m_signed(keepSign)
{
	// FROMAT is a global variable, so need to make sure no Bc6Converter instance is used
	// concurrently.
	m_mutex.lock();
	ZOH::Utils::FORMAT = m_signed ? ZOH::SIGNED_F16 : ZOH::UNSIGNED_F16;
}

Bc6HConverter::~Bc6HConverter()
{
	m_mutex.unlock();
}

void Bc6HConverter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	float weights[blockDim*blockDim];
	createWeights(weights, blockColors, weightAlpha());

	ZOH::Tile zohTile(4, 4);
	for (unsigned int y = 0; y < blockDim; ++y)
	{
		for (unsigned int x = 0; x < blockDim; ++x)
		{
			std::uint16_t rHalf = nv::to_half(blockColors[y*blockDim + x].r);
			std::uint16_t gHalf = nv::to_half(blockColors[y*blockDim + x].g);
			std::uint16_t bHalf = nv::to_half(blockColors[y*blockDim + x].b);
			zohTile.data[y][x].x = ZOH::Tile::half2float(rHalf);
			zohTile.data[y][x].y = ZOH::Tile::half2float(gHalf);
			zohTile.data[y][x].z = ZOH::Tile::half2float(bHalf);
			zohTile.importance_map[y][x] = weights[blockDim*y + x];
		}
	}

	ZOH::compress(zohTile, reinterpret_cast<char*>(block));
}


Bc7Converter::Bc7Converter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 16, quality)
{
	bc7enc_compress_block_init();
}

void Bc7Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	// Take advantage of full colorblock support of nvidia-texture-tools for highest quality only
	// since it's very slow.
	bool useBc7Enc = quality() < Texture::Quality::Highest;

	if (useBc7Enc)
	{
		std::uint8_t colorBlock[16][4];
		toBc7ColorBlock(colorBlock, blockColors);

		// NOTE: would be slightly more optimal to create this ahead of time, but can't forward
		// declare the type due to how it's declared in bc7enc. The overhead of this is expected
		// to be very small.
		bc7enc_compress_block_params params = createBc7BlockParams(*this);
		bc7enc_compress_block(block, &colorBlock, &params);
	}
	else
	{
		float weights[blockDim*blockDim];
		createWeights(weights, blockColors, weightAlpha());

		AVPCL::Tile avpclTile(4, 4);
		for (uint y = 0; y < 4; ++y)
		{
			for (uint x = 0; x < 4; ++x)
			{
				nv::Vector4 color =
					reinterpret_cast<const nv::Vector4*>(blockColors)[blockDim*y + x];
				avpclTile.data[y][x] = color*255.0f;
				avpclTile.importance_map[y][x] = weights[blockDim*y + x];
			}
		}

		AVPCL::compress(avpclTile, reinterpret_cast<char*>(block));
	}
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_S3TC
