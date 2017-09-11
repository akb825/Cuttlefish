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

#include <cuttlefish/Color.h>
#include <cuttlefish/Image.h>
#include <cuttlefish/Texture.h>
#include <gtest/gtest.h>
#include <vector>

namespace cuttlefish
{

struct TextureConvertTestInfo
{
	TextureConvertTestInfo(Texture::Format _format, std::vector<Texture::Type> _types)
		: format(_format)
		, types(std::move(_types))
	{
	}

	Texture::Format format;
	std::vector<Texture::Type> types;
};

class TextureConvertTest : public testing::TestWithParam<TextureConvertTestInfo>
{
};

class TextureConvertSpecialTest : public testing::TestWithParam<TextureConvertTestInfo>
{
};

TEST(TextureTest, Create)
{
	Texture texture(Texture::Dimension::Dim2D, 10, 15, 0);
	EXPECT_EQ(Texture::Dimension::Dim2D, texture.dimension());
	EXPECT_EQ(10U, texture.width());
	EXPECT_EQ(15U, texture.height());
	EXPECT_EQ(1U, texture.depth());
	EXPECT_FALSE(texture.isArray());
	EXPECT_EQ(1U, texture.mipLevelCount());
	EXPECT_EQ(1U, texture.faceCount());

	EXPECT_TRUE(texture.initialize(Texture::Dimension::Cube, 15, 10, 5, Texture::allMipLevels));
	EXPECT_EQ(Texture::Dimension::Cube, texture.dimension());
	EXPECT_EQ(15U, texture.width());
	EXPECT_EQ(10U, texture.height());
	EXPECT_EQ(5U, texture.depth());
	EXPECT_TRUE(texture.isArray());
	EXPECT_EQ(4U, texture.mipLevelCount());
}

TEST(TextureTest, SetImages)
{
	Texture texture(Texture::Dimension::Cube, 15, 10, 5);

	EXPECT_FALSE(texture.setImage(Image(Image::Format::RGBAF, 10, 15), Texture::CubeFace::PosX));

	for (unsigned int i = 0; i < 6; ++i)
	{
		for (unsigned int j = 0; j < 5; ++j)
		{
			EXPECT_FALSE(texture.imagesComplete());
			EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10),
				static_cast<Texture::CubeFace>(i), 0, j));
		}
	}

	EXPECT_TRUE(texture.imagesComplete());
}

TEST(TextureTest, GenerateMipmaps)
{
	Texture texture(Texture::Dimension::Cube, 15, 10, 5);

	EXPECT_FALSE(texture.setImage(Image(Image::Format::RGBAF, 10, 15), Texture::CubeFace::PosX));

	for (unsigned int i = 0; i < 6; ++i)
	{
		for (unsigned int j = 0; j < 5; ++j)
		{
			EXPECT_FALSE(texture.imagesComplete());
			EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10),
				static_cast<Texture::CubeFace>(i), 0, j));
		}
	}

	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_TRUE(texture.generateMipmaps());
	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_EQ(4U, texture.mipLevelCount());

	EXPECT_EQ(15U, texture.getImage(Texture::CubeFace::PosX, 0, 1).width());
	EXPECT_EQ(10U, texture.getImage(Texture::CubeFace::PosX, 0, 1).height());
	EXPECT_EQ(7U, texture.getImage(Texture::CubeFace::PosX, 1, 1).width());
	EXPECT_EQ(5U, texture.getImage(Texture::CubeFace::PosX, 1, 1).height());
	EXPECT_EQ(3U, texture.getImage(Texture::CubeFace::PosX, 2, 1).width());
	EXPECT_EQ(2U, texture.getImage(Texture::CubeFace::PosX, 2, 1).height());
	EXPECT_EQ(1U, texture.getImage(Texture::CubeFace::PosX, 3, 1).width());
	EXPECT_EQ(1U, texture.getImage(Texture::CubeFace::PosX, 3, 1).height());
}

TEST(TextureTest, Generate3DMipmaps)
{
	Texture texture(Texture::Dimension::Dim3D, 15, 10, 5);

	EXPECT_FALSE(texture.setImage(Image(Image::Format::RGBAF, 10, 15), Texture::CubeFace::PosX));

	for (unsigned int j = 0; j < 5; ++j)
	{
		EXPECT_FALSE(texture.imagesComplete());
		EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10), 0, j));
	}

	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_TRUE(texture.generateMipmaps());
	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_EQ(4U, texture.mipLevelCount());

	EXPECT_EQ(15U, texture.getImage(0, 0).width());
	EXPECT_EQ(10U, texture.getImage(0, 0).height());
	EXPECT_EQ(7U, texture.getImage(1, 0).width());
	EXPECT_EQ(5U, texture.getImage(1, 0).height());
	EXPECT_EQ(3U, texture.getImage(2, 0).width());
	EXPECT_EQ(2U, texture.getImage(2, 0).height());
	EXPECT_EQ(1U, texture.getImage(3, 0).width());
	EXPECT_EQ(1U, texture.getImage(3, 0).height());

	EXPECT_FALSE(texture.getImage(1, 2));
	EXPECT_TRUE(texture.getImage(1, 1));
	EXPECT_FALSE(texture.getImage(2, 1));
	EXPECT_TRUE(texture.getImage(2, 0));
	EXPECT_FALSE(texture.getImage(3, 1));
	EXPECT_TRUE(texture.getImage(3, 0));
}

TEST_P(TextureConvertTest, Convert)
{
	const TextureConvertTestInfo& info = GetParam();
	for (Texture::Type type : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, type));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());
	}
}

TEST_P(TextureConvertSpecialTest, Convert)
{
	const TextureConvertTestInfo& info = GetParam();
	for (Texture::Type type : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, type));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());
	}
}

INSTANTIATE_TEST_CASE_P(TextureConvertTestTypes,
	TextureConvertTest,
	testing::Values(
		TextureConvertTestInfo(Texture::Format::R4G4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R4G4B4A4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::B4G4R4A4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A4R4G4B4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R5G6B5, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::B5G6R5, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R5G5B5A1, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::B5G5R5A1, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A1R5G5B5, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R8, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::R8G8, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::R8G8B8, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::B8G8R8, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R8G8B8A8, {Texture::Type::UNorm,
			Texture::Type::SNorm, Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::B8G8R8A8, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A8B8G8R8, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A2R10G10B10, {Texture::Type::UNorm,
			Texture::Type::UInt}),
		TextureConvertTestInfo(Texture::Format::A2B10G10R10, {Texture::Type::UNorm,
			Texture::Type::UInt}),
		TextureConvertTestInfo(Texture::Format::R16, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R16G16, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R16G16B16, {Texture::Type::UNorm,
			Texture::Type::SNorm, Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R16G16B16A16, {Texture::Type::UNorm,
			Texture::Type::SNorm, Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32, {Texture::Type::UInt, Texture::Type::Int,
			Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32G32, {Texture::Type::UInt, Texture::Type::Int,
			Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32G32B32, {Texture::Type::UInt, Texture::Type::Int,
			Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32G32B32A32, {Texture::Type::UInt,
			Texture::Type::Int, Texture::Type::Float})));

#if CUTTLEFISH_HAS_S3TC
#define S3TC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::BC1_RGB, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC1_RGBA, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC2, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC3, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC4, {Texture::Type::UNorm, Texture::Type::SNorm}), \
	TextureConvertTestInfo(Texture::Format::BC5, {Texture::Type::UNorm, Texture::Type::SNorm}), \
	TextureConvertTestInfo(Texture::Format::BC6H, {Texture::Type::UFloat, Texture::Type::Float}), \
	TextureConvertTestInfo(Texture::Format::BC7, {Texture::Type::UNorm})
#else
#define S3TC_CONVERSION_TESTS
#endif

#if CUTTLEFISH_HAS_ETC
#define ETC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::ETC1, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::ETC2_R8G8B8, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::ETC2_R8G8B8A1, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::ETC2_R8G8B8A8, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::EAC_R11, {Texture::Type::UNorm, Texture::Type::SNorm}), \
	TextureConvertTestInfo(Texture::Format::EAC_R11G11, {Texture::Type::UNorm, Texture::Type::SNorm})
#else
#define ETC_CONVERSION_TESTS
#endif

#if CUTTLEFISH_HAS_ASTC
#define ASTC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::ASTC_4x4, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_5x4, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_5x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_6x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_8x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_8x6, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_8x8, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x6, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x8, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x10, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_12x10, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_12x12, {Texture::Type::UNorm, Texture::Type::UFloat})
#else
#define ASTC_CONVERSION_TESTS
#endif

#if CUTTLEFISH_HAS_PVRTC
#define PVRTC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::PVRTC1_RGB_2BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC1_RGBA_2BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC1_RGB_4BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC1_RGBA_4BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC2_RGBA_2BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC2_RGBA_4BPP, {Texture::Type::UNorm})
#else
#define PVRTC_CONVERSION_TESTS
#endif

INSTANTIATE_TEST_CASE_P(TextureConvertTestTypes,
	TextureConvertSpecialTest,
	testing::Values(
		TextureConvertTestInfo(Texture::Format::B10G11R11_UFloat, {Texture::Type::UFloat}),
		TextureConvertTestInfo(Texture::Format::E5B9G9R9_UFloat, {Texture::Type::UFloat})
		S3TC_CONVERSION_TESTS
		ETC_CONVERSION_TESTS
		ASTC_CONVERSION_TESTS
		PVRTC_CONVERSION_TESTS
		));

} // namespace cuttlefish
