/*
 * Copyright 2017-2022 Aaron Barany
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

#include "SaveKtx.h"
#include "Shared.h"
#include <cassert>
#include <cstring>
#include <ostream>

#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_HALF_FLOAT                     0x140B
#define GL_RED                            0x1903
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGB16                          0x8054
#define GL_RGBA16                         0x805B
#define GL_RGB8                           0x8051
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_RG                             0x8227
#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT 0x8A54
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT 0x8A55
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT 0x8A56
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT 0x8A57
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#define GL_R11F_G11F_B10F                 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
#define GL_RGB9_E5                        0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV       0x8C3E
#define GL_SRGB8                          0x8C41
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define GL_RGB565                         0x8D62
#define GL_ETC1_RGB8_OES                  0x8D64
#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I                          0x8D8F
#define GL_RED_INTEGER                    0x8D94
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99
#define GL_BGR_INTEGER                    0x8D9A
#define GL_BGRA_INTEGER                   0x8D9B
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_COMPRESSED_RED_RGTC1           0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
#define GL_COMPRESSED_RG_RGTC2            0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE
#define GL_COMPRESSED_RGBA_BPTC_UNORM     0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B
#define GL_RGB10_A2UI                     0x906F
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG 0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG 0x9138
#define GL_COMPRESSED_R11_EAC             0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC      0x9271
#define GL_COMPRESSED_RG11_EAC            0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC     0x9273
#define GL_COMPRESSED_RGB8_ETC2           0x9274
#define GL_COMPRESSED_SRGB8_ETC2          0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC      0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR   0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR   0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR   0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR   0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR   0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR   0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR   0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR   0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR  0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR  0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR  0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR 0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR 0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR 0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR 0x93DD
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG 0x93F0
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG 0x93F1

namespace cuttlefish
{

struct FormatInfo
{
	std::uint32_t type;
	std::uint32_t typeSize;
	std::uint32_t format;
	std::uint32_t internalFormat;
	std::uint32_t baseInternalFormat;
};

static char header[12] =
{
	'\xAB', 'K', 'T', 'X', ' ', '1', '1', '\xBB', '\r', '\n', '\x1A', '\n'
};

static const std::uint32_t endianness = 0x04030201;

static bool getFormatInfo(FormatInfo& info, Texture::Format format, Texture::Type type,
	ColorSpace colorSpace)
{
	switch (format)
	{
		case Texture::Format::R4G4B4A4:
			info.type = GL_UNSIGNED_SHORT_4_4_4_4;
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_RGBA4;
				return true;
			}
			return false;
		case Texture::Format::B4G4R4A4:
			info.type = GL_UNSIGNED_SHORT_4_4_4_4;
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_BGRA;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_RGBA4;
				return true;
			}
			return false;
		case Texture::Format::R5G6B5:
			info.type = GL_UNSIGNED_SHORT_5_6_5;
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_RGB565;
				return true;
			}
			return false;
		case Texture::Format::B5G6R5:
			info.type = GL_UNSIGNED_SHORT_5_6_5_REV;
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_RGB565;
				return true;
			}
			return false;
		case Texture::Format::R5G5B5A1:
			info.type = GL_UNSIGNED_SHORT_5_5_5_1;
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_RGB5_A1;
				return true;
			}
			return false;
		case Texture::Format::B5G5R5A1:
			info.type = GL_UNSIGNED_SHORT_5_5_5_1;
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_BGRA;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_RGB5_A1;
				return true;
			}
			return false;
		case Texture::Format::A1R5G5B5:
			info.type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_BGRA;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_RGB5_A1;
				return true;
			}
			return false;
		case Texture::Format::R8:
			info.typeSize = 1;
			info.format = GL_RED;
			info.baseInternalFormat = GL_LUMINANCE;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.type = GL_UNSIGNED_BYTE;
					info.internalFormat = GL_R8;
					return true;
				case Texture::Type::SNorm:
					info.type = GL_BYTE;
					info.internalFormat = GL_R8_SNORM;
					return true;
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_BYTE;
					info.internalFormat = GL_R8UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_BYTE;
					info.internalFormat = GL_R8I;
					return true;
				default:
					return false;
			}
		case Texture::Format::R8G8:
			info.type = GL_UNSIGNED_BYTE;
			info.typeSize = 1;
			info.format = GL_RG;
			info.baseInternalFormat = GL_LUMINANCE_ALPHA;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.internalFormat = GL_RG8;
					return true;
				case Texture::Type::SNorm:
					info.internalFormat = GL_RG8_SNORM;
					return true;
				case Texture::Type::UInt:
					info.internalFormat = GL_RG8UI;
					return true;
				case Texture::Type::Int:
					info.internalFormat = GL_RG8I;
					return true;
				default:
					return false;
			}
		case Texture::Format::R8G8B8:
			info.typeSize = 1;
			info.format = info.baseInternalFormat = GL_RGB;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.type = GL_UNSIGNED_BYTE;
					if (colorSpace == ColorSpace::sRGB)
						info.internalFormat = GL_SRGB8;
					else
						info.internalFormat = GL_RGB8;
					return true;
				case Texture::Type::SNorm:
					info.type = GL_BYTE;
					info.internalFormat = GL_RGB8_SNORM;
					return true;
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_BYTE;
					info.internalFormat = GL_RGB8UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_BYTE;
					info.internalFormat = GL_RGB8I;
					return true;
				default:
					return false;
			}
		case Texture::Format::R8G8B8A8:
			info.typeSize = 1;
			info.baseInternalFormat = GL_RGBA;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.type = GL_UNSIGNED_BYTE;
					if (colorSpace == ColorSpace::sRGB)
						info.internalFormat = GL_SRGB8_ALPHA8;
					else
						info.internalFormat = GL_RGBA8;
					info.format = GL_RGBA;
					return true;
				case Texture::Type::SNorm:
					info.type = GL_BYTE;
					info.internalFormat = GL_RGBA8_SNORM;
					info.format = GL_RGBA;
					return true;
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_BYTE;
					info.internalFormat = GL_RGBA8UI;
					info.format = GL_RGBA_INTEGER;
					return true;
				case Texture::Type::Int:
					info.type = GL_BYTE;
					info.internalFormat = GL_RGBA8I;
					info.format = GL_RGBA_INTEGER;
					return true;
				default:
					return false;
			}
		case Texture::Format::B8G8R8A8:
			info.type = GL_UNSIGNED_INT_8_8_8_8;
			info.typeSize = 4;
			info.baseInternalFormat = GL_BGRA;
			switch (type)
			{
				case Texture::Type::UNorm:
					if (colorSpace == ColorSpace::sRGB)
						info.internalFormat = GL_SRGB8_ALPHA8;
					else
						info.internalFormat = GL_RGBA8;
					info.format = GL_BGRA;
					return true;
				case Texture::Type::SNorm:
					info.internalFormat = GL_RGBA8_SNORM;
					info.format = GL_BGRA;
					return true;
				case Texture::Type::UInt:
					info.internalFormat = GL_RGBA8UI;
					info.format = GL_BGRA_INTEGER;
					return true;
				case Texture::Type::Int:
					info.internalFormat = GL_RGBA8I;
					return true;
				default:
					return false;
			}
		case Texture::Format::A8B8G8R8:
			info.type = GL_UNSIGNED_INT_8_8_8_8_REV;
			info.typeSize = 4;
			info.baseInternalFormat = GL_RGBA;
			switch (type)
			{
				case Texture::Type::UNorm:
					if (colorSpace == ColorSpace::sRGB)
						info.internalFormat = GL_SRGB8_ALPHA8;
					else
						info.internalFormat = GL_RGBA8;
					info.format = GL_RGBA;
					return true;
				case Texture::Type::SNorm:
					info.internalFormat = GL_RGBA8_SNORM;
					info.format = GL_RGBA;
					return true;
				case Texture::Type::UInt:
					info.internalFormat = GL_RGBA8UI;
					info.format = GL_RGBA_INTEGER;
					return true;
				case Texture::Type::Int:
					info.internalFormat = GL_RGBA8I;
					info.format = GL_RGBA_INTEGER;
					return true;
				default:
					return false;
			}
		case Texture::Format::A2R10G10B10:
			info.type = GL_UNSIGNED_INT_2_10_10_10_REV;
			info.typeSize = 4;
			info.baseInternalFormat = GL_BGRA;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.internalFormat = GL_RGB10_A2;
					info.format = GL_BGRA;
					return true;
				case Texture::Type::UInt:
					info.internalFormat = GL_RGB10_A2UI;
					info.format = GL_BGRA_INTEGER;
					return true;
				default:
					return false;
			}
		case Texture::Format::A2B10G10R10:
			info.type = GL_UNSIGNED_INT_2_10_10_10_REV;
			info.typeSize = 4;
			info.baseInternalFormat = GL_RGBA;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.internalFormat = GL_RGB10_A2;
					info.format = GL_RGBA;
					return true;
				case Texture::Type::UInt:
					info.internalFormat = GL_RGB10_A2UI;
					info.format = GL_RGBA_INTEGER;
					return true;
				default:
					return false;
			}
		case Texture::Format::R16:
			info.typeSize = 2;
			info.format = GL_RED;
			info.baseInternalFormat = GL_LUMINANCE;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_R16;
					return true;
				case Texture::Type::SNorm:
					info.type = GL_SHORT;
					info.internalFormat = GL_R16_SNORM;
					return true;
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_R16UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_SHORT;
					info.internalFormat = GL_R16I;
					return true;
				case Texture::Type::Float:
					info.type = GL_HALF_FLOAT;
					info.internalFormat = GL_R16F;
					return true;
				default:
					return false;
			}
		case Texture::Format::R16G16:
			info.typeSize = 2;
			info.format = GL_RG;
			info.baseInternalFormat = GL_LUMINANCE_ALPHA;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_RG16;
					return true;
				case Texture::Type::SNorm:
					info.type = GL_SHORT;
					info.internalFormat = GL_RG16_SNORM;
					return true;
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_RG16UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_SHORT;
					info.internalFormat = GL_RG16I;
					return true;
				case Texture::Type::Float:
					info.type = GL_HALF_FLOAT;
					info.internalFormat = GL_RG16F;
					return true;
				default:
					return false;
			}
		case Texture::Format::R16G16B16:
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_RGB;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_RGB16;
					return true;
				case Texture::Type::SNorm:
					info.type = GL_SHORT;
					info.internalFormat = GL_RGB16_SNORM;
					return true;
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_RGB16UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_SHORT;
					info.internalFormat = GL_RGB16I;
					return true;
				case Texture::Type::Float:
					info.type = GL_HALF_FLOAT;
					info.internalFormat = GL_RGB16F;
					return true;
				default:
					return false;
			}
		case Texture::Format::R16G16B16A16:
			info.typeSize = 2;
			info.format = info.baseInternalFormat = GL_RGBA;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_RGBA16;
					return true;
				case Texture::Type::SNorm:
					info.type = GL_SHORT;
					info.internalFormat = GL_RGBA16_SNORM;
					return true;
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_SHORT;
					info.internalFormat = GL_RGBA16UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_SHORT;
					info.internalFormat = GL_RGBA16I;
					return true;
				case Texture::Type::Float:
					info.type = GL_HALF_FLOAT;
					info.internalFormat = GL_RGBA16F;
					return true;
				default:
					return false;
			}
		case Texture::Format::R32:
			info.typeSize = 4;
			info.format = GL_RED;
			info.baseInternalFormat = GL_LUMINANCE;
			switch (type)
			{
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_INT;
					info.internalFormat = GL_R32UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_INT;
					info.internalFormat = GL_R32I;
					return true;
				case Texture::Type::Float:
					info.type = GL_FLOAT;
					info.internalFormat = GL_R32F;
					return true;
				default:
					return false;
			}
		case Texture::Format::R32G32:
			info.typeSize = 4;
			info.format = GL_RG;
			info.baseInternalFormat = GL_LUMINANCE_ALPHA;
			switch (type)
			{
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_INT;
					info.internalFormat = GL_RG32UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_INT;
					info.internalFormat = GL_RG32I;
					return true;
				case Texture::Type::Float:
					info.type = GL_FLOAT;
					info.internalFormat = GL_RG32F;
					return true;
				default:
					return false;
			}
		case Texture::Format::R32G32B32:
			info.typeSize = 4;
			info.format = info.baseInternalFormat = GL_RGB;
			switch (type)
			{
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_INT;
					info.internalFormat = GL_RGB32UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_INT;
					info.internalFormat = GL_RGB32I;
					return true;
				case Texture::Type::Float:
					info.type = GL_FLOAT;
					info.internalFormat = GL_RGB32F;
					return true;
				default:
					return false;
			}
		case Texture::Format::R32G32B32A32:
			info.typeSize = 4;
			info.format = info.baseInternalFormat = GL_RGBA;
			switch (type)
			{
				case Texture::Type::UInt:
					info.type = GL_UNSIGNED_INT;
					info.internalFormat = GL_RGBA32UI;
					return true;
				case Texture::Type::Int:
					info.type = GL_INT;
					info.internalFormat = GL_RGBA32I;
					return true;
				case Texture::Type::Float:
					info.type = GL_FLOAT;
					info.internalFormat = GL_RGBA32F;
					return true;
				default:
					return false;
			}

		// Special formats.
		case Texture::Format::B10G11R11_UFloat:
			info.type = GL_UNSIGNED_INT_10F_11F_11F_REV;
			info.typeSize = 4;
			info.format = info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UFloat)
			{
				info.internalFormat = GL_R11F_G11F_B10F;
				return true;
			}
			return false;
		case Texture::Format::E5B9G9R9_UFloat:
			info.type = GL_UNSIGNED_INT_5_9_9_9_REV;
			info.typeSize = 4;
			info.format = info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UFloat)
			{
				info.internalFormat = GL_RGB9_E5;
				return true;
			}
			return false;

		// Compressed formats.
		case Texture::Format::BC1_RGB:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				return true;
			}
			return false;
		case Texture::Format::BC1_RGBA:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				return true;
			}
			return false;
		case Texture::Format::BC2:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				return true;
			}
			return false;
		case Texture::Format::BC3:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				return true;
			}
			return false;
		case Texture::Format::BC4:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RED;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.internalFormat = GL_COMPRESSED_RED_RGTC1;
					return true;
				case Texture::Type::SNorm:
					info.internalFormat = GL_COMPRESSED_SIGNED_RED_RGTC1;
					return true;
				default:
					return false;
			}
		case Texture::Format::BC5:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RG;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.internalFormat = GL_COMPRESSED_RG_RGTC2;
					return true;
				case Texture::Type::SNorm:
					info.internalFormat = GL_COMPRESSED_SIGNED_RG_RGTC2;
					return true;
				default:
					return false;
			}
		case Texture::Format::BC6H:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGB;
			switch (type)
			{
				case Texture::Type::UFloat:
					info.internalFormat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
					return true;
				case Texture::Type::Float:
					info.internalFormat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
					return true;
				default:
					return false;
			}
		case Texture::Format::BC7:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
				return true;
			}
			return false;
		case Texture::Format::ETC1:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UNorm)
			{
				info.internalFormat = GL_ETC1_RGB8_OES;
				return true;
			}
			return false;
		case Texture::Format::ETC2_R8G8B8:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ETC2;
				else
					info.internalFormat = GL_COMPRESSED_RGB8_ETC2;
				return true;
			}
			return false;
		case Texture::Format::ETC2_R8G8B8A1:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
				else
					info.internalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
				return true;
			}
			return false;
		case Texture::Format::ETC2_R8G8B8A8:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
				else
					info.internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
				return true;
			}
			return false;
		case Texture::Format::EAC_R11:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RED;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.internalFormat = GL_COMPRESSED_R11_EAC;
					return true;
				case Texture::Type::SNorm:
					info.internalFormat = GL_COMPRESSED_SIGNED_R11_EAC;
					return true;
				default:
					return false;
			}
		case Texture::Format::EAC_R11G11:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RG;
			switch (type)
			{
				case Texture::Type::UNorm:
					info.internalFormat = GL_COMPRESSED_RG11_EAC;
					return true;
				case Texture::Type::SNorm:
					info.internalFormat = GL_COMPRESSED_SIGNED_RG11_EAC;
					return true;
				default:
					return false;
			}
		case Texture::Format::ASTC_4x4:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_5x4:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_5x5:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_6x5:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_6x6:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_8x5:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_8x6:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_8x8:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_10x5:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_10x6:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_10x8:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_10x10:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_12x10:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
				return true;
			}
			return false;
		case Texture::Format::ASTC_12x12:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm || type == Texture::Type::UFloat)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
				return true;
			}
			return false;
		case Texture::Format::PVRTC1_RGB_2BPP:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
				return true;
			}
			return false;
		case Texture::Format::PVRTC1_RGBA_2BPP:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
				return true;
			}
			return false;
		case Texture::Format::PVRTC1_RGB_4BPP:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGB;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
				return true;
			}
			return false;
		case Texture::Format::PVRTC1_RGBA_4BPP:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
				return true;
			}
			return false;
		case Texture::Format::PVRTC2_RGBA_2BPP:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
				return true;
			}
			return false;
		case Texture::Format::PVRTC2_RGBA_4BPP:
			info.type = 0;
			info.typeSize = 1;
			info.format = 0;
			info.baseInternalFormat = GL_RGBA;
			if (type == Texture::Type::UNorm)
			{
				if (colorSpace == ColorSpace::sRGB)
					info.internalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG;
				else
					info.internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
				return true;
			}
			return false;

		case Texture::Format::Unknown:
		case Texture::Format::R4G4:
		case Texture::Format::A4R4G4B4:
		case Texture::Format::B8G8R8:
			return false;
	}
	assert(false);
	return false;
}

bool isValidForKtx(Texture::Format format, Texture::Type type)
{
	FormatInfo info;
	return getFormatInfo(info, format, type, ColorSpace::Linear);
}

Texture::SaveResult saveKtx(const Texture& texture, std::ostream& stream)
{
	static_assert(sizeof(0U) == sizeof(std::uint32_t), "unexpected integer size");
	static_assert(sizeof(unsigned int) == sizeof(std::uint32_t), "unexpected integer size");

	FormatInfo info;
	if (!getFormatInfo(info, texture.format(), texture.type(), texture.colorSpace()))
		return Texture::SaveResult::Unsupported;

	if (!write(stream, header))
		return Texture::SaveResult::WriteError;

	if (!write(stream, endianness))
		return Texture::SaveResult::WriteError;

	if (!write(stream, info))
		return Texture::SaveResult::WriteError;

	if (!write(stream, texture.width()))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.dimension() == Texture::Dimension::Dim1D ? 0U : texture.height()))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.dimension() == Texture::Dimension::Dim3D ? texture.depth() : 0U))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.isArray() ? texture.depth() : 0U))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.faceCount()))
		return Texture::SaveResult::WriteError;
	if (!write(stream, texture.mipLevelCount()))
		return Texture::SaveResult::WriteError;
	if (!write(stream, 0U))
		return Texture::SaveResult::WriteError;

	bool compressed = Texture::blockWidth(texture.format()) > 1;
	unsigned int formatSize = Texture::blockSize(texture.format());
	for (unsigned int level = 0; level < texture.mipLevelCount(); ++level)
	{
		// Scanlins are 4-byte aligned.
		std::uint32_t imageSize = 0;
		if (compressed)
		{
			for (unsigned int depth = 0; depth < texture.depth(level); ++depth)
			{
				imageSize += static_cast<std::uint32_t>(texture.dataSize(Texture::CubeFace::PosX,
					level));
			}
		}
		else
		{
			for (unsigned int depth = 0; depth < texture.depth(level); ++depth)
				imageSize += ((texture.width(level)*formatSize + 3)/4)*4*texture.height(level);
		}
		if (texture.isArray())
			imageSize *= texture.faceCount();

		// Compressed formats should be at least 4-byte aligned. We should never have to write
		// padding bytes for cube faces or mipmaps.
		assert(imageSize % 4 == 0);
		if (!write(stream, imageSize))
			return Texture::SaveResult::WriteError;

		if (compressed)
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
		else
		{
			for (unsigned int depth = 0; depth < texture.depth(level); ++depth)
			{
				for (unsigned int face = 0; face < texture.faceCount(); ++face)
				{
					const char* data = reinterpret_cast<const char*>(texture.data(
						static_cast<Texture::CubeFace>(face), level, depth));
					unsigned int rowSize = texture.width(level)*formatSize;
					unsigned int padding = rowSize % 4;
					if (padding != 0)
						padding = 4 - padding;
					for (unsigned int y = 0; y < texture.height(level); ++y, data += rowSize)
					{
						stream.write(data, rowSize);
						for (unsigned int i = 0; i < padding; ++i)
							stream.put(0);
						if (!stream.good())
							return Texture::SaveResult::WriteError;
					}
				}
			}
		}
	}

	return Texture::SaveResult::Success;
}

} // namespace cuttlefish
