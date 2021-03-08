/*
 * Copyright 2017-2021 Aaron Barany
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

#include <cassert>

#if CUTTLEFISH_HAS_S3TC

#if CUTTLEFISH_CLANG || CUTTLEFISH_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#if CUTTLEFISH_ISPC
#include "bc7e_ispc.h"
#include "ispc_texcomp.h"
#else
#include "bc7enc.h"
#endif
#include "cmp_core.h"
#include "rgbcx.h"
#include "squish.h"
#include <glm/gtc/packing.hpp>

#if CUTTLEFISH_CLANG || CUTTLEFISH_GCC
#pragma GCC diagnostic pop
#endif

namespace cuttlefish
{

static bool initializeRgbcxImpl()
{
	rgbcx::init();
	return true;
}

static void initializeRgbcx()
{
	static bool initialized = initializeRgbcxImpl();
	CUTTLEFISH_UNUSED(initialized);
}

static uint32_t getRgbcxQualityLevel(Texture::Quality quality)
{
	auto qualityCount = static_cast<uint32_t>(Texture::Quality::Highest);
	auto qualityValue = static_cast<uint32_t>(quality);
	return rgbcx::MIN_LEVEL + (rgbcx::MAX_LEVEL - rgbcx::MIN_LEVEL)*qualityValue/qualityCount;
}

static float getCompressonatorQualityLevel(Texture::Quality quality)
{
	auto qualityCount = static_cast<float>(Texture::Quality::Highest);
	auto qualityValue = static_cast<float>(quality);
	return qualityValue/qualityCount;
}

static uint32_t getSearchRadius(Texture::Quality quality)
{
	switch (quality)
	{
		case Texture::Quality::Lowest:
		case Texture::Quality::Low:
			return 3;
		case Texture::Quality::Normal:
			return 5;
		case Texture::Quality::High:
			return 16;
		case Texture::Quality::Highest:
			return 32;
	}
	return 0;
}

static void toColorBlock(std::uint8_t outBlock[S3tcConverter::blockPixels][4],
	const ColorRGBAf* blockColors)
{
	for (unsigned int i = 0; i < S3tcConverter::blockPixels; ++i)
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

static void packBc2Alpha(std::uint8_t outAlpha[8], std::uint8_t colorBlock[16][4])
{
	const float alphaScale = 15.0f/255.0f;
	for (unsigned int i = 0; i < S3tcConverter::blockPixels/2; ++i)
	{
		std::uint8_t alpha0 = colorBlock[i*2][3];
		std::uint8_t alpha1 = colorBlock[i*2 + 1][3];

		auto compressedAlpha0 = static_cast<std::uint8_t>(std::round(alpha0*alphaScale));
		auto compressedAlpha1 = static_cast<std::uint8_t>(std::round(alpha1*alphaScale));
		outAlpha[i] = static_cast<std::uint8_t>(compressedAlpha0 | (compressedAlpha1 << 4));
	}
}

#if !CUTTLEFISH_ISPC
static bc7enc_compress_block_params createBc7BlockParams(const S3tcConverter& converter)
{
	bc7enc_compress_block_params params;

	switch (converter.quality())
	{
		case Texture::Quality::Lowest:
			params.m_max_partitions = 0;
			params.m_uber_level = 0;
			params.m_try_least_squares = false;
			params.m_mode17_partition_estimation_filterbank = true;
			bc7enc_compress_block_params_init_linear_weights(&params);
			break;
		case Texture::Quality::Low:
			params.m_max_partitions = 16;
			params.m_uber_level = 0;
			params.m_try_least_squares = true;
			params.m_mode17_partition_estimation_filterbank = true;
			bc7enc_compress_block_params_init_linear_weights(&params);
			break;
		case Texture::Quality::Normal:
			params.m_max_partitions = BC7ENC_MAX_PARTITIONS;
			params.m_uber_level = 1;
			params.m_try_least_squares = true;
			params.m_mode17_partition_estimation_filterbank = false;
			if (converter.image().colorSpace() == ColorSpace::sRGB)
				bc7enc_compress_block_params_init_perceptual_weights(&params);
			else
				bc7enc_compress_block_params_init_linear_weights(&params);
			break;
		case Texture::Quality::High:
		case Texture::Quality::Highest:
			params.m_max_partitions = BC7ENC_MAX_PARTITIONS;
			params.m_uber_level = 4;
			params.m_try_least_squares = true;
			params.m_mode17_partition_estimation_filterbank = false;
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
#endif

S3tcConverter::S3tcConverter(const Texture& texture, const Image& image, unsigned int blockSize,
	Texture::Quality quality)
	: Converter(image), m_blockSize(blockSize),
	m_jobsX((image.width() + blockDim - 1)/blockDim),
	m_jobsY((image.height() + blockDim - 1)/blockDim), m_colorSpace(image.colorSpace()),
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
	: S3tcConverter(texture, image, 8, quality), m_qualityLevel(getRgbcxQualityLevel(quality))
{
	initializeRgbcx();
}

void Bc1Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	std::uint8_t colorBlock[blockPixels][4];
	toColorBlock(colorBlock, blockColors);
	// Fully utilize 3-color mode since alpha channel will be ignored.
	rgbcx::encode_bc1(m_qualityLevel, block, reinterpret_cast<std::uint8_t*>(colorBlock), true,
		true, nullptr);
}

Bc1AConverter::Bc1AConverter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 8, quality), m_squishFlags(squish::kDxt1),
	m_qualityLevel(getRgbcxQualityLevel(quality))
{
	if (quality <= Texture::Quality::Low)
		m_squishFlags |= squish::kColourRangeFit;
	else if (quality == Texture::Quality::Highest)
		m_squishFlags |= squish::kColourIterativeClusterFit;
	initializeRgbcx();
}

void Bc1AConverter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	bool hasAlpha = false;
	for (unsigned int i = 0; i < blockPixels; ++i)
	{
		if (blockColors[i].a < 0.5f)
			hasAlpha = true;
	}

	if (hasAlpha)
	{
		float weights[3];
		bool srgb = colorSpace() == ColorSpace::sRGB;
		if (colorMask().r)
		{
			if (srgb)
				weights[0] = 0.2126f;
			else
				weights[0] = 1.0f;
		}
		else
			weights[0] = 0.0f;

		if (colorMask().g)
		{
			if (srgb)
				weights[1] = 0.7152f;
			else
				weights[1] = 1.0f;
		}
		else
			weights[1] = 0.0f;

		if (colorMask().b)
		{
			if (srgb)
				weights[2] = 0.0722f;
			else
				weights[2] = 1.0f;
		}
		else
			weights[2] = 0.0f;

		squish::Compress(reinterpret_cast<float*>(blockColors), block, m_squishFlags, weights);
	}
	else
	{
		std::uint8_t colorBlock[blockPixels][4];
		toColorBlock(colorBlock, blockColors);

		// Allow 3-color mode, but can't use black for 3-color since it will be treated as
		// transparent.
		rgbcx::encode_bc1(m_qualityLevel, block, reinterpret_cast<std::uint8_t*>(colorBlock), true,
			false, nullptr);
	}
}

Bc2Converter::Bc2Converter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 16, quality), m_qualityLevel(getRgbcxQualityLevel(quality))
{
	initializeRgbcx();
}

void Bc2Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	std::uint8_t colorBlock[16][4];
	toColorBlock(colorBlock, blockColors);
	auto compressedAlphaBlock = reinterpret_cast<std::uint8_t*>(block);
	std::uint8_t* compressedColorBlock = compressedAlphaBlock + 8;
	packBc2Alpha(compressedAlphaBlock, colorBlock);
	// NOTE: BC2 only supports 4 color mode, so disable all 3 color mode options.
	rgbcx::encode_bc1(m_qualityLevel, compressedColorBlock,
		reinterpret_cast<std::uint8_t*>(colorBlock), false, false, nullptr);
}

Bc3Converter::Bc3Converter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 16, quality), m_qualityLevel(getRgbcxQualityLevel(quality)),
	m_searchRadius(getSearchRadius(quality))
{
	initializeRgbcx();
}

void Bc3Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	std::uint8_t colorBlock[blockPixels][4];
	toColorBlock(colorBlock, blockColors);
	if (quality() <= Texture::Quality::Low)
		rgbcx::encode_bc3(m_qualityLevel, block, reinterpret_cast<std::uint8_t*>(colorBlock));
	else
	{
		rgbcx::encode_bc3_hq(m_qualityLevel, block, reinterpret_cast<std::uint8_t*>(colorBlock),
			m_searchRadius);
	}
}

Bc4Converter::Bc4Converter(const Texture& texture, const Image& image, Texture::Quality quality,
	bool keepSign)
	: S3tcConverter(texture, image, 8, quality), m_signed(keepSign),
	m_searchRadius(getSearchRadius(quality)), m_compressonatorOptions(nullptr)
{
	if (m_signed)
	{
		// rgbcx doesn't support signed BC4 so use Compressonator instead.
		CreateOptionsBC4(&m_compressonatorOptions);
		assert(m_compressonatorOptions);
		SetQualityBC4(m_compressonatorOptions, getCompressonatorQualityLevel(quality));
	}
	else
		initializeRgbcx();
}

Bc4Converter::~Bc4Converter()
{
	if (m_compressonatorOptions)
		DestroyOptionsBC4(m_compressonatorOptions);
}

void Bc4Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	if (m_signed)
	{
		std::uint8_t colorBlock[blockPixels];
		for (unsigned int i = 0; i < blockPixels; ++i)
		{
			colorBlock[i] =
				static_cast<std::uint8_t>(std::round(clamp(blockColors[i].r, -1.0f, 1.0f)*0x7F));
		}

		assert(m_compressonatorOptions);
		CompressBlockBC4S(reinterpret_cast<const char*>(colorBlock), blockDim,
			reinterpret_cast<std::uint8_t*>(block), m_compressonatorOptions);
	}
	else
	{
		std::uint8_t colorBlock[blockPixels];
		for (unsigned int i = 0; i < blockPixels; ++i)
		{
			colorBlock[i] =
				static_cast<std::uint8_t>(std::round(clamp(blockColors[i].r, 0.0f, 1.0f)*0xFF));
		}

		if (quality() <= Texture::Quality::Low)
			rgbcx::encode_bc4(block, colorBlock, 1);
		else
			rgbcx::encode_bc4_hq(block, colorBlock, 1, m_searchRadius);
	}
}

Bc5Converter::Bc5Converter(const Texture& texture, const Image& image, Texture::Quality quality,
	bool keepSign)
	: S3tcConverter(texture, image, 16, quality), m_signed(keepSign),
	m_searchRadius(getSearchRadius(quality)), m_compressonatorOptions(nullptr)
{
	if (m_signed)
	{
		// rgbcx doesn't support signed BC4 so use Compressonator instead.
		CreateOptionsBC5(&m_compressonatorOptions);
		assert(m_compressonatorOptions);
		SetQualityBC5(m_compressonatorOptions, getCompressonatorQualityLevel(quality));
	}
	else
		initializeRgbcx();
}

Bc5Converter::~Bc5Converter()
{
	if (m_compressonatorOptions)
		DestroyOptionsBC5(m_compressonatorOptions);
}

void Bc5Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	if (m_signed)
	{
		std::uint8_t colorBlock[2][blockPixels];
		for (unsigned int i = 0; i < blockPixels; ++i)
		{
			colorBlock[0][i] =
				static_cast<std::uint8_t>(std::round(clamp(blockColors[i].r, -1.0f, 1.0f)*0x7F));
			colorBlock[1][i] =
				static_cast<std::uint8_t>(std::round(clamp(blockColors[i].g, -1.0f, 1.0f)*0x7F));
		}

		assert(m_compressonatorOptions);
		CompressBlockBC5S(reinterpret_cast<const char*>(&colorBlock[0]), blockDim,
			reinterpret_cast<const char*>(&colorBlock[1]), blockDim,
			reinterpret_cast<std::uint8_t*>(block), m_compressonatorOptions);
	}
	else
	{
		std::uint8_t colorBlock[blockPixels][2];
		for (unsigned int i = 0; i < blockPixels; ++i)
		{
			colorBlock[i][0] =
				static_cast<std::uint8_t>(std::round(clamp(blockColors[i].r, 0.0f, 1.0f)*0xFF));
			colorBlock[i][1] =
				static_cast<std::uint8_t>(std::round(clamp(blockColors[i].g, 0.0f, 1.0f)*0xFF));
		}

		if (quality() <= Texture::Quality::Low)
			rgbcx::encode_bc5(block, reinterpret_cast<uint8_t*>(colorBlock), 0, 1, 2);
		else
		{
			rgbcx::encode_bc5_hq(block, reinterpret_cast<uint8_t*>(colorBlock), 0, 1, 2,
				m_searchRadius);
		}
	}
}

Bc6HConverter::Bc6HConverter(const Texture& texture, const Image& image, Texture::Quality quality,
	bool keepSign)
	: S3tcConverter(texture, image, 16, quality), m_compressonatorOptions(nullptr)
{
	bool useCompressonator = true;
#if CUTTLEFISH_ISPC
	m_ispcTexcompSettings = nullptr;
	if (!keepSign)
	{
		// NOTE: ispc_texcomp only supports unsigned BC6H.
		useCompressonator = false;
		m_ispcTexcompSettings = new bc6h_enc_settings;
		switch (quality)
		{
			case Texture::Quality::Lowest:
				GetProfile_bc6h_veryfast(m_ispcTexcompSettings);
				break;
			case Texture::Quality::Low:
				GetProfile_bc6h_fast(m_ispcTexcompSettings);
				break;
			case Texture::Quality::Normal:
				GetProfile_bc6h_basic(m_ispcTexcompSettings);
				break;
			case Texture::Quality::High:
				GetProfile_bc6h_slow(m_ispcTexcompSettings);
				break;
			case Texture::Quality::Highest:
				GetProfile_bc6h_veryslow(m_ispcTexcompSettings);
				break;
			default:
				assert(false);
				break;
		}
	}
#endif

	if (useCompressonator)
	{
		CreateOptionsBC6(&m_compressonatorOptions);
		assert(m_compressonatorOptions);
		SetQualityBC6(m_compressonatorOptions, getCompressonatorQualityLevel(quality));
		SetSignedBC6(m_compressonatorOptions, keepSign);
	}
}

Bc6HConverter::~Bc6HConverter()
{
#if CUTTLEFISH_ISPC
	delete m_ispcTexcompSettings;
#endif
	if (m_compressonatorOptions)
		DestroyOptionsBC6(m_compressonatorOptions);
}

void Bc6HConverter::compressBlock(void* block, ColorRGBAf* blockColors)
{
#if CUTTLEFISH_ISPC
	if (m_ispcTexcompSettings)
	{
		std::uint16_t colorBlock[blockPixels][4];
		for (unsigned int i = 0; i < blockPixels; ++i)
		{
			for (unsigned int j = 0; j < 4; ++j)
			{
				colorBlock[i][j] =
					glm::packHalf(glm::vec1(reinterpret_cast<float*>(blockColors + i)[j])).x;
			}
		}

		rgba_surface surface = {reinterpret_cast<std::uint8_t*>(colorBlock), blockDim, blockDim,
			static_cast<std::int32_t>(sizeof(std::uint16_t)*4*blockDim)};
		CompressBlocksBC6H(&surface, reinterpret_cast<std::uint8_t*>(block), m_ispcTexcompSettings);
		return;
	}
#endif

	assert(m_compressonatorOptions);
	std::uint16_t colorBlock[blockPixels][3];
	for (unsigned int i = 0; i < blockPixels; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			colorBlock[i][j] =
				glm::packHalf(glm::vec1(reinterpret_cast<float*>(blockColors + i)[j])).x;
		}
	}

	CompressBlockBC6(reinterpret_cast<std::uint16_t*>(colorBlock), 3*blockDim,
		reinterpret_cast<std::uint8_t*>(block), m_compressonatorOptions);
}


Bc7Converter::Bc7Converter(const Texture& texture, const Image& image, Texture::Quality quality)
	: S3tcConverter(texture, image, 16, quality), m_params(nullptr)
{
#if CUTTLEFISH_ISPC
	ispc::bc7e_compress_block_init();
	m_params = new ispc::bc7e_compress_block_params;
	bool perceptual = image.colorSpace() == ColorSpace::sRGB;
	switch (quality)
	{
		case Texture::Quality::Lowest:
			ispc::bc7e_compress_block_params_init_ultrafast(m_params, perceptual);
			break;
		case Texture::Quality::Low:
			ispc::bc7e_compress_block_params_init_fast(m_params, perceptual);
			break;
		case Texture::Quality::Normal:
			ispc::bc7e_compress_block_params_init_basic(m_params, perceptual);
			break;
		case Texture::Quality::High:
			ispc::bc7e_compress_block_params_init_slow(m_params, perceptual);
			break;
		case Texture::Quality::Highest:
			ispc::bc7e_compress_block_params_init_slowest(m_params, perceptual);
			break;
		default:
			assert(false);
			break;
	}
#else
	bc7enc_compress_block_init();
	m_params = new bc7enc_compress_block_params(createBc7BlockParams(*this));
#endif
}

Bc7Converter::~Bc7Converter()
{
	delete m_params;
}

void Bc7Converter::compressBlock(void* block, ColorRGBAf* blockColors)
{
	std::uint8_t colorBlock[blockPixels][4];
	toColorBlock(colorBlock, blockColors);

#if CUTTLEFISH_ISPC
	ispc::bc7e_compress_blocks(1, reinterpret_cast<std::uint64_t*>(block),
		reinterpret_cast<std::uint32_t*>(colorBlock), m_params);
#else
	// NOTE: would be slightly more optimal to create this ahead of time, but can't forward
	// declare the type due to how it's declared in bc7enc. The overhead of this is expected
	// to be very small.
	bc7enc_compress_block(block, &colorBlock, m_params);
#endif
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_S3TC
