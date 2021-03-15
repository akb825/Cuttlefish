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

#if CUTTLEFISH_HAS_PVRTC

#include "PvrtcConverter.h"
#include "Shared.h"
#include <cuttlefish/Color.h>
#include <cassert>
#include <cmath>

#if CUTTLEFISH_GCC || CUTTLEFISH_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

#include <PVRTexLib.hpp>

#if CUTTLEFISH_GCC || CUTTLEFISH_CLANG
#pragma GCC diagnostic pop
#endif

namespace cuttlefish
{

PvrtcConverter::PvrtcConverter(const Texture& texture, const Image& image, Texture::Quality quality)
	: Converter(image)
	, m_format(texture.format())
	, m_quality(quality)
	, m_premultipliedAlpha(texture.alphaType() == Texture::Alpha::PreMultiplied)
{
}

void PvrtcConverter::process(unsigned int, unsigned int, ThreadData*)
{
	unsigned int width = image().width(), height = image().height();
	const PVRTuint64 inPixelType = PVRTGENPIXELID4('r', 'g', 'b', 'a', 8, 8, 8, 8);
	pvrtexlib::PVRTexture pvrTexture(pvrtexlib::PVRTextureHeader(inPixelType, width, height, 1, 1,
		1, 1, PVRTLCS_Linear, PVRTLVT_UnsignedByteNorm, m_premultipliedAlpha), nullptr);

	auto dstData = reinterpret_cast<std::uint8_t*>(pvrTexture.GetTextureDataPointer());
	for (unsigned int y = 0; y < height; ++y)
	{
		const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(y));
		for (unsigned int x = 0; x < width; ++x)
		{
			unsigned int index = (y*width + x)*4;
			dstData[index] = static_cast<std::uint8_t>(
				std::round(clamp(scanline[x].r, 0.0f, 1.0f)*0xFF));
			dstData[index + 1] = static_cast<std::uint8_t>(
				std::round(clamp(scanline[x].g, 0.0f, 1.0f)*0xFF));
			dstData[index + 2] = static_cast<std::uint8_t>(
				std::round(clamp(scanline[x].b, 0.0f, 1.0f)*0xFF));
			dstData[index + 3] = static_cast<std::uint8_t>(
				std::round(clamp(scanline[x].a, 0.0f, 1.0f)*0xFF));
		}
	}

	std::uint64_t pixelType;
	switch (m_format)
	{
		case Texture::Format::PVRTC1_RGB_2BPP:
			pixelType = PVRTLPF_PVRTCI_2bpp_RGB;
			break;
		case Texture::Format::PVRTC1_RGBA_2BPP:
			pixelType = PVRTLPF_PVRTCI_2bpp_RGBA;
			break;
		case Texture::Format::PVRTC1_RGB_4BPP:
			pixelType = PVRTLPF_PVRTCI_4bpp_RGB;
			break;
		case Texture::Format::PVRTC1_RGBA_4BPP:
			pixelType = PVRTLPF_PVRTCI_4bpp_RGBA;
			break;
		case Texture::Format::PVRTC2_RGBA_2BPP:
			pixelType = PVRTLPF_PVRTCII_2bpp;
			break;
		case Texture::Format::PVRTC2_RGBA_4BPP:
			pixelType = PVRTLPF_PVRTCII_4bpp;
			break;
		default:
			assert(false);
			return;
	}

	PVRTexLibCompressorQuality quality;
	switch (m_quality)
	{
		case Texture::Quality::Lowest:
			quality = PVRTLCQ_PVRTCFastest;
			break;
		case Texture::Quality::Low:
			quality = PVRTLCQ_PVRTCLow;
			break;
		case Texture::Quality::Normal:
			quality = PVRTLCQ_PVRTCNormal;
			break;
		case Texture::Quality::High:
			quality = PVRTLCQ_PVRTCHigh;
			break;
		case Texture::Quality::Highest:
			quality = PVRTLCQ_PVRTCBest;
			break;
		default:
			assert(false);
			return;
	}

	pvrTexture.Transcode(pixelType, PVRTLVT_UnsignedByteNorm, PVRTLCS_Linear, quality);

	auto textureData = reinterpret_cast<const std::uint8_t*>(pvrTexture.GetTextureDataPointer());
	data().assign(textureData, textureData + pvrTexture.GetTextureDataSize());
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_PVRTC
