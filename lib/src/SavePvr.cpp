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

#include "SaveDds.h"
#include "Shared.h"
#include <cassert>
#include <cstring>
#include <fstream>

#define PVR_GENERIC_FORMAT(channel0, bits0, channel1, bits1, channel2, bits2, channel3, bits3) \
	(((uint64_t)(channel0) | ((uint64_t)(channel1) << 8) | ((uint64_t)(channel2) << 16) | \
		((uint64_t)(channel3) << 24) | \
	((uint64_t)(bits0) << 32) | ((uint64_t)(bits1) << 40) | ((uint64_t)(bits2) << 48) | \
		((uint64_t)(bits3) << 56)))

namespace cuttlefish
{

enum PvrChannelType
{
	PvrChannelType_UByteN,
	PvrChannelType_SByteN,
	PvrChannelType_UByte,
	PvrChannelType_SByte,
	PvrChannelType_UShortN,
	PvrChannelType_SShortN,
	PvrChannelType_UShort,
	PvrChannelType_SShort,
	PvrChannelType_UIntN,
	PvrChannelType_SIntN,
	PvrChannelType_UInt,
	PvrChannelType_SInt,
	PvrChannelType_Float,
	PvrChannelType_UFloat,
	PvrChannelTypeCount
};
static_assert(PvrChannelTypeCount == 14, "invalid PVR channel type enum");

enum PvrSpecialFormat
{
	PvrSpecialFormat_PVRTC_2bppRGB,
	PvrSpecialFormat_PVRTC_2bppRGBA,
	PvrSpecialFormat_PVRTC_4bppRGB,
	PvrSpecialFormat_PVRTC_4bppRGBA,
	PvrSpecialFormat_PVRTC2_2bpp,
	PvrSpecialFormat_PVRTC2_4bpp,
	PvrSpecialFormat_ETC1,
	PvrSpecialFormat_DXT1, PvrSpecialFormat_BC1 = PvrSpecialFormat_DXT1,
	PvrSpecialFormat_DXT2,
	PvrSpecialFormat_DXT3, PvrSpecialFormat_BC2 = PvrSpecialFormat_DXT3,
	PvrSpecialFormat_DXT4,
	PvrSpecialFormat_DXT5, PvrSpecialFormat_BC3 = PvrSpecialFormat_DXT5,
	PvrSpecialFormat_BC4,
	PvrSpecialFormat_BC5,
	PvrSpecialFormat_BC6,
	PvrSpecialFormat_BC7,
	PvrSpecialFormat_UYVY,
	PvrSpecialFormat_YUY2,
	PvrSpecialFormat_BW1bpp,
	PvrSpecialFormat_R9G9B9E5_UFloat,
	PvrSpecialFormat_R8G8B8G8,
	PvrSpecialFormat_G8R8G8B8,
	PvrSpecialFormat_ETC2_RGB,
	PvrSpecialFormat_ETC2_RGBA,
	PvrSpecialFormat_ETC2_RGB_A1,
	PvrSpecialFormat_EAC_R11,
	PvrSpecialFormat_EAC_RG11,
	PvrSpecialFormat_ASTC_4x4,
	PvrSpecialFormat_ASTC_5x4,
	PvrSpecialFormat_ASTC_5x5,
	PvrSpecialFormat_ASTC_6x5,
	PvrSpecialFormat_ASTC_6x6,
	PvrSpecialFormat_ASTC_8x5,
	PvrSpecialFormat_ASTC_8x6,
	PvrSpecialFormat_ASTC_8x8,
	PvrSpecialFormat_ASTC_10x5,
	PvrSpecialFormat_ASTC_10x6,
	PvrSpecialFormat_ASTC_10x8,
	PvrSpecialFormat_ASTC_10x10,
	PvrSpecialFormat_ASTC_12x10,
	PvrSpecialFormat_ASTC_12x12,
	PvrSpecialFormat_ASTC_3x3x3,
	PvrSpecialFormat_ASTC_4x3x3,
	PvrSpecialFormat_ASTC_4x4x3,
	PvrSpecialFormat_ASTC_4x4x4,
	PvrSpecialFormat_ASTC_5x4x4,
	PvrSpecialFormat_ASTC_5x5x4,
	PvrSpecialFormat_ASTC_5x5x5,
	PvrSpecialFormat_ASTC_6x5x5,
	PvrSpecialFormat_ASTC_6x6x5,
	PvrSpecialFormat_ASTC_6x6x6,
	PvrSpecialFormatCount
};
static_assert(PvrSpecialFormatCount == 51, "invalid PVR special format enum");

static PvrChannelType getChannelType(const Texture& texture)
{
	switch (texture.type())
	{
		case Texture::Type::UNorm:
			switch (texture.format())
			{
				case Texture::Format::R4G4:
				case Texture::Format::R8:
				case Texture::Format::R8G8:
				case Texture::Format::R8G8B8:
				case Texture::Format::B8G8R8:
				case Texture::Format::R8G8B8A8:
				case Texture::Format::B8G8R8A8:
				case Texture::Format::A8B8G8R8:
				case Texture::Format::BC4:
				case Texture::Format::BC5:
					return PvrChannelType_UByteN;
				case Texture::Format::R4G4B4A4:
				case Texture::Format::B4G4R4A4:
				case Texture::Format::A4R4G4B4:
				case Texture::Format::R5G6B5:
				case Texture::Format::B5G6R5:
				case Texture::Format::R5G5B5A1:
				case Texture::Format::B5G5R5A1:
				case Texture::Format::A1R5G5B5:
				case Texture::Format::R16:
				case Texture::Format::R16G16:
				case Texture::Format::R16G16B16:
				case Texture::Format::R16G16B16A16:
				case Texture::Format::EAC_R11:
				case Texture::Format::EAC_R11G11:
					return PvrChannelType_UShortN;
				case Texture::Format::A2R10G10B10:
				case Texture::Format::A2B10G10R10:
				case Texture::Format::R32:
				case Texture::Format::R32G32:
				case Texture::Format::R32G32B32:
				case Texture::Format::R32G32B32A32:
					return PvrChannelType_UIntN;
				default:
					return PvrChannelType_UByteN;
			}
		case Texture::Type::SNorm:
			switch (texture.format())
			{
				case Texture::Format::R4G4:
				case Texture::Format::R8:
				case Texture::Format::R8G8:
				case Texture::Format::R8G8B8:
				case Texture::Format::B8G8R8:
				case Texture::Format::R8G8B8A8:
				case Texture::Format::B8G8R8A8:
				case Texture::Format::A8B8G8R8:
				case Texture::Format::BC4:
				case Texture::Format::BC5:
					return PvrChannelType_SByteN;
				case Texture::Format::R4G4B4A4:
				case Texture::Format::B4G4R4A4:
				case Texture::Format::A4R4G4B4:
				case Texture::Format::R5G6B5:
				case Texture::Format::B5G6R5:
				case Texture::Format::R5G5B5A1:
				case Texture::Format::B5G5R5A1:
				case Texture::Format::A1R5G5B5:
				case Texture::Format::R16:
				case Texture::Format::R16G16:
				case Texture::Format::R16G16B16:
				case Texture::Format::R16G16B16A16:
				case Texture::Format::EAC_R11:
				case Texture::Format::EAC_R11G11:
					return PvrChannelType_SShortN;
				case Texture::Format::A2R10G10B10:
				case Texture::Format::A2B10G10R10:
				case Texture::Format::R32:
				case Texture::Format::R32G32:
				case Texture::Format::R32G32B32:
				case Texture::Format::R32G32B32A32:
					return PvrChannelType_SIntN;
				default:
					return PvrChannelType_SByteN;
			}
		case Texture::Type::UInt:
			switch (texture.format())
			{
				case Texture::Format::R4G4:
				case Texture::Format::R8:
				case Texture::Format::R8G8:
				case Texture::Format::R8G8B8:
				case Texture::Format::B8G8R8:
				case Texture::Format::R8G8B8A8:
				case Texture::Format::B8G8R8A8:
				case Texture::Format::A8B8G8R8:
					return PvrChannelType_UByte;
				case Texture::Format::R4G4B4A4:
				case Texture::Format::B4G4R4A4:
				case Texture::Format::A4R4G4B4:
				case Texture::Format::R5G6B5:
				case Texture::Format::B5G6R5:
				case Texture::Format::R5G5B5A1:
				case Texture::Format::B5G5R5A1:
				case Texture::Format::A1R5G5B5:
				case Texture::Format::R16:
				case Texture::Format::R16G16:
				case Texture::Format::R16G16B16:
				case Texture::Format::R16G16B16A16:
					return PvrChannelType_UShort;
				case Texture::Format::A2R10G10B10:
				case Texture::Format::A2B10G10R10:
				case Texture::Format::R32:
				case Texture::Format::R32G32:
				case Texture::Format::R32G32B32:
				case Texture::Format::R32G32B32A32:
					return PvrChannelType_UInt;
				default:
					return PvrChannelType_UByte;
			}
		case Texture::Type::Int:
			switch (texture.format())
			{
				case Texture::Format::R4G4:
				case Texture::Format::R8:
				case Texture::Format::R8G8:
				case Texture::Format::R8G8B8:
				case Texture::Format::B8G8R8:
				case Texture::Format::R8G8B8A8:
				case Texture::Format::B8G8R8A8:
				case Texture::Format::A8B8G8R8:
					return PvrChannelType_SByte;
				case Texture::Format::R4G4B4A4:
				case Texture::Format::B4G4R4A4:
				case Texture::Format::A4R4G4B4:
				case Texture::Format::R5G6B5:
				case Texture::Format::B5G6R5:
				case Texture::Format::R5G5B5A1:
				case Texture::Format::B5G5R5A1:
				case Texture::Format::A1R5G5B5:
				case Texture::Format::R16:
				case Texture::Format::R16G16:
				case Texture::Format::R16G16B16:
				case Texture::Format::R16G16B16A16:
					return PvrChannelType_SShort;
				case Texture::Format::A2R10G10B10:
				case Texture::Format::A2B10G10R10:
				case Texture::Format::R32:
				case Texture::Format::R32G32:
				case Texture::Format::R32G32B32:
				case Texture::Format::R32G32B32A32:
					return PvrChannelType_SInt;
				default:
					return PvrChannelType_UByte;
			}
		case Texture::Type::UFloat:
			return PvrChannelType_UFloat;
		case Texture::Type::Float:
			return PvrChannelType_Float;
	}
	assert(false);
	return PvrChannelType_UByte;
}

static bool getPixelFormat(std::uint64_t& pixelFormat, Texture::Format format,
	Texture::Alpha alphaType)
{
	switch (format)
	{
		case Texture::Format::Unknown:
			return false;
		case Texture::Format::R4G4:
			pixelFormat = PVR_GENERIC_FORMAT('r', 4, 'g', 4, 0, 0, 0, 0);
			return true;
		case Texture::Format::R4G4B4A4:
			pixelFormat = PVR_GENERIC_FORMAT('r', 4, 'g', 4, 'b', 4, 'a', 4);
			return true;
		case Texture::Format::B4G4R4A4:
			pixelFormat = PVR_GENERIC_FORMAT('b', 4, 'g', 4, 'r', 4, 'a', 4);
			return true;
		case Texture::Format::A4R4G4B4:
			pixelFormat = PVR_GENERIC_FORMAT('a', 4, 'r', 4, 'g', 4, 'b', 4);
			return true;
		case Texture::Format::R5G6B5:
			pixelFormat = PVR_GENERIC_FORMAT('r', 5, 'g', 6, 'b', 5, 0, 0);
			return true;
		case Texture::Format::B5G6R5:
			pixelFormat = PVR_GENERIC_FORMAT('b', 5, 'g', 6, 'r', 5, 0, 0);
			return true;
		case Texture::Format::R5G5B5A1:
			pixelFormat = PVR_GENERIC_FORMAT('r', 5, 'g', 5, 'b', 5, 'a', 1);
			return true;
		case Texture::Format::B5G5R5A1:
			pixelFormat = PVR_GENERIC_FORMAT('b', 5, 'g', 5, 'r', 5, 'a', 1);
			return true;
		case Texture::Format::A1R5G5B5:
			pixelFormat = PVR_GENERIC_FORMAT('a', 1, 'r', 5, 'g', 5, 'b', 5);
			return true;
		case Texture::Format::R8:
			pixelFormat = PVR_GENERIC_FORMAT('r', 8, 0, 0, 0, 0, 0, 0);
			return true;
		case Texture::Format::R8G8:
			pixelFormat = PVR_GENERIC_FORMAT('r', 8, 'g', 8, 0, 0, 0, 0);
			return true;
		case Texture::Format::R8G8B8:
			pixelFormat = PVR_GENERIC_FORMAT('r', 8, 'g', 8, 'b', 8, 0, 0);
			return true;
		case Texture::Format::B8G8R8:
			pixelFormat = PVR_GENERIC_FORMAT('b', 8, 'g', 8, 'r', 8, 0, 0);
			return true;
		case Texture::Format::R8G8B8A8:
			pixelFormat = PVR_GENERIC_FORMAT('r', 8, 'g', 8, 'b', 8, 'a', 8);
			return true;
		case Texture::Format::B8G8R8A8:
			pixelFormat = PVR_GENERIC_FORMAT('b', 8, 'g', 8, 'r', 8, 'a', 8);
			return true;
		case Texture::Format::A8B8G8R8:
			pixelFormat = PVR_GENERIC_FORMAT('a', 8, 'b', 8, 'g', 8, 'r', 8);
			return true;
		case Texture::Format::A2R10G10B10:
			pixelFormat = PVR_GENERIC_FORMAT('a', 2, 'r', 10, 'g', 10, 'b', 10);
			return true;
		case Texture::Format::A2B10G10R10:
			pixelFormat = PVR_GENERIC_FORMAT('a', 2, 'b', 10, 'g', 10, 'r', 10);
			return true;
		case Texture::Format::R16:
			pixelFormat = PVR_GENERIC_FORMAT('r', 16, 0, 0, 0, 0, 0, 0);
			return true;
		case Texture::Format::R16G16:
			pixelFormat = PVR_GENERIC_FORMAT('r', 16, 'g', 16, 0, 0, 0, 0);
			return true;
		case Texture::Format::R16G16B16:
			pixelFormat = PVR_GENERIC_FORMAT('r', 16, 'g', 16, 'b', 16, 0, 0);
			return true;
		case Texture::Format::R16G16B16A16:
			pixelFormat = PVR_GENERIC_FORMAT('r', 16, 'g', 16, 'b', 16, 'a', 16);
			return true;
		case Texture::Format::R32:
			pixelFormat = PVR_GENERIC_FORMAT('r', 32, 0, 0, 0, 0, 0, 0);
			return true;
		case Texture::Format::R32G32:
			pixelFormat = PVR_GENERIC_FORMAT('r', 32, 'g', 32, 0, 0, 0, 0);
			return true;
		case Texture::Format::R32G32B32:
			pixelFormat = PVR_GENERIC_FORMAT('r', 32, 'g', 32, 'b', 32, 0, 0);
			return true;
		case Texture::Format::R32G32B32A32:
			pixelFormat = PVR_GENERIC_FORMAT('r', 32, 'g', 32, 'b', 32, 'a', 32);
			return true;
		case Texture::Format::B10G11R11_UFloat:
			pixelFormat = PVR_GENERIC_FORMAT('b', 10, 'g', 11, 'r', 11, 0, 0);
			return true;
		case Texture::Format::E5B9G9R9_UFloat:
			pixelFormat = PvrSpecialFormat_R9G9B9E5_UFloat;
			return true;
		case Texture::Format::BC1_RGB:
		case Texture::Format::BC1_RGBA:
			pixelFormat = PvrSpecialFormat_BC1;
			return true;
		case Texture::Format::BC2:
			if (alphaType == Texture::Alpha::PreMultiplied)
				pixelFormat = PvrSpecialFormat_DXT2;\
			else
				pixelFormat = PvrSpecialFormat_BC2;
			return true;
		case Texture::Format::BC3:
			if (alphaType == Texture::Alpha::PreMultiplied)
				pixelFormat = PvrSpecialFormat_DXT4;
			else
				pixelFormat = PvrSpecialFormat_BC3;
			return true;
		case Texture::Format::BC4:
			pixelFormat = PvrSpecialFormat_BC4;
			return true;
		case Texture::Format::BC5:
			pixelFormat = PvrSpecialFormat_BC5;
			return true;
		case Texture::Format::BC6H:
			pixelFormat = PvrSpecialFormat_BC6;
			return true;
		case Texture::Format::BC7:
			pixelFormat = PvrSpecialFormat_BC7;
			return true;
		case Texture::Format::ETC1:
			pixelFormat = PvrSpecialFormat_ETC1;
			return true;
		case Texture::Format::ETC2_R8G8B8:
			pixelFormat = PvrSpecialFormat_ETC2_RGB;
			return true;
		case Texture::Format::ETC2_R8G8B8A1:
			pixelFormat = PvrSpecialFormat_ETC2_RGB_A1;
			return true;
		case Texture::Format::ETC2_R8G8B8A8:
			pixelFormat = PvrSpecialFormat_ETC2_RGBA;
			return true;
		case Texture::Format::EAC_R11:
			pixelFormat = PvrSpecialFormat_EAC_R11;
			return true;
		case Texture::Format::EAC_R11G11:
			pixelFormat = PvrSpecialFormat_EAC_RG11;
			return true;
		case Texture::Format::ASTC_4x4:
			pixelFormat = PvrSpecialFormat_ASTC_4x4;
			return true;
		case Texture::Format::ASTC_5x4:
			pixelFormat = PvrSpecialFormat_ASTC_5x4;
			return true;
		case Texture::Format::ASTC_5x5:
			pixelFormat = PvrSpecialFormat_ASTC_5x5;
			return true;
		case Texture::Format::ASTC_6x5:
			pixelFormat = PvrSpecialFormat_ASTC_6x5;
			return true;
		case Texture::Format::ASTC_6x6:
			pixelFormat = PvrSpecialFormat_ASTC_6x6;
			return true;
		case Texture::Format::ASTC_8x5:
			pixelFormat = PvrSpecialFormat_ASTC_8x5;
			return true;
		case Texture::Format::ASTC_8x6:
			pixelFormat = PvrSpecialFormat_ASTC_8x6;
			return true;
		case Texture::Format::ASTC_8x8:
			pixelFormat = PvrSpecialFormat_ASTC_8x8;
			return true;
		case Texture::Format::ASTC_10x5:
			pixelFormat = PvrSpecialFormat_ASTC_10x5;
			return true;
		case Texture::Format::ASTC_10x6:
			pixelFormat = PvrSpecialFormat_ASTC_10x6;
			return true;
		case Texture::Format::ASTC_10x8:
			pixelFormat = PvrSpecialFormat_ASTC_10x8;
			return true;
		case Texture::Format::ASTC_10x10:
			pixelFormat = PvrSpecialFormat_ASTC_10x10;
			return true;
		case Texture::Format::ASTC_12x10:
			pixelFormat = PvrSpecialFormat_ASTC_12x10;
			return true;
		case Texture::Format::ASTC_12x12:
			pixelFormat = PvrSpecialFormat_ASTC_12x12;
			return true;
		case Texture::Format::PVRTC1_RGB_2BPP:
			pixelFormat = PvrSpecialFormat_PVRTC_2bppRGB;
			return true;
		case Texture::Format::PVRTC1_RGBA_2BPP:
			pixelFormat = PvrSpecialFormat_PVRTC_2bppRGBA;
			return true;
		case Texture::Format::PVRTC1_RGB_4BPP:
			pixelFormat = PvrSpecialFormat_PVRTC_4bppRGB;
			return true;
		case Texture::Format::PVRTC1_RGBA_4BPP:
			pixelFormat = PvrSpecialFormat_PVRTC_4bppRGBA;
			return true;
		case Texture::Format::PVRTC2_RGBA_2BPP:
			pixelFormat = PvrSpecialFormat_PVRTC2_2bpp;
			return true;
		case Texture::Format::PVRTC2_RGBA_4BPP:
			pixelFormat = PvrSpecialFormat_PVRTC2_4bpp;
			return true;
	}
	assert(false);
	return false;
}

template <typename T>
static bool write(std::ofstream& stream, const T& value)
{
	stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
	return stream.good();
}

bool isValidForPvr(Texture::Format format, Texture::Type)
{
	std::uint64_t pixelFormat;
	return getPixelFormat(pixelFormat, format, Texture::Alpha::Standard);
}

Texture::SaveResult savePvr(const Texture& texture, const char* fileName)
{
	static_assert(sizeof(0U) == sizeof(std::uint32_t), "unexpected integer size");
	static_assert(sizeof(unsigned int) == sizeof(std::uint32_t), "unexpected integer size");

	std::uint64_t pixelFormat;
	if (!getPixelFormat(pixelFormat, texture.format(), texture.alphaType()))
		return Texture::SaveResult::Unsupported;

	std::ofstream stream(fileName, std::ofstream::binary);
	if (!stream.is_open())
		return Texture::SaveResult::WriteError;

	const std::uint32_t version = FOURCC('P', 'V', 'R', 3);
	if (!write(stream, version))
		return Texture::SaveResult::WriteError;

	std::uint32_t flags = 0;
	if (texture.alphaType() == Texture::Alpha::PreMultiplied)
		flags = 0x2;
	if (!write(stream, flags))
		return Texture::SaveResult::WriteError;

	if (!write(stream, pixelFormat))
		return Texture::SaveResult::WriteError;

	std::uint32_t colorSpace = 0;
	if (texture.colorSpace() == Texture::Color::sRGB)
		colorSpace = 1;
	if (!write(stream, colorSpace))
		return Texture::SaveResult::WriteError;

	std::uint32_t channelType = getChannelType(texture);
	if (!write(stream, channelType))
		return Texture::SaveResult::WriteError;

	if (!write(stream, texture.height()))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.width()))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.dimension() == Texture::Dimension::Dim3D ? texture.depth() : 1U))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.isArray() ? texture.depth() : 1U))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.faceCount()))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.mipLevelCount()))
		return Texture::SaveResult::WriteError;

	// Use metadata to determine if BC1 format has alpha or not.
	if (texture.format() == Texture::Format::BC1_RGB ||
		texture.format() == Texture::Format::BC1_RGBA)
	{
		if (!write(stream, static_cast<std::uint32_t>(sizeof(std::uint32_t)*3)))
			return Texture::SaveResult::WriteError;
		if (!write(stream, FOURCC('C', 'T', 'F', 'S')))
			return Texture::SaveResult::WriteError;

		std::uint32_t code;
		if (texture.format() == Texture::Format::BC1_RGBA)
			code = FOURCC('B', 'C', '1', 'A');
		else
		{
			assert(texture.format() == Texture::Format::BC1_RGB);
			code = FOURCC('B', 'C', '1', 0);
		}
		if (!write(stream, code))
			return Texture::SaveResult::WriteError;

		if (!write(stream, 0U))
			return Texture::SaveResult::WriteError;
	}
	else
	{
		if (!write(stream, 0U))
			return Texture::SaveResult::WriteError;
	}

	for (unsigned int level = 0; level < texture.mipLevelCount(); ++level)
	{
		for (unsigned int depth = 0; depth < texture.depth(level); ++depth)
		{
			for (unsigned int face = 0; face < texture.faceCount(); ++face)
			{
				stream.write(reinterpret_cast<const char*>(texture.data(
					static_cast<Texture::CubeFace>(face), level, depth)), texture.dataSize(
					static_cast<Texture::CubeFace>(face), level, depth));
				if (!stream.good())
					return Texture::SaveResult::WriteError;
			}
		}
	}

	return Texture::SaveResult::Success;
}

} // namespace cuttlefish
