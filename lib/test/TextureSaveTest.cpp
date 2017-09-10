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
#include <utility>

namespace cuttlefish
{

static const auto success = Texture::SaveResult::Success;
static const auto unsupported = Texture::SaveResult::Unsupported;
#if CUTTLEFISH_WINDOWS
static const char* nullFile = "nul";
#else
static const char* nullFile = "/dev/null";
#endif

struct TextureSaveTestInfo
{
	TextureSaveTestInfo(Texture::Format _format,
		std::vector<std::pair<Texture::Type, Texture::SaveResult>> _types)
		: format(_format)
		, types(std::move(_types))
	{
	}

	Texture::Format format;
	std::vector<std::pair<Texture::Type, Texture::SaveResult>> types;
};

class TextureSaveDdsTest : public testing::TestWithParam<TextureSaveTestInfo>
{
};

class TextureSaveSpecialDdsTest : public testing::TestWithParam<TextureSaveTestInfo>
{
};

class TextureSaveKtxTest : public testing::TestWithParam<TextureSaveTestInfo>
{
};

class TextureSaveSpecialKtxTest : public testing::TestWithParam<TextureSaveTestInfo>
{
};

class TextureSavePvrTest : public testing::TestWithParam<TextureSaveTestInfo>
{
};

class TextureSaveSpecialPvrTest : public testing::TestWithParam<TextureSaveTestInfo>
{
};

TEST_P(TextureSaveDdsTest, Save)
{
	const TextureSaveTestInfo& info = GetParam();
	for (const auto& typeInfo : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, typeInfo.first));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());

		EXPECT_EQ(typeInfo.second == success, Texture::isFormatValid(info.format, typeInfo.first,
			Texture::FileType::DDS));
		EXPECT_EQ(typeInfo.second, texture.save(nullFile, Texture::FileType::DDS));
	}
}

TEST_P(TextureSaveSpecialDdsTest, Save)
{
	const TextureSaveTestInfo& info = GetParam();
	for (const auto& typeInfo : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, typeInfo.first));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());

		EXPECT_EQ(typeInfo.second == success, Texture::isFormatValid(info.format, typeInfo.first,
			Texture::FileType::DDS));
		EXPECT_EQ(typeInfo.second, texture.save(nullFile, Texture::FileType::DDS));
	}
}

TEST_P(TextureSaveKtxTest, Save)
{
	const TextureSaveTestInfo& info = GetParam();
	for (const auto& typeInfo : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, typeInfo.first));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());

		EXPECT_EQ(typeInfo.second == success, Texture::isFormatValid(info.format, typeInfo.first,
			Texture::FileType::KTX));
		EXPECT_EQ(typeInfo.second, texture.save(nullFile, Texture::FileType::KTX));
	}
}

TEST_P(TextureSaveSpecialKtxTest, Save)
{
	const TextureSaveTestInfo& info = GetParam();
	for (const auto& typeInfo : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, typeInfo.first));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());

		EXPECT_EQ(typeInfo.second == success, Texture::isFormatValid(info.format, typeInfo.first,
			Texture::FileType::KTX));
		EXPECT_EQ(typeInfo.second, texture.save(nullFile, Texture::FileType::KTX));
	}
}

TEST_P(TextureSavePvrTest, Save)
{
	const TextureSaveTestInfo& info = GetParam();
	for (const auto& typeInfo : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, typeInfo.first));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());

		EXPECT_EQ(typeInfo.second == success, Texture::isFormatValid(info.format, typeInfo.first,
			Texture::FileType::PVR));
		EXPECT_EQ(typeInfo.second, texture.save(nullFile, Texture::FileType::PVR));
	}
}

TEST_P(TextureSaveSpecialPvrTest, Save)
{
	const TextureSaveTestInfo& info = GetParam();
	for (const auto& typeInfo : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, typeInfo.first));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());

		EXPECT_EQ(typeInfo.second == success, Texture::isFormatValid(info.format, typeInfo.first,
			Texture::FileType::PVR));
		EXPECT_EQ(typeInfo.second, texture.save(nullFile, Texture::FileType::PVR));
	}
}

INSTANTIATE_TEST_CASE_P(TextureSaveTestTypes,
	TextureSaveDdsTest,
	testing::Values(
		TextureSaveTestInfo(Texture::Format::R4G4, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R4G4B4A4, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::B4G4R4A4, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R5G6B5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::B5G6R5, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::R5G5B5A1, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::B5G5R5A1, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::A1R5G5B5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::R8G8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::R8G8B8, {{Texture::Type::UNorm, unsupported},
			{Texture::Type::SNorm, unsupported}, {Texture::Type::UInt, unsupported},
			{Texture::Type::Int, unsupported}}),
		TextureSaveTestInfo(Texture::Format::B8G8R8, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::R8G8B8A8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::B8G8R8A8, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::A8B8G8R8, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::A2R10G10B10, {{Texture::Type::UNorm, unsupported},
			{Texture::Type::UInt, unsupported}}),
		TextureSaveTestInfo(Texture::Format::A2B10G10R10, {{Texture::Type::UNorm, success},
			{Texture::Type::UInt, success}}),
		TextureSaveTestInfo(Texture::Format::R16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16B16, {{Texture::Type::UNorm, unsupported},
			{Texture::Type::SNorm, unsupported}, {Texture::Type::UInt, unsupported},
			{Texture::Type::Int, unsupported}, {Texture::Type::Float, unsupported}}),
		TextureSaveTestInfo(Texture::Format::R16G16B16A16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32B32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32B32A32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}})));

#if CUTTLEFISH_HAS_S3TC
#define S3TC_SAVE_DDS_TESTS \
	, TextureSaveTestInfo(Texture::Format::BC1_RGB, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC1_RGBA, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC2, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC3, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC4, {{Texture::Type::UNorm, success}, \
	{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC5, {{Texture::Type::UNorm, success}, \
	{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC6H, {{Texture::Type::UFloat, success}, \
	{Texture::Type::Float, success}}), \
	TextureSaveTestInfo(Texture::Format::BC7, {{Texture::Type::UNorm, success}})
#else
#define S3TC_SAVE_DDS_TESTS
#endif

#if CUTTLEFISH_HAS_ETC
#define ETC_SAVE_DDS_TESTS \
	, TextureSaveTestInfo(Texture::Format::ETC1, {{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8, {{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8A1, {{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8A8, {{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::EAC_R11, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::SNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::EAC_R11G11, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::SNorm, unsupported}})
#else
#define ETC_SAVE_DDS_TESTS
#endif

#if CUTTLEFISH_HAS_ASTC
#define ASTC_SAVE_DDS_TESTS \
	, TextureSaveTestInfo(Texture::Format::ASTC_4x4, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_5x4, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_5x5, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_6x5, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x5, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x6, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x8, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x5, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x6, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x8, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x10, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_12x10, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_12x12, {{Texture::Type::UNorm, unsupported}, \
		{Texture::Type::UFloat, unsupported}})
#else
#define ASTC_SAVE_DDS_TESTS
#endif

#if CUTTLEFISH_HAS_PVRTC
#define PVRTC_SAVE_DDS_TESTS \
	, TextureSaveTestInfo(Texture::Format::PVRTC1_RGB_2BPP, \
		{{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGBA_2BPP, \
		{{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGB_4BPP, \
		{{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGBA_4BPP, \
		{{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC2_RGBA_2BPP, \
		{{Texture::Type::UNorm, unsupported}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC2_RGBA_4BPP, \
		{{Texture::Type::UNorm, unsupported}})
#else
#define PVRTC_SAVE_DDS_TESTS
#endif

INSTANTIATE_TEST_CASE_P(TextureSaveTestTypes,
	TextureSaveSpecialDdsTest,
	testing::Values(
		TextureSaveTestInfo(Texture::Format::B10G11R11_UFloat, {{Texture::Type::UFloat, success}}),
		TextureSaveTestInfo(Texture::Format::E5B9G9R9_UFloat, {{Texture::Type::UFloat, success}})
		S3TC_SAVE_DDS_TESTS
		ETC_SAVE_DDS_TESTS
		ASTC_SAVE_DDS_TESTS
		PVRTC_SAVE_DDS_TESTS
		));

INSTANTIATE_TEST_CASE_P(TextureSaveTestTypes,
	TextureSaveKtxTest,
	testing::Values(
		TextureSaveTestInfo(Texture::Format::R4G4, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::R4G4B4A4, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::B4G4R4A4, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R5G6B5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::B5G6R5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R5G5B5A1, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::B5G5R5A1, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::A1R5G5B5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::R8G8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::R8G8B8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::B8G8R8, {{Texture::Type::UNorm, unsupported}}),
		TextureSaveTestInfo(Texture::Format::R8G8B8A8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::B8G8R8A8, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::A8B8G8R8, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::A2R10G10B10, {{Texture::Type::UNorm, success},
			{Texture::Type::UInt, success}}),
		TextureSaveTestInfo(Texture::Format::A2B10G10R10, {{Texture::Type::UNorm, success},
			{Texture::Type::UInt, success}}),
		TextureSaveTestInfo(Texture::Format::R16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16B16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16B16A16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32B32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32B32A32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}})));

#if CUTTLEFISH_HAS_S3TC
#define S3TC_SAVE_KTX_TESTS \
	, TextureSaveTestInfo(Texture::Format::BC1_RGB, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC1_RGBA, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC2, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC3, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC4, {{Texture::Type::UNorm, success}, \
	{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC5, {{Texture::Type::UNorm, success}, \
	{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC6H, {{Texture::Type::UFloat, success}, \
	{Texture::Type::Float, success}}), \
	TextureSaveTestInfo(Texture::Format::BC7, {{Texture::Type::UNorm, success}})
#else
#define S3TC_SAVE_KTX_TESTS
#endif

#if CUTTLEFISH_HAS_ETC
#define ETC_SAVE_KTX_TESTS \
	, TextureSaveTestInfo(Texture::Format::ETC1, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8A1, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8A8, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::EAC_R11, {{Texture::Type::UNorm, success}, \
		{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::EAC_R11G11, {{Texture::Type::UNorm, success}, \
		{Texture::Type::SNorm, success}})
#else
#define ETC_SAVE_KTX_TESTS
#endif

#if CUTTLEFISH_HAS_ASTC
#define ASTC_SAVE_KTX_TESTS \
	, TextureSaveTestInfo(Texture::Format::ASTC_4x4, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_5x4, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_5x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_6x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x6, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x8, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x6, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x8, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x10, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_12x10, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_12x12, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}})
#else
#define ASTC_SAVE_KTX_TESTS
#endif

#if CUTTLEFISH_HAS_PVRTC
#define PVRTC_SAVE_KTX_TESTS \
	, TextureSaveTestInfo(Texture::Format::PVRTC1_RGB_2BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGBA_2BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGB_4BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGBA_4BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC2_RGBA_2BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC2_RGBA_4BPP, \
		{{Texture::Type::UNorm, success}})
#else
#define PVRTC_SAVE_KTX_TESTS
#endif

INSTANTIATE_TEST_CASE_P(TextureSaveTestTypes,
	TextureSaveSpecialKtxTest,
	testing::Values(
		TextureSaveTestInfo(Texture::Format::B10G11R11_UFloat, {{Texture::Type::UFloat, success}}),
		TextureSaveTestInfo(Texture::Format::E5B9G9R9_UFloat, {{Texture::Type::UFloat, success}})
		S3TC_SAVE_KTX_TESTS
		ETC_SAVE_KTX_TESTS
		ASTC_SAVE_KTX_TESTS
		PVRTC_SAVE_KTX_TESTS
		));

INSTANTIATE_TEST_CASE_P(TextureSaveTestTypes,
	TextureSavePvrTest,
	testing::Values(
		TextureSaveTestInfo(Texture::Format::R4G4, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R4G4B4A4, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::B4G4R4A4, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R5G6B5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::B5G6R5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R5G5B5A1, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::B5G5R5A1, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::A1R5G5B5, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::R8G8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::R8G8B8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::B8G8R8, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::R8G8B8A8, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}}),
		TextureSaveTestInfo(Texture::Format::B8G8R8A8, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::A8B8G8R8, {{Texture::Type::UNorm, success}}),
		TextureSaveTestInfo(Texture::Format::A2R10G10B10, {{Texture::Type::UNorm, success},
			{Texture::Type::UInt, success}}),
		TextureSaveTestInfo(Texture::Format::A2B10G10R10, {{Texture::Type::UNorm, success},
			{Texture::Type::UInt, success}}),
		TextureSaveTestInfo(Texture::Format::R16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16B16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R16G16B16A16, {{Texture::Type::UNorm, success},
			{Texture::Type::SNorm, success}, {Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32B32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}}),
		TextureSaveTestInfo(Texture::Format::R32G32B32A32, {{Texture::Type::UInt, success},
			{Texture::Type::Int, success}, {Texture::Type::Float, success}})));

#if CUTTLEFISH_HAS_S3TC
#define S3TC_SAVE_PVR_TESTS \
	, TextureSaveTestInfo(Texture::Format::BC1_RGB, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC1_RGBA, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC2, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC3, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC4, {{Texture::Type::UNorm, success}, \
	{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC5, {{Texture::Type::UNorm, success}, \
	{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::BC6H, {{Texture::Type::UFloat, success}, \
	{Texture::Type::Float, success}}), \
	TextureSaveTestInfo(Texture::Format::BC7, {{Texture::Type::UNorm, success}})
#else
#define S3TC_SAVE_PVR_TESTS
#endif

#if CUTTLEFISH_HAS_ETC
#define ETC_SAVE_PVR_TESTS \
	, TextureSaveTestInfo(Texture::Format::ETC1, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8A1, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::ETC2_R8G8B8A8, {{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::EAC_R11, {{Texture::Type::UNorm, success}, \
		{Texture::Type::SNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::EAC_R11G11, {{Texture::Type::UNorm, success}, \
		{Texture::Type::SNorm, success}})
#else
#define ETC_SAVE_PVR_TESTS
#endif

#if CUTTLEFISH_HAS_ASTC
#define ASTC_SAVE_PVR_TESTS \
	, TextureSaveTestInfo(Texture::Format::ASTC_4x4, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_5x4, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_5x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_6x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x6, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_8x8, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x5, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x6, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x8, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_10x10, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_12x10, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}}), \
	TextureSaveTestInfo(Texture::Format::ASTC_12x12, {{Texture::Type::UNorm, success}, \
		{Texture::Type::UFloat, success}})
#else
#define ASTC_SAVE_PVR_TESTS
#endif

#if CUTTLEFISH_HAS_PVRTC
#define PVRTC_SAVE_PVR_TESTS \
	, TextureSaveTestInfo(Texture::Format::PVRTC1_RGB_2BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGBA_2BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGB_4BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC1_RGBA_4BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC2_RGBA_2BPP, \
		{{Texture::Type::UNorm, success}}), \
	TextureSaveTestInfo(Texture::Format::PVRTC2_RGBA_4BPP, \
		{{Texture::Type::UNorm, success}})
#else
#define PVRTC_SAVE_PVR_TESTS
#endif

INSTANTIATE_TEST_CASE_P(TextureSaveTestTypes,
	TextureSaveSpecialPvrTest,
	testing::Values(
		TextureSaveTestInfo(Texture::Format::B10G11R11_UFloat, {{Texture::Type::UFloat, success}}),
		TextureSaveTestInfo(Texture::Format::E5B9G9R9_UFloat, {{Texture::Type::UFloat, success}})
		S3TC_SAVE_PVR_TESTS
		ETC_SAVE_PVR_TESTS
		ASTC_SAVE_PVR_TESTS
		PVRTC_SAVE_PVR_TESTS
		));

} // namespace cuttlefish
