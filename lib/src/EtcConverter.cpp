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

#include "EtcConverter.h"
#include <cuttlefish/Color.h>
#include <cassert>
#include <algorithm>
#include <cstring>

#if CUTTLEFISH_HAS_ETC

#include <Etc.h>

namespace cuttlefish
{

EtcConverter::EtcConverter(const Texture& texture, const Image& image, Texture::Quality quality)
	: Converter(image), m_jobsX((image.width() + blockDim - 1)/blockDim),
	m_jobsY((image.height() + blockDim - 1)/blockDim)
{
	switch (quality)
	{
		case Texture::Quality::Lowest:
			m_effort = ETCCOMP_MIN_EFFORT_LEVEL;
			break;
		case Texture::Quality::Low:
			m_effort = (ETCCOMP_MIN_EFFORT_LEVEL + ETCCOMP_DEFAULT_EFFORT_LEVEL)/2;
			break;
		case Texture::Quality::Normal:
			m_effort = ETCCOMP_DEFAULT_EFFORT_LEVEL;
			break;
		case Texture::Quality::High:
			m_effort = (ETCCOMP_DEFAULT_EFFORT_LEVEL + ETCCOMP_MAX_EFFORT_LEVEL)/2;
			break;
		case Texture::Quality::Highest:
			m_effort = ETCCOMP_MAX_EFFORT_LEVEL;
			break;
		default:
			assert(false);
			break;
	}

	switch (texture.format())
	{
		case Texture::Format::ETC1:
			m_blockSize = 8;
			m_format = Etc::Image::Format::ETC1;
			if (texture.colorSpace() == Texture::Color::Linear)
				m_metric = Etc::RGBX;
			else
				m_metric = Etc::REC709;
			break;
		case Texture::Format::ETC2_R8G8B8:
			m_blockSize = 8;
			m_format = Etc::Image::Format::RGB8;
			if (texture.colorSpace() == Texture::Color::Linear)
				m_metric = Etc::RGBX;
			else
				m_metric = Etc::REC709;
			break;
		case Texture::Format::ETC2_R8G8B8A1:
			m_blockSize = 8;
			m_format = Etc::Image::Format::RGB8A1;
			if (texture.colorSpace() == Texture::Color::Linear)
				m_metric = Etc::RGBA;
			else
				m_metric = Etc::REC709;
			break;
		case Texture::Format::ETC2_R8G8B8A8:
			m_blockSize = 16;
			m_format = Etc::Image::Format::RGBA8;
			if (texture.colorSpace() == Texture::Color::Linear)
				m_metric = Etc::RGBA;
			else
				m_metric = Etc::REC709;
			break;
		case Texture::Format::EAC_R11:
			m_blockSize = 8;
			if (texture.type() == Texture::Type::UNorm)
				m_format = Etc::Image::Format::R11;
			else
			{
				assert(texture.type() == Texture::Type::SNorm);
				m_format = Etc::Image::Format::SIGNED_R11;
			}
			m_metric = Etc::NUMERIC;
			break;
		case Texture::Format::EAC_R11G11:
			m_blockSize = 16;
			if (texture.type() == Texture::Type::UNorm)
				m_format = Etc::Image::Format::RG11;
			else
			{
				assert(texture.type() == Texture::Type::SNorm);
				m_format = Etc::Image::Format::SIGNED_RG11;
			}
			m_metric = Etc::NUMERIC;
			break;
		default:
			assert(false);
			break;
	}

	data().resize(m_jobsX*m_jobsY*m_blockSize);
}

void EtcConverter::process(unsigned int x, unsigned int y)
{
	ColorRGBAf pixels[blockDim*blockDim];
	unsigned int limitX = std::min((x + 1)*blockDim, image().width());
	unsigned int limitY = std::min((y + 1)*blockDim, image().height());
	for (unsigned int j = y*blockDim, index = 0; j < limitY; ++j)
	{
		auto scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(j));
		for (unsigned int i = x*blockDim; i < limitX; ++i, ++index)
			pixels[index] = scanline[i];
	}

	Etc::Image etcImage(reinterpret_cast<float*>(pixels), limitX - x*blockDim, limitY - y*blockDim,
		m_metric);
	etcImage.Encode(m_format, m_metric, m_effort, 1, 1);

	assert(etcImage.GetEncodingBitsBytes() == m_blockSize);
	void* block = data().data() + (y*m_jobsX + x)*m_blockSize;
	std::memcpy(block, etcImage.GetEncodingBits(), m_blockSize);
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ETC
