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

#include "PvrtcConverter.h"
#include "Shared.h"
#include <cuttlefish/Color.h>
#include <cassert>
#include <cmath>
#include <string.h>

#if CUTTLEFISH_HAS_PVRTC

#if CUTTLEFISH_GCC || CUTTLEFISH_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <PVRTextureUtilities.h>
#include <PVRTTexture.h>

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

void PvrtcConverter::process(unsigned int, unsigned int)
{
	pvrtexture::CPVRTexture pvrTexture(pvrtexture::CPVRTextureHeader(
		pvrtexture::PVRStandard8PixelType.PixelTypeID, image().width(), image().height(), 1,
		1, 1, 1, ePVRTCSpacelRGB, ePVRTVarTypeUnsignedByteNorm, m_premultipliedAlpha));

	auto dstData = reinterpret_cast<std::uint8_t*>(pvrTexture.getDataPtr());
	unsigned int width = image().width(), height = image().height();
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
			pixelType = ePVRTPF_PVRTCI_2bpp_RGB;
			break;
		case Texture::Format::PVRTC1_RGBA_2BPP:
			pixelType = ePVRTPF_PVRTCI_2bpp_RGBA;
			break;
		case Texture::Format::PVRTC1_RGB_4BPP:
			pixelType = ePVRTPF_PVRTCI_4bpp_RGB;
			break;
		case Texture::Format::PVRTC1_RGBA_4BPP:
			pixelType = ePVRTPF_PVRTCI_4bpp_RGBA;
			break;
		case Texture::Format::PVRTC2_RGBA_2BPP:
			pixelType = ePVRTPF_PVRTCII_2bpp;
			break;
		case Texture::Format::PVRTC2_RGBA_4BPP:
			pixelType = ePVRTPF_PVRTCII_4bpp;
			break;
		default:
			assert(false);
			return;
	}

	pvrtexture::Transcode(pvrTexture, pixelType, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB,
		static_cast<pvrtexture::ECompressorQuality>(m_quality));

	auto textureData = reinterpret_cast<const std::uint8_t*>(pvrTexture.getDataPtr());
	data().assign(textureData, textureData + pvrTexture.getDataSize());
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_PVRTC
