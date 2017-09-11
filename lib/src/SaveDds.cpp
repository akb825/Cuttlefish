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

namespace cuttlefish
{

static const std::uint32_t magicNumber = 0x20534444;

enum DdsFlags
{
	DdsFlags_Caps = 0x1,
	DdsFlags_Height = 0x2,
	DdsFlags_Width = 0x4,
	DdsFlags_Pitch = 0x8,
	DdsFlags_PixelFormat = 0x1000,
	DdsFlags_MipmapCount = 0x20000,
	DdsFlags_LinearSize = 0x80000,
	DdsFlags_Depth = 0x800000,
	DdsFlags_Requred = DdsFlags_Caps | DdsFlags_Height | DdsFlags_Width | DdsFlags_PixelFormat
};

enum DdsFormatFlags
{
	DdsFormatFlags_AlphaPixels = 0x1,
	DdsFormatFlags_Alpha = 0x2,
	DdsFormatFlags_FourCC = 0x4,
	DdsFormatFlags_Rgb = 0x40,
	DdsFormatFlags_Yuv = 0x200,
	DdsFormatFlags_Luminance = 0x20000
};

enum DdsCapsFlags
{
	DdsCapsFlags_Complex = 0x8,
	DdsCapsFlags_Mipmap = 0x400000,
	DdsCapsFlags_Texture = 0x1000
};

enum DdsCaps2Flags
{
	DdsCaps2Flags_Cube = 0x200,
	DdsCaps2Flags_PosX = 0x400,
	DdsCaps2Flags_NegX = 0x800,
	DdsCaps2Flags_PosY = 0x1000,
	DdsCaps2Flags_NegY = 0x2000,
	DdsCaps2Flags_PosZ = 0x4000,
	DdsCaps2Flags_NegZ = 0x8000,
	DdsCaps2Flags_Volume = 0x200000
};

enum DdsDxt10Format
{
	DdsDxt10Format_UNKNOWN                     = 0,
	DdsDxt10Format_R32G32B32A32_TYPELESS       = 1,
	DdsDxt10Format_R32G32B32A32_FLOAT          = 2,
	DdsDxt10Format_R32G32B32A32_UINT           = 3,
	DdsDxt10Format_R32G32B32A32_SINT           = 4,
	DdsDxt10Format_R32G32B32_TYPELESS          = 5,
	DdsDxt10Format_R32G32B32_FLOAT             = 6,
	DdsDxt10Format_R32G32B32_UINT              = 7,
	DdsDxt10Format_R32G32B32_SINT              = 8,
	DdsDxt10Format_R16G16B16A16_TYPELESS       = 9,
	DdsDxt10Format_R16G16B16A16_FLOAT          = 10,
	DdsDxt10Format_R16G16B16A16_UNORM          = 11,
	DdsDxt10Format_R16G16B16A16_UINT           = 12,
	DdsDxt10Format_R16G16B16A16_SNORM          = 13,
	DdsDxt10Format_R16G16B16A16_SINT           = 14,
	DdsDxt10Format_R32G32_TYPELESS             = 15,
	DdsDxt10Format_R32G32_FLOAT                = 16,
	DdsDxt10Format_R32G32_UINT                 = 17,
	DdsDxt10Format_R32G32_SINT                 = 18,
	DdsDxt10Format_R32G8X24_TYPELESS           = 19,
	DdsDxt10Format_D32_FLOAT_S8X24_UINT        = 20,
	DdsDxt10Format_R32_FLOAT_X8X24_TYPELESS    = 21,
	DdsDxt10Format_X32_TYPELESS_G8X24_UINT     = 22,
	DdsDxt10Format_R10G10B10A2_TYPELESS        = 23,
	DdsDxt10Format_R10G10B10A2_UNORM           = 24,
	DdsDxt10Format_R10G10B10A2_UINT            = 25,
	DdsDxt10Format_R11G11B10_FLOAT             = 26,
	DdsDxt10Format_R8G8B8A8_TYPELESS           = 27,
	DdsDxt10Format_R8G8B8A8_UNORM              = 28,
	DdsDxt10Format_R8G8B8A8_UNORM_SRGB         = 29,
	DdsDxt10Format_R8G8B8A8_UINT               = 30,
	DdsDxt10Format_R8G8B8A8_SNORM              = 31,
	DdsDxt10Format_R8G8B8A8_SINT               = 32,
	DdsDxt10Format_R16G16_TYPELESS             = 33,
	DdsDxt10Format_R16G16_FLOAT                = 34,
	DdsDxt10Format_R16G16_UNORM                = 35,
	DdsDxt10Format_R16G16_UINT                 = 36,
	DdsDxt10Format_R16G16_SNORM                = 37,
	DdsDxt10Format_R16G16_SINT                 = 38,
	DdsDxt10Format_R32_TYPELESS                = 39,
	DdsDxt10Format_D32_FLOAT                   = 40,
	DdsDxt10Format_R32_FLOAT                   = 41,
	DdsDxt10Format_R32_UINT                    = 42,
	DdsDxt10Format_R32_SINT                    = 43,
	DdsDxt10Format_R24G8_TYPELESS              = 44,
	DdsDxt10Format_D24_UNORM_S8_UINT           = 45,
	DdsDxt10Format_R24_UNORM_X8_TYPELESS       = 46,
	DdsDxt10Format_X24_TYPELESS_G8_UINT        = 47,
	DdsDxt10Format_R8G8_TYPELESS               = 48,
	DdsDxt10Format_R8G8_UNORM                  = 49,
	DdsDxt10Format_R8G8_UINT                   = 50,
	DdsDxt10Format_R8G8_SNORM                  = 51,
	DdsDxt10Format_R8G8_SINT                   = 52,
	DdsDxt10Format_R16_TYPELESS                = 53,
	DdsDxt10Format_R16_FLOAT                   = 54,
	DdsDxt10Format_D16_UNORM                   = 55,
	DdsDxt10Format_R16_UNORM                   = 56,
	DdsDxt10Format_R16_UINT                    = 57,
	DdsDxt10Format_R16_SNORM                   = 58,
	DdsDxt10Format_R16_SINT                    = 59,
	DdsDxt10Format_R8_TYPELESS                 = 60,
	DdsDxt10Format_R8_UNORM                    = 61,
	DdsDxt10Format_R8_UINT                     = 62,
	DdsDxt10Format_R8_SNORM                    = 63,
	DdsDxt10Format_R8_SINT                     = 64,
	DdsDxt10Format_A8_UNORM                    = 65,
	DdsDxt10Format_R1_UNORM                    = 66,
	DdsDxt10Format_R9G9B9E5_SHAREDEXP          = 67,
	DdsDxt10Format_R8G8_B8G8_UNORM             = 68,
	DdsDxt10Format_G8R8_G8B8_UNORM             = 69,
	DdsDxt10Format_BC1_TYPELESS                = 70,
	DdsDxt10Format_BC1_UNORM                   = 71,
	DdsDxt10Format_BC1_UNORM_SRGB              = 72,
	DdsDxt10Format_BC2_TYPELESS                = 73,
	DdsDxt10Format_BC2_UNORM                   = 74,
	DdsDxt10Format_BC2_UNORM_SRGB              = 75,
	DdsDxt10Format_BC3_TYPELESS                = 76,
	DdsDxt10Format_BC3_UNORM                   = 77,
	DdsDxt10Format_BC3_UNORM_SRGB              = 78,
	DdsDxt10Format_BC4_TYPELESS                = 79,
	DdsDxt10Format_BC4_UNORM                   = 80,
	DdsDxt10Format_BC4_SNORM                   = 81,
	DdsDxt10Format_BC5_TYPELESS                = 82,
	DdsDxt10Format_BC5_UNORM                   = 83,
	DdsDxt10Format_BC5_SNORM                   = 84,
	DdsDxt10Format_B5G6R5_UNORM                = 85,
	DdsDxt10Format_B5G5R5A1_UNORM              = 86,
	DdsDxt10Format_B8G8R8A8_UNORM              = 87,
	DdsDxt10Format_B8G8R8X8_UNORM              = 88,
	DdsDxt10Format_R10G10B10_XR_BIAS_A2_UNORM  = 89,
	DdsDxt10Format_B8G8R8A8_TYPELESS           = 90,
	DdsDxt10Format_B8G8R8A8_UNORM_SRGB         = 91,
	DdsDxt10Format_B8G8R8X8_TYPELESS           = 92,
	DdsDxt10Format_B8G8R8X8_UNORM_SRGB         = 93,
	DdsDxt10Format_BC6H_TYPELESS               = 94,
	DdsDxt10Format_BC6H_UF16                   = 95,
	DdsDxt10Format_BC6H_SF16                   = 96,
	DdsDxt10Format_BC7_TYPELESS                = 97,
	DdsDxt10Format_BC7_UNORM                   = 98,
	DdsDxt10Format_BC7_UNORM_SRGB              = 99,
	DdsDxt10Format_AYUV                        = 100,
	DdsDxt10Format_Y410                        = 101,
	DdsDxt10Format_Y416                        = 102,
	DdsDxt10Format_NV12                        = 103,
	DdsDxt10Format_P010                        = 104,
	DdsDxt10Format_P016                        = 105,
	DdsDxt10Format_420_OPAQUE                  = 106,
	DdsDxt10Format_YUY2                        = 107,
	DdsDxt10Format_Y210                        = 108,
	DdsDxt10Format_Y216                        = 109,
	DdsDxt10Format_NV11                        = 110,
	DdsDxt10Format_AI44                        = 111,
	DdsDxt10Format_IA44                        = 112,
	DdsDxt10Format_P8                          = 113,
	DdsDxt10Format_A8P8                        = 114,
	DdsDxt10Format_B4G4R4A4_UNORM              = 115,
	DdsDxt10Format_P208                        = 130,
	DdsDxt10Format_V208                        = 131,
	DdsDxt10Format_V408                        = 132,
};

enum DdsTextureDim
{
  DdsTextureDim_UNKNOWN    = 0,
  DdsTextureDim_BUFFER     = 1,
  DdsTextureDim_TEXTURE1D  = 2,
  DdsTextureDim_TEXTURE2D  = 3,
  DdsTextureDim_TEXTURE3D  = 4
};

enum DdsDxt10MiscFlag
{
	DdsDxt10MiscFlag_CubeMap = 0x4
};

enum DdsDxt10MiscFlags2
{
	DdsDxt10MiscFlags2_AlphaModeUnknown       = 0,
	DdsDxt10MiscFlags2_AlphaModeStraight      = 1,
	DdsDxt10MiscFlags2_AlphaModePreMultiplied = 2,
	DdsDxt10MiscFlags2_AlphaModeOpaque        = 3,
	DdsDxt10MiscFlags2_AlphaModeCustom        = 4,
};

struct DdsPixelFormat
{
	std::uint32_t size;
	std::uint32_t flags;
	std::uint32_t fourCC;
	std::uint32_t rgbBitCount;
	std::uint32_t rBitMask;
	std::uint32_t gBitMask;
	std::uint32_t bBitMask;
	std::uint32_t aBitMask;
};

struct DdsHeader
{
	std::uint32_t size;
	std::uint32_t flags;
	std::uint32_t height;
	std::uint32_t width;
	std::uint32_t pitchOrLinearSize;
	std::uint32_t depth;
	std::uint32_t mipMapCount;
	std::uint32_t reserved1[11];
	DdsPixelFormat ddspf;
	std::uint32_t caps;
	std::uint32_t caps2;
	std::uint32_t caps3;
	std::uint32_t caps4;
	std::uint32_t reserved2;
};

struct DdsHeaderDxt10
{
	std::uint32_t dxgiFormat;
	std::uint32_t resourceDimension;
	std::uint32_t miscFlag;
	std::uint32_t arraySize;
	std::uint32_t miscFlags2;
};

template <typename T>
static bool write(std::ofstream& stream, const T& value)
{
	stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
	return stream.good();
}

static DdsDxt10Format getDdsFormat(Texture::Format format, Texture::Type type,
	Texture::Color colorSpace)
{
	switch (format)
	{
		case Texture::Format::R4G4:
			if (type == Texture::Type::UNorm)
				return DdsDxt10Format_IA44;
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::A4R4G4B4:
			if (type == Texture::Type::UNorm)
				return DdsDxt10Format_B4G4R4A4_UNORM;
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::R5G6B5:
			if (type == Texture::Type::UNorm)
				return DdsDxt10Format_B5G6R5_UNORM;
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::A1R5G5B5:
			if (type == Texture::Type::UNorm)
				return DdsDxt10Format_B5G5R5A1_UNORM;
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::R8:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_R8_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_R8_SNORM;
				case Texture::Type::UInt:
					return DdsDxt10Format_R8_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R8_SINT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R8G8:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_R8G8_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_R8G8_SNORM;
				case Texture::Type::UInt:
					return DdsDxt10Format_R8G8_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R8G8_SINT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R8G8B8A8:
			switch (type)
			{
				case Texture::Type::UNorm:
					if (colorSpace == Texture::Color::sRGB)
						return DdsDxt10Format_R8G8B8A8_UNORM_SRGB;
					return DdsDxt10Format_R8G8B8A8_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_R8G8B8A8_SNORM;
				case Texture::Type::UInt:
					return DdsDxt10Format_R8G8B8A8_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R8G8B8A8_SINT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::B8G8R8A8:
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == Texture::Color::sRGB)
					return DdsDxt10Format_B8G8R8A8_UNORM_SRGB;
				return DdsDxt10Format_B8G8R8A8_UNORM;
			}
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::A2B10G10R10:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_R10G10B10A2_UNORM;
				case Texture::Type::UInt:
					return DdsDxt10Format_R10G10B10A2_UINT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R16:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_R16_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_R16_SNORM;
				case Texture::Type::UInt:
					return DdsDxt10Format_R16_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R16_SINT;
				case Texture::Type::Float:
					return DdsDxt10Format_R16_FLOAT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R16G16:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_R16G16_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_R16G16_SNORM;
				case Texture::Type::UInt:
					return DdsDxt10Format_R16G16_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R16G16_SINT;
				case Texture::Type::Float:
					return DdsDxt10Format_R16G16_FLOAT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R16G16B16A16:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_R16G16B16A16_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_R16G16B16A16_SNORM;
				case Texture::Type::UInt:
					return DdsDxt10Format_R16G16B16A16_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R16G16B16A16_SINT;
				case Texture::Type::Float:
					return DdsDxt10Format_R16G16B16A16_FLOAT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R32:
			switch (type)
			{
				case Texture::Type::UInt:
					return DdsDxt10Format_R32_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R32_SINT;
				case Texture::Type::Float:
					return DdsDxt10Format_R32_FLOAT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R32G32:
			switch (type)
			{
				case Texture::Type::UInt:
					return DdsDxt10Format_R32G32_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R32G32_SINT;
				case Texture::Type::Float:
					return DdsDxt10Format_R32G32_FLOAT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R32G32B32:
			switch (type)
			{
				case Texture::Type::UInt:
					return DdsDxt10Format_R32G32B32_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R32G32B32_SINT;
				case Texture::Type::Float:
					return DdsDxt10Format_R32G32B32_FLOAT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::R32G32B32A32:
			switch (type)
			{
				case Texture::Type::UInt:
					return DdsDxt10Format_R32G32B32A32_UINT;
				case Texture::Type::Int:
					return DdsDxt10Format_R32G32B32A32_SINT;
				case Texture::Type::Float:
					return DdsDxt10Format_R32G32B32A32_FLOAT;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
		case Texture::Format::B10G11R11_UFloat:
			if (type == Texture::Type::UFloat)
				return DdsDxt10Format_R11G11B10_FLOAT;
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::E5B9G9R9_UFloat:
			if (type == Texture::Type::UFloat)
				return DdsDxt10Format_R9G9B9E5_SHAREDEXP;
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::BC1_RGB:
		case Texture::Format::BC1_RGBA:
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == Texture::Color::sRGB)
					return DdsDxt10Format_BC1_UNORM_SRGB;
				return DdsDxt10Format_BC1_UNORM;
			}
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::BC2:
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == Texture::Color::sRGB)
					return DdsDxt10Format_BC2_UNORM_SRGB;
				return DdsDxt10Format_BC2_UNORM;
			}
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::BC3:
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == Texture::Color::sRGB)
					return DdsDxt10Format_BC3_UNORM_SRGB;
				return DdsDxt10Format_BC3_UNORM;
			}
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::BC4:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_BC4_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_BC4_SNORM;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::BC5:
			switch (type)
			{
				case Texture::Type::UNorm:
					return DdsDxt10Format_BC5_UNORM;
				case Texture::Type::SNorm:
					return DdsDxt10Format_BC5_SNORM;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::BC6H:
			switch (type)
			{
				case Texture::Type::UFloat:
					return DdsDxt10Format_BC6H_UF16;
				case Texture::Type::Float:
					return DdsDxt10Format_BC6H_SF16;
				default:
					return DdsDxt10Format_UNKNOWN;
			}
			return DdsDxt10Format_UNKNOWN;
		case Texture::Format::BC7:
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == Texture::Color::sRGB)
					return DdsDxt10Format_BC7_UNORM_SRGB;
				return DdsDxt10Format_BC7_UNORM;
			}
			return DdsDxt10Format_UNKNOWN;

		// Unsupported by DirectX (and therefore DDS)
		case Texture::Format::Unknown:
		case Texture::Format::R4G4B4A4:
		case Texture::Format::B4G4R4A4:
		case Texture::Format::B5G6R5:
		case Texture::Format::R5G5B5A1:
		case Texture::Format::B5G5R5A1:
		case Texture::Format::R8G8B8:
		case Texture::Format::B8G8R8:
		case Texture::Format::A8B8G8R8:
		case Texture::Format::A2R10G10B10:
		case Texture::Format::R16G16B16:
		case Texture::Format::ETC1:
		case Texture::Format::ETC2_R8G8B8:
		case Texture::Format::ETC2_R8G8B8A1:
		case Texture::Format::ETC2_R8G8B8A8:
		case Texture::Format::EAC_R11:
		case Texture::Format::EAC_R11G11:
		case Texture::Format::ASTC_4x4:
		case Texture::Format::ASTC_5x4:
		case Texture::Format::ASTC_5x5:
		case Texture::Format::ASTC_6x5:
		case Texture::Format::ASTC_6x6:
		case Texture::Format::ASTC_8x5:
		case Texture::Format::ASTC_8x6:
		case Texture::Format::ASTC_8x8:
		case Texture::Format::ASTC_10x5:
		case Texture::Format::ASTC_10x6:
		case Texture::Format::ASTC_10x8:
		case Texture::Format::ASTC_10x10:
		case Texture::Format::ASTC_12x10:
		case Texture::Format::ASTC_12x12:
		case Texture::Format::PVRTC1_RGB_2BPP:
		case Texture::Format::PVRTC1_RGBA_2BPP:
		case Texture::Format::PVRTC1_RGB_4BPP:
		case Texture::Format::PVRTC1_RGBA_4BPP:
		case Texture::Format::PVRTC2_RGBA_2BPP:
		case Texture::Format::PVRTC2_RGBA_4BPP:
			return DdsDxt10Format_UNKNOWN;
	}
	assert(false);
	return DdsDxt10Format_UNKNOWN;
}

static std::uint32_t computePitch(const Texture& texture)
{
	unsigned int blockWidth = Texture::blockWidth(texture.format());
	unsigned int blockSize = Texture::blockSize(texture.format());
	return (texture.width() + blockWidth - 1)/blockWidth*blockSize;
}

bool isValidForDds(Texture::Format format, Texture::Type type)
{
	return getDdsFormat(format, type, Texture::Color::Linear) != DdsDxt10Format_UNKNOWN;
}

Texture::SaveResult saveDds(const Texture& texture, const char* fileName)
{
	DdsDxt10Format ddsFormat = getDdsFormat(texture.format(), texture.type(),
		texture.colorSpace());
	if (ddsFormat == DdsDxt10Format_UNKNOWN)
		return Texture::SaveResult::Unsupported;

	std::ofstream stream(fileName, std::ofstream::binary);
	if (!stream.is_open())
		return Texture::SaveResult::WriteError;

	if (!write(stream, magicNumber))
		return Texture::SaveResult::WriteError;

	DdsHeader header;
	std::memset(&header, 0, sizeof(header));
	header.size = sizeof(DdsHeader);
	header.flags = DdsFlags_Requred | DdsFlags_MipmapCount | DdsFlags_Pitch;
	if (texture.dimension() == Texture::Dimension::Dim3D)
		header.flags |= DdsFlags_Depth;
	header.height = texture.height();
	header.width = texture.width();
	header.pitchOrLinearSize = computePitch(texture);
	header.depth = texture.dimension() == Texture::Dimension::Dim3D ? texture.depth() : 0;
	header.mipMapCount = texture.mipLevelCount();

	header.ddspf.size = sizeof(header.ddspf);
	header.ddspf.flags = DdsFormatFlags_FourCC;
	header.ddspf.fourCC = FOURCC('D', 'X', '1', '0');

	header.caps = DdsCapsFlags_Texture;
	if (texture.mipLevelCount() > 1)
		header.caps |= DdsCapsFlags_Mipmap;
	if (texture.mipLevelCount() > 1 || texture.dimension() == Texture::Dimension::Dim3D ||
		texture.dimension() == Texture::Dimension::Dim3D || texture.isArray())
	{
		header.caps |= DdsCapsFlags_Complex;
	}

	if (texture.dimension() == Texture::Dimension::Cube)
	{
		header.caps2 = DdsCaps2Flags_Cube | DdsCaps2Flags_PosX | DdsCaps2Flags_NegX |
			DdsCaps2Flags_PosY | DdsCaps2Flags_NegY | DdsCaps2Flags_PosZ | DdsCaps2Flags_NegZ;
	}
	else if (texture.dimension() == Texture::Dimension::Dim3D)
		header.caps2 = DdsCaps2Flags_Volume;

	if (!write(stream, header))
		return Texture::SaveResult::WriteError;

	DdsHeaderDxt10 dxt10Header;
	std::memset(&dxt10Header, 0, sizeof(dxt10Header));
	dxt10Header.dxgiFormat = ddsFormat;
	switch (texture.dimension())
	{
		case Texture::Dimension::Dim1D:
			dxt10Header.resourceDimension = DdsTextureDim_TEXTURE1D;
			break;
		case Texture::Dimension::Dim2D:
			dxt10Header.resourceDimension = DdsTextureDim_TEXTURE2D;
			break;
		case Texture::Dimension::Dim3D:
			dxt10Header.resourceDimension = DdsTextureDim_TEXTURE3D;
			break;
		case Texture::Dimension::Cube:
			dxt10Header.resourceDimension = DdsTextureDim_TEXTURE2D;
			dxt10Header.miscFlag = DdsDxt10MiscFlag_CubeMap;
			break;
		default:
			assert(false);
			break;
	}
	dxt10Header.arraySize = texture.dimension() == Texture::Dimension::Dim3D ? 1 : texture.depth();
	if (Texture::hasAlpha(texture.format()))
	{
		switch (texture.alphaType())
		{
			case Texture::Alpha::None:
				dxt10Header.miscFlags2 = DdsDxt10MiscFlags2_AlphaModeOpaque;
				break;
			case Texture::Alpha::Standard:
				dxt10Header.miscFlags2 = DdsDxt10MiscFlags2_AlphaModeStraight;
				break;
			case Texture::Alpha::PreMultiplied:
				dxt10Header.miscFlags2 = DdsDxt10MiscFlags2_AlphaModePreMultiplied;
				break;
			case Texture::Alpha::Encoded:
				dxt10Header.miscFlags2 = DdsDxt10MiscFlags2_AlphaModeCustom;
				break;
		}
	}
	else
		dxt10Header.miscFlags2 = DdsDxt10MiscFlags2_AlphaModeOpaque;
	if (!write(stream, dxt10Header))
		return Texture::SaveResult::WriteError;

	unsigned int elements = texture.isArray() ? texture.depth() : 1;
	for (unsigned int element = 0; element < elements; ++element)
	{
		for (unsigned int face = 0; face < texture.faceCount(); ++face)
		{
			auto faceEnum = static_cast<Texture::CubeFace>(face);
			for (unsigned int level = 0; level < texture.mipLevelCount(); ++level)
			{
				unsigned int volumes = texture.dimension() == Texture::Dimension::Dim3D ?
					texture.depth(level) : 1;
				for (unsigned int volume = 0; volume < volumes; ++volume)
				{
					assert(element == 0 || volume == 0);
					unsigned int index = volume + element;
					assert(texture.dataSize(faceEnum, level, index) > 0);
					stream.write(
						reinterpret_cast<const char*>(texture.data(faceEnum, level, index)),
						texture.dataSize(faceEnum, level, index));
					if (!stream.good())
						return Texture::SaveResult::WriteError;
				}
			}
		}
	}

	return Texture::SaveResult::Success;
}

} // namespace cuttlefish
