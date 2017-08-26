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

#include <cuttlefish/Texture.h>

#include "Converter.h"
#include <cuttlefish/Color.h>
#include <algorithm>
#include <cmath>
#include <thread>
#include <vector>

#if CUTTLEFISH_MSC
#include <intrin.h>
#endif

namespace cuttlefish
{

static const auto formatCount = static_cast<unsigned int>(Texture::Format::PVRTC2_RGBA_4BPP) + 1;
static const auto typeCount = static_cast<unsigned int>(Texture::Type::Float) + 1;
static const Image emptyImage;

#if CUTTLEFISH_HAS_S3TC
static const bool hasS3tc = true;
#else
static const bool hasS3tc = false;
#endif

#if CUTTLEFISH_HAS_ETC
static const bool hasEtc = true;
#else
static const bool hasEtc = false;
#endif

#if CUTTLEFISH_HAS_ASTC
static const bool hasAstc = true;
#else
static const bool hasAstc = false;
#endif

#if CUTTLEFISH_HAS_PVRTC
static const bool hasPvrtc = true;
#else
static const bool hasPvrtc = false;
#endif

static inline std::uint32_t clz(std::uint32_t x)
{
#if CUTTLEFISH_MSC
	if (!x)
		return 32;

	unsigned long leading = 0;
	_BitScanReverse( &leading, x);
	return 31 - leading;
#elif CUTTLEFISH_GCC || CUTTLEFISH_CLANG
	return x ? __builtin_clz(x) : 32;
#else
#error Need to implement clz for current compiler.
#endif
}

static float lerp(float t, float a, float b)
{
	return a + (b - a)*t;
}

using FaceImageList = std::vector<Image>;
using DepthImageList = std::vector<FaceImageList>;
using MipImageList = std::vector<DepthImageList>;

using TextureData = std::vector<std::uint8_t>;
using FaceTextureList = std::vector<TextureData>;
using DepthTextureList = std::vector<FaceTextureList>;
using MipTextureList = std::vector<DepthTextureList>;

struct Texture::Impl
{
	Dimension dimension;
	unsigned int width;
	unsigned int height;
	unsigned int depth;
	unsigned int mipLevels;
	unsigned int faces;
	MipImageList images;

	Format format = Format::Unknown;
	Type type = Type::UNorm;
	Color colorSpace = Color::Linear;
	Alpha alphaType = Alpha::Standard;
	MipTextureList textures;
};

bool Texture::isFormatValid(Format format, Type type)
{
	static const bool valid[formatCount][typeCount] =
	{
		// UNorm, SNorm, UInt, Int, UFloat, Float
		{false, false, false, false, false, false}, // Unknown

		// Standard formats
		{true, false, false, false, false, false}, // R4G4
		{true, false, false, false, false, false}, // R4G4B4A4
		{true, false, false, false, false, false}, // B4G4R4A4
		{true, false, false, false, false, false}, // R5G6B5
		{true, false, false, false, false, false}, // B5G6R5
		{true, false, false, false, false, false}, // R5G5B5A1
		{true, false, false, false, false, false}, // B5G5R5A1
		{true, false, false, false, false, false}, // A1R5G5B5
		{true, true, true, true, false, false},    // R8
		{true, true, true, true, false, false},    // R8G8
		{true, true, true, true, false, false},    // R8G8B8
		{true, false, false, false, false, false}, // B8G8R8
		{true, false, false, false, false, false}, // R8G8B8A8
		{true, false, false, false, false, false}, // B8G8R8A8
		{true, false, false, false, false, false}, // A8B8G8R8
		{true, false, true, false, false, false},  // A2R10G10B10
		{true, false, true, false, false, false},  // A2B10G10R10
		{true, true, true, true, false, true},     // R16
		{true, true, true, true, false, true},     // R16G16
		{true, true, true, true, false, true},     // R16G16B16
		{true, true, true, true, false, true},     // R16G16B16A16
		{true, true, true, true, false, true},     // R32
		{true, true, true, true, false, true},     // R32G32
		{true, true, true, true, false, true},     // R32G32B32
		{true, true, true, true, false, true},     // R32G32B32A32

		// Special formats.
		{false, false, false, false, true, false}, // B10G11R11_UFloat
		{false, false, false, false, true, false}, // E5B9G9R9_UFloat

		// Compressed formats.
		{hasS3tc, false, false, false, false, false},   // BC1_RGB
		{hasS3tc, false, false, false, false, false},   // BC1_RGBA
		{hasS3tc, false, false, false, false, false},   // BC2
		{hasS3tc, false, false, false, false, false},   // BC3
		{hasS3tc, hasS3tc, false, false, false, false}, // BC4
		{hasS3tc, hasS3tc, false, false, false, false}, // BC5
		{false, false, false, false, hasS3tc, hasS3tc}, // BC6H
		{hasS3tc, false, false, false, false, false},   // BC7
		{hasEtc, false, false, false, false, false},    // ETC1
		{hasEtc, false, false, false, false, false},    // ETC2_R8G8B8
		{hasEtc, false, false, false, false, false},    // ETC2_R8G8B8A1
		{hasEtc, false, false, false, false, false},    // ETC2_R8G8B8A8
		{hasEtc, hasEtc, false, false, false, false},   // EAC_R11
		{hasEtc, hasEtc, false, false, false, false},   // EAC_R11G11
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_4x4
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_5x4
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_5x5
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_6x5
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_6x6
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_8x5
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_8x6
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_8x8
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_10x5
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_10x6
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_10x8
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_10x10
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_12x10
		{hasAstc, false, false, false, hasAstc, false}, // ASTC_12x12
		{hasPvrtc, false, false, false, false, false},  // PVRTC1_RGB_2BPP
		{hasPvrtc, false, false, false, false, false},  // PVRTC1_RGBA_2BPP
		{hasPvrtc, false, false, false, false, false},  // PVRTC1_RGB_4BPP
		{hasPvrtc, false, false, false, false, false},  // PVRTC1_RGBA_4BPP
		{hasPvrtc, false, false, false, false, false},  // PVRTC2_RGBA_2BPP
		{hasPvrtc, false, false, false, false, false},  // PVRTC2_RGBA_4BPP
	};

	if (static_cast<unsigned int>(format) >= formatCount ||
		static_cast<unsigned int>(type) >= typeCount)
	{
		return false;
	}

	return valid[static_cast<unsigned int>(format)][static_cast<unsigned int>(type)];
}

bool Texture::hasNativeSRGB(Format format)
{
	switch (format)
	{
		case Format::R8G8B8:
		case Format::R8G8B8A8:
		case Format::B8G8R8A8:
		case Format::BC1_RGB:
		case Format::BC1_RGBA:
		case Format::BC2:
		case Format::BC3:
		case Format::BC7:
		case Format::ETC2_R8G8B8:
		case Format::ETC2_R8G8B8A1:
		case Format::ETC2_R8G8B8A8:
		case Format::ASTC_4x4:
		case Format::ASTC_5x4:
		case Format::ASTC_5x5:
		case Format::ASTC_6x5:
		case Format::ASTC_6x6:
		case Format::ASTC_8x5:
		case Format::ASTC_8x6:
		case Format::ASTC_8x8:
		case Format::ASTC_10x5:
		case Format::ASTC_10x6:
		case Format::ASTC_10x8:
		case Format::ASTC_10x10:
		case Format::ASTC_12x10:
		case Format::ASTC_12x12:
		case Format::PVRTC1_RGB_2BPP:
		case Format::PVRTC1_RGBA_2BPP:
		case Format::PVRTC1_RGB_4BPP:
		case Format::PVRTC1_RGBA_4BPP:
		case Format::PVRTC2_RGBA_2BPP:
		case Format::PVRTC2_RGBA_4BPP:
			return true;

		default:
			return false;
	}
}

unsigned int Texture::maxMipmapLevels(Dimension dimension, unsigned int width,
	unsigned int height, unsigned int depth)
{
	unsigned int levelCountWidth = 32 - clz(width);
	unsigned int levelCountHeight = 32 - clz(height);
	unsigned int maxWidthHeight = std::max(levelCountWidth, levelCountHeight);
	if (dimension == Dimension::Dim3D)
	{
		unsigned int levelCountDepth = 32 - clz(depth);
		return std::max(maxWidthHeight, levelCountDepth);
	}
	else
		return maxWidthHeight;
}

unsigned int Texture::blockWidth(Format format)
{
	static unsigned int sizes[] =
	{
		0,  // Unknown

		// Standard formats
		1, // R4G4
		1, // R4G4B4A4
		1, // B4G4R4A4
		1, // R5G6B5
		1, // B5G6R5
		1, // R5G5B5A1
		1, // B5G5R5A1
		1, // A1R5G5B5
		1, // R8
		1, // R8G8
		1, // R8G8B8
		1, // B8G8R8
		1, // R8G8B8A8
		1, // B8G8R8A8
		1, // A8B8G8R8
		1, // A2R10G10B10
		1, // A2B10G10R10
		1, // R16
		1, // R16G16
		1, // R16G16B16
		1, // R16G16B16A16
		1, // R32
		1, // R32G32
		1, // R32G32B32
		1, // R32G32B32A32

		// Special formats.
		1, // B10G11R11_UFloat
		1, // E5B9G9R9_UFloat

		// Compressed formats
		4,  // BC1_RGB
		4,  // BC1_RGBA
		4,  // BC2
		4,  // BC3
		4,  // BC4
		4,  // BC5
		4,  // BC6H
		4,  // BC7
		4,  // ETC1
		4,  // ETC2_R8G8B8
		4,  // ETC2_R8G8B8A1
		4,  // ETC2_R8G8B8A8
		4,  // EAC_R11
		4,  // EAC_R11G11
		4,  // ASTC_4x4
		5,  // ASTC_5x4
		5,  // ASTC_5x5
		6,  // ASTC_6x5
		6,  // ASTC_6x6
		8,  // ASTC_8x5
		8,  // ASTC_8x6
		8,  // ASTC_8x8
		10, // ASTC_10x5
		10, // ASTC_10x6
		10, // ASTC_10x8
		10, // ASTC_10x10
		12, // ASTC_12x10
		12, // ASTC_12x12
		8,  // PVRTC1_RGB_2BPP
		8,  // PVRTC1_RGBA_2BPP
		4,  // PVRTC1_RGB_4BPP
		4,  // PVRTC1_RGBA_4BPP
		8,  // PVRTC2_RGBA_2BPP
		4,  // PVRTC2_RGBA_4BPP
	};
	static_assert(sizeof(sizes)/sizeof(*sizes) == formatCount, "Enum array mimatch");

	if (static_cast<unsigned int>(format) >= formatCount)
		return 0;

	return sizes[static_cast<unsigned int>(format)];
}

unsigned int Texture::blockHeight(Format format)
{
	static unsigned int sizes[] =
	{
		0,  // Unknown

		// Standard formats
		1, // R4G4
		1, // R4G4B4A4
		1, // B4G4R4A4
		1, // R5G6B5
		1, // B5G6R5
		1, // R5G5B5A1
		1, // B5G5R5A1
		1, // A1R5G5B5
		1, // R8
		1, // R8G8
		1, // R8G8B8
		1, // B8G8R8
		1, // R8G8B8A8
		1, // B8G8R8A8
		1, // A8B8G8R8
		1, // A2R10G10B10
		1, // A2B10G10R10
		1, // R16
		1, // R16G16
		1, // R16G16B16
		1, // R16G16B16A16
		1, // R32
		1, // R32G32
		1, // R32G32B32
		1, // R32G32B32A32

		// Special formats.
		1, // B10G11R11_UFloat
		1, // E5B9G9R9_UFloat

		// Compressed formats
		4,  // BC1_RGB
		4,  // BC1_RGBA
		4,  // BC2
		4,  // BC3
		4,  // BC4
		4,  // BC5
		4,  // BC6H
		4,  // BC7
		4,  // ETC1
		4,  // ETC2_R8G8B8
		4,  // ETC2_R8G8B8A1
		4,  // ETC2_R8G8B8A8
		4,  // EAC_R11
		4,  // EAC_R11G11
		4,  // ASTC_4x4
		4,  // ASTC_5x4
		5,  // ASTC_5x5
		5,  // ASTC_6x5
		6,  // ASTC_6x6
		5,  // ASTC_8x5
		6,  // ASTC_8x6
		8,  // ASTC_8x8
		5,  // ASTC_10x5
		6,  // ASTC_10x6
		8,  // ASTC_10x8
		10, // ASTC_10x10
		10, // ASTC_12x10
		12, // ASTC_12x12
		4,  // PVRTC1_RGB_2BPP
		4,  // PVRTC1_RGBA_2BPP
		4,  // PVRTC1_RGB_4BPP
		4,  // PVRTC1_RGBA_4BPP
		4,  // PVRTC2_RGBA_2BPP
		4,  // PVRTC2_RGBA_4BPP
	};
	static_assert(sizeof(sizes)/sizeof(*sizes) == formatCount, "Enum array mimatch");

	if (static_cast<unsigned int>(format) >= formatCount)
		return 0;

	return sizes[static_cast<unsigned int>(format)];
}

unsigned int Texture::minWidth(Format format)
{
	static unsigned int sizes[] =
	{
		0,  // Unknown

		// Standard formats
		1, // R4G4
		1, // R4G4B4A4
		1, // B4G4R4A4
		1, // R5G6B5
		1, // B5G6R5
		1, // R5G5B5A1
		1, // B5G5R5A1
		1, // A1R5G5B5
		1, // R8
		1, // R8G8
		1, // R8G8B8
		1, // B8G8R8
		1, // R8G8B8A8
		1, // B8G8R8A8
		1, // A8B8G8R8
		1, // A2R10G10B10
		1, // A2B10G10R10
		1, // R16
		1, // R16G16
		1, // R16G16B16
		1, // R16G16B16A16
		1, // R32
		1, // R32G32
		1, // R32G32B32
		1, // R32G32B32A32

		// Special formats.
		1, // B10G11R11_UFloat
		1, // E5B9G9R9_UFloat

		// Compressed formats
		4,  // BC1_RGB
		4,  // BC1_RGBA
		4,  // BC2
		4,  // BC3
		4,  // BC4
		4,  // BC5
		4,  // BC6H
		4,  // BC7
		4,  // ETC1
		4,  // ETC2_R8G8B8
		4,  // ETC2_R8G8B8A1
		4,  // ETC2_R8G8B8A8
		4,  // EAC_R11
		4,  // EAC_R11G11
		4,  // ASTC_4x4
		5,  // ASTC_5x4
		5,  // ASTC_5x5
		6,  // ASTC_6x5
		6,  // ASTC_6x6
		8,  // ASTC_8x5
		8,  // ASTC_8x6
		8,  // ASTC_8x8
		10, // ASTC_10x5
		10, // ASTC_10x6
		10, // ASTC_10x8
		10, // ASTC_10x10
		12, // ASTC_12x10
		12, // ASTC_12x12
		16, // PVRTC1_RGB_2BPP
		16, // PVRTC1_RGBA_2BPP
		8,  // PVRTC1_RGB_4BPP
		8,  // PVRTC1_RGBA_4BPP
		16, // PVRTC2_RGBA_2BPP
		8,  // PVRTC2_RGBA_4BPP
	};
	static_assert(sizeof(sizes)/sizeof(*sizes) == formatCount, "Enum array mimatch");

	if (static_cast<unsigned int>(format) >= formatCount)
		return 0;

	return sizes[static_cast<unsigned int>(format)];
}

unsigned int Texture::minHeight(Format format)
{
	static unsigned int sizes[] =
	{
		0,  // Unknown

		// Standard formats
		1, // R4G4
		1, // R4G4B4A4
		1, // B4G4R4A4
		1, // R5G6B5
		1, // B5G6R5
		1, // R5G5B5A1
		1, // B5G5R5A1
		1, // A1R5G5B5
		1, // R8
		1, // R8G8
		1, // R8G8B8
		1, // B8G8R8
		1, // R8G8B8A8
		1, // B8G8R8A8
		1, // A8B8G8R8
		1, // A2R10G10B10
		1, // A2B10G10R10
		1, // R16
		1, // R16G16
		1, // R16G16B16
		1, // R16G16B16A16
		1, // R32
		1, // R32G32
		1, // R32G32B32
		1, // R32G32B32A32

		// Special formats.
		1, // B10G11R11_UFloat
		1, // E5B9G9R9_UFloat

		// Compressed formats
		4,  // BC1_RGB
		4,  // BC1_RGBA
		4,  // BC2
		4,  // BC3
		4,  // BC4
		4,  // BC5
		4,  // BC6H
		4,  // BC7
		4,  // ETC1
		4,  // ETC2_R8G8B8
		4,  // ETC2_R8G8B8A1
		4,  // ETC2_R8G8B8A8
		4,  // EAC_R11
		4,  // EAC_R11G11
		4,  // ASTC_4x4
		4,  // ASTC_5x4
		5,  // ASTC_5x5
		5,  // ASTC_6x5
		6,  // ASTC_6x6
		5,  // ASTC_8x5
		6,  // ASTC_8x6
		8,  // ASTC_8x8
		5,  // ASTC_10x5
		6,  // ASTC_10x6
		8,  // ASTC_10x8
		10, // ASTC_10x10
		10, // ASTC_12x10
		12, // ASTC_12x12
		8,  // PVRTC1_RGB_2BPP
		8,  // PVRTC1_RGBA_2BPP
		8,  // PVRTC1_RGB_4BPP
		8,  // PVRTC1_RGBA_4BPP
		8,  // PVRTC2_RGBA_2BPP
		8,  // PVRTC2_RGBA_4BPP
	};
	static_assert(sizeof(sizes)/sizeof(*sizes) == formatCount, "Enum array mimatch");

	if (static_cast<unsigned int>(format) >= formatCount)
		return 0;

	return sizes[static_cast<unsigned int>(format)];
}

Texture::Texture()
	: m_impl(nullptr)
{
}

Texture::~Texture()
{
	delete m_impl;
}

Texture::Texture(Dimension dimension, unsigned int width, unsigned int height, unsigned int depth,
	unsigned int mipLevels)
	: m_impl(nullptr)
{
	initialize(dimension, width, height, depth, mipLevels);
}

Texture::Texture(const Texture& other)
	: m_impl(nullptr)
{
	if (other.m_impl)
		m_impl = new Impl(*other.m_impl);
}

Texture::Texture(Texture&& other)
	: m_impl(other.m_impl)
{
	other.m_impl = nullptr;
}

Texture& Texture::operator=(const Texture& other)
{
	if (&other == this)
		return *this;

	if (other.m_impl)
	{
		if (m_impl)
			*m_impl = *other.m_impl;
		else
			m_impl = new Impl(*other.m_impl);
	}
	else
		reset();

	return *this;
}

Texture& Texture::operator=(Texture&& other)
{
	delete m_impl;
	m_impl = other.m_impl;
	other.m_impl = nullptr;
	return *this;
}

bool Texture::isValid() const
{
	return m_impl != nullptr;
}

Texture::operator bool() const
{
	return m_impl != nullptr;
}

bool Texture::initialize(Dimension dimension, unsigned int width, unsigned int height,
	unsigned int depth, unsigned int mipLevels)
{
	reset();

	if (width == 0 || height == 0 || (dimension == Dimension::Dim3D && depth == 0))
		return false;

	m_impl->dimension = dimension;
	m_impl->width = width;
	m_impl->height = height;
	m_impl->depth = depth;
	m_impl->mipLevels = std::min(std::max(mipLevels, 1U),
		maxMipmapLevels(dimension, width, height, depth));
	m_impl->faces = dimension == Dimension::Cube ? 6 : 1;

	m_impl->images.resize(m_impl->mipLevels);
	for (DepthImageList& depthImages : m_impl->images)
	{
		depthImages.resize(std::max(m_impl->depth, 1U));
		for (FaceImageList& faceImages : depthImages)
			faceImages.resize(m_impl->faces);
	}

	return true;
}

void Texture::reset()
{
	delete m_impl;
	m_impl = nullptr;
}

Texture::Dimension Texture::dimension() const
{
	if (!m_impl)
		return Dimension::Dim2D;

	return m_impl->dimension;
}

bool Texture::isArray() const
{
	return m_impl && m_impl->dimension != Dimension::Dim3D && m_impl->depth > 0;
}

unsigned int Texture::width(unsigned int mipLevel) const
{
	if (!m_impl || mipLevel >= m_impl->mipLevels)
		return 0;

	return std::max(m_impl->width >> mipLevel, 1U);
}

unsigned int Texture::height(unsigned int mipLevel) const
{
	if (!m_impl || mipLevel >= m_impl->mipLevels)
		return 0;

	return std::max(m_impl->height >> mipLevel, 1U);
}

unsigned int Texture::depth(unsigned int mipLevel) const
{
	if (!m_impl || mipLevel >= m_impl->mipLevels)
		return 0;

	if (m_impl->dimension == Dimension::Dim3D)
		return std::max(m_impl->depth >> mipLevel, 1U);
	else
		return std::max(m_impl->depth, 1U);
}

unsigned int Texture::mipLevelCount() const
{
	if (!m_impl)
		return 0;

	return m_impl->mipLevels;
}

unsigned int Texture::faceCount() const
{
	if (!m_impl)
		return 0;

	return m_impl->faces;
}

const Image& Texture::getImage(unsigned int mipLevel, unsigned int depth) const
{
	// This will implicitly check if m_impl is present and mipLevel is in range. (depth would be 0)
	if (depth >= Texture::depth(mipLevel) || m_impl->faces != 1)
		return emptyImage;

	return m_impl->images[mipLevel][depth][0];
}

const Image& Texture::getImage(CubeFace face, unsigned int mipLevel, unsigned int depth) const
{
	// This will implicitly check if m_impl is present and mipLevel is in range. (depth would be 0)
	if (depth >= Texture::depth(mipLevel) || (m_impl->faces != 6 && face != CubeFace::PosX))
		return emptyImage;

	return m_impl->images[mipLevel][depth][static_cast<unsigned int>(face)];
}

bool Texture::setImage(const Image& image, unsigned int mipLevel, unsigned int depth)
{
	// This will implicitly check if m_impl is present and mipLevel is in range. (depth would be 0)
	if (depth >= Texture::depth(mipLevel) || image.width() != width(mipLevel) ||
		image.height() != height(mipLevel) || m_impl->faces != 1)
	{
		return false;
	}

	m_impl->images[mipLevel][depth][0] = image.convert(Image::Format::RGBAF);
	return m_impl->images[mipLevel][depth][0].isValid();
}

bool Texture::setImage(Image&& image, unsigned int mipLevel, unsigned int depth)
{
	// This will implicitly check if m_impl is present and mipLevel is in range. (depth would be 0)
	if (depth >= Texture::depth(mipLevel) || image.width() != width(mipLevel) ||
		image.height() != height(mipLevel) || m_impl->faces != 1)
	{
		return false;
	}

	if (image.format() == Image::Format::RGBAF)
		m_impl->images[mipLevel][depth][0] = std::move(image);
	else
		m_impl->images[mipLevel][depth][0] = image.convert(Image::Format::RGBAF);

	return m_impl->images[mipLevel][depth][0].isValid();
}

bool Texture::setImage(const Image& image, CubeFace face, unsigned int mipLevel, unsigned int depth)
{
	// This will implicitly check if m_impl is present and mipLevel is in range. (depth would be 0)
	if (depth >= Texture::depth(mipLevel) || image.width() != width(mipLevel) ||
		image.height() != height(mipLevel) || (m_impl->faces != 6 && face != CubeFace::PosX))
	{
		return false;
	}

	m_impl->images[mipLevel][depth][static_cast<unsigned int>(face)] =
		image.convert(Image::Format::RGBAF);
	return m_impl->images[mipLevel][depth][static_cast<unsigned int>(face)].isValid();
}

bool Texture::setImage(Image&& image, CubeFace face, unsigned int mipLevel, unsigned int depth)
{
	// This will implicitly check if m_impl is present and mipLevel is in range. (depth would be 0)
	if (depth >= Texture::depth(mipLevel) || image.width() != width(mipLevel) ||
		image.height() != height(mipLevel) || (m_impl->faces != 6 && face != CubeFace::PosX))
	{
		return false;
	}

	if (image.format() == Image::Format::RGBAF)
		m_impl->images[mipLevel][depth][static_cast<unsigned int>(face)] = std::move(image);
	else
	{
		m_impl->images[mipLevel][depth][static_cast<unsigned int>(face)] =
			image.convert(Image::Format::RGBAF);
	}

	return m_impl->images[mipLevel][depth][static_cast<unsigned int>(face)].isValid();
}

bool Texture::generateMipmaps(Image::ResizeFilter filter, unsigned int mipLevels)
{
	if (!m_impl)
		return false;

	for (const FaceImageList& faceImages : m_impl->images[0])
	{
		for (const Image& image : faceImages)
		{
			if (!image)
				return false;
		}
	}

	mipLevels = std::min(std::max(mipLevels, 1U), maxMipmapLevels(dimension(), width(), height(),
		depth()));
	m_impl->images.resize(mipLevels);

	if (m_impl->dimension == Dimension::Dim3D)
	{
		// Mipmap along X, Y, and Z.
		std::vector<Image> tempImages;
		for (unsigned int mip = 1; mip < m_impl->images.size(); ++mip)
		{
			// Resize the images from the previous mip level so they can be interpolated later.
			tempImages.resize(m_impl->images[mip - 1].size());
			for (unsigned int d = 0; d < tempImages.size(); ++d)
			{
				tempImages[d] = m_impl->images[mip - 1][d][0].resize(width(mip), height(mip),
					filter);
			}

			// Interpolate between the depth levels.
			DepthImageList& depthImages = m_impl->images[mip];
			depthImages.resize(depth(mip));
			for (unsigned int d = 0; d < depthImages.size(); ++d)
			{
				float interpD = (static_cast<float>(d) + 0.5f)/
					static_cast<float>(depthImages.size());
				unsigned int srcD0 = static_cast<unsigned int>(
					std::floor(interpD*static_cast<float>(tempImages.size() - 1)));
				unsigned int srcD1 = static_cast<unsigned int>(
					std::ceil(interpD*static_cast<float>(tempImages.size() - 1)));
				float t = interpD*static_cast<float>(tempImages.size()) - srcD0;

				unsigned int w = width(mip);
				unsigned int h = height(mip);
				depthImages[d][0].initialize(Image::Format::RGBAF, w, h);
				for (unsigned int y = 0; y < h; ++y)
				{
					ColorRGBAf* srcScanline0 = reinterpret_cast<ColorRGBAf*>(
						tempImages[srcD0].scanline(y));
					ColorRGBAf* srcScanline1 = reinterpret_cast<ColorRGBAf*>(
						tempImages[srcD1].scanline(y));
					ColorRGBAf* scanline = reinterpret_cast<ColorRGBAf*>(
						depthImages[d][0].scanline(y));
					for (unsigned int x = 0; x < w; ++x)
					{
						scanline[x].r = lerp(t, srcScanline0[x].r, srcScanline1[x].r);
						scanline[x].g = lerp(t, srcScanline0[x].g, srcScanline1[x].g);
						scanline[x].b = lerp(t, srcScanline0[x].b, srcScanline1[x].b);
						scanline[x].a = lerp(t, srcScanline0[x].a, srcScanline1[x].a);
					}
				}
			}
		}
	}
	else
	{
		for (unsigned int mip = 1; mip < m_impl->images.size(); ++mip)
		{
			DepthImageList& depthImages = m_impl->images[mip];
			depthImages.resize(depth(mip));
			for (unsigned int d = 0; d < depthImages.size(); ++d)
			{
				FaceImageList& faceImages = depthImages[d];
				faceImages.resize(m_impl->faces);
				for (unsigned int f = 0; f < faceImages.size(); ++f)
				{
					m_impl->images[mip][d][f] =
						m_impl->images[mip - 1][d][f].resize(width(mip), height(mip), filter);
				}
			}
		}
	}

	return true;
}

bool Texture::imagesComplete() const
{
	if (!m_impl)
		return false;

	for (const DepthImageList& depthImages : m_impl->images)
	{
		for (const FaceImageList& faceImages : depthImages)
		{
			for (const Image& image : faceImages)
			{
				if (!image)
					return false;
			}
		}
	}

	return true;
}

bool Texture::convert(Format format, Type type, Quality quality, Color colorSpace, Alpha alphaType,
	unsigned int threads)
{
	if (!imagesComplete() || !isFormatValid(format, type))
		return false;

	if (colorSpace == Color::sRGB && !hasNativeSRGB(format))
		return false;

	unsigned int w = width(), h = height();
	if (w < minWidth(format) || h < minHeight(format) || w % blockWidth(format) != 0 ||
		h % blockHeight(format) != 0)
	{
		return false;
	}

	m_impl->format = format;
	m_impl->type = type;
	m_impl->colorSpace = colorSpace;
	m_impl->alphaType = alphaType;

	threads = std::max(threads, 1U);
	if (threads == allCores)
		threads = std::thread::hardware_concurrency();

	if (!Converter::convert(*this, m_impl->images, m_impl->textures, quality, threads))
	{
		m_impl->format = Format::Unknown;
		return false;
	}

	return true;
}

bool Texture::converted() const
{
	return m_impl && !m_impl->textures.empty();
}

Texture::Format Texture::format() const
{
	if (!m_impl)
		return Format::Unknown;

	return m_impl->format;
}

Texture::Type Texture::type() const
{
	if (!m_impl)
		return Type::UNorm;

	return m_impl->type;
}

Texture::Color Texture::colorSpace() const
{
	if (!m_impl)
		return Color::Linear;

	return m_impl->colorSpace;
}

Texture::Alpha Texture::alphaType() const
{
	if (!m_impl)
		return Alpha::Standard;

	return m_impl->alphaType;
}

std::size_t Texture::dataSize(unsigned int mipLevel, unsigned int depth) const
{
	if (!converted() || depth >= Texture::depth(mipLevel) || m_impl->faces != 1)
		return 0;

	return m_impl->textures[mipLevel][depth][0].size();
}

std::size_t Texture::dataSize(CubeFace face, unsigned int mipLevel, unsigned int depth) const
{
	if (!converted() || depth >= Texture::depth(mipLevel) ||
		(m_impl->faces != 6 && face != CubeFace::PosX))
	{
		return 0;
	}

	return m_impl->textures[mipLevel][depth][static_cast<unsigned int>(face)].size();
}

const void* Texture::data(unsigned int mipLevel, unsigned int depth) const
{
	if (!converted() || depth >= Texture::depth(mipLevel) || m_impl->faces != 1)
		return nullptr;

	return m_impl->textures[mipLevel][depth][0].data();
}

const void* Texture::data(CubeFace face, unsigned int mipLevel, unsigned int depth) const
{
	if (!converted() || depth >= Texture::depth(mipLevel) ||
		(m_impl->faces != 6 && face != CubeFace::PosX))
	{
		return nullptr;
	}

	return m_impl->textures[mipLevel][depth][static_cast<unsigned int>(face)].data();
}

} // namespace cuttlefish
