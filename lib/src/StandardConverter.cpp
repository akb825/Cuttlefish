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

#include "StandardConverter.h"

namespace cuttlefish
{

void R4G4Converter::process(unsigned int x, unsigned int)
{
	std::uint8_t* curData = reinterpret_cast<std::uint8_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint8_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0xF)) & 0xF;
		auto g = static_cast<std::uint8_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0xF)) & 0xF;
		curData[i] = static_cast<std::uint8_t>(g | (r << 4));
	}
}

void R4G4B4A4Converter::process(unsigned int x, unsigned int)
{
	std::uint16_t* curData = reinterpret_cast<std::uint16_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint16_t>(std::round(clamp(scanline[col].r, 0, 1)*0xF)) & 0xF;
		auto g = static_cast<std::uint16_t>(std::round(clamp(scanline[col].g, 0, 1)*0xF)) & 0xF;
		auto b = static_cast<std::uint16_t>(std::round(clamp(scanline[col].b, 0, 1)*0xF)) & 0xF;
		auto a = static_cast<std::uint16_t>(std::round(clamp(scanline[col].a, 0, 1)*0xF)) & 0xF;
		curData[i] = static_cast<std::uint16_t>(a | (b << 4) | (g << 8) | (r << 12));
	}
}

void B4G4R4A4Converter::process(unsigned int x, unsigned int)
{
	std::uint16_t* curData = reinterpret_cast<std::uint16_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint16_t>(std::round(clamp(scanline[col].r, 0, 1)*0xF)) & 0xF;
		auto g = static_cast<std::uint16_t>(std::round(clamp(scanline[col].g, 0, 1)*0xF)) & 0xF;
		auto b = static_cast<std::uint16_t>(std::round(clamp(scanline[col].b, 0, 1)*0xF)) & 0xF;
		auto a = static_cast<std::uint16_t>(std::round(clamp(scanline[col].a, 0, 1)*0xF)) & 0xF;
		curData[i] = static_cast<std::uint16_t>(a | (r << 4) | (g << 8) | (b << 12));
	}
}

void R5G6B5Converter::process(unsigned int x, unsigned int)
{
	std::uint16_t* curData = reinterpret_cast<std::uint16_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint16_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto g = static_cast<std::uint16_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0x3F)) & 0x3F;
		auto b = static_cast<std::uint16_t>(std::round(clamp(scanline[col].a, 0.0f, 1.0f)*0x1F)) & 0x1F;
		curData[i] = static_cast<std::uint16_t>(g | (b << 5) | (r << 11));
	}
}

void B5G6R5Converter::process(unsigned int x, unsigned int)
{
	std::uint16_t* curData = reinterpret_cast<std::uint16_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint16_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto g = static_cast<std::uint16_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0x3F)) & 0x3F;
		auto b = static_cast<std::uint16_t>(std::round(clamp(scanline[col].a, 0.0f, 1.0f)*0x1F)) & 0x1F;
		curData[i] = static_cast<std::uint16_t>(r | (g << 5) | (b << 11));
	}
}

void R5G5B5A1Converter::process(unsigned int x, unsigned int)
{
	std::uint16_t* curData = reinterpret_cast<std::uint16_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto a = static_cast<std::uint16_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto r = static_cast<std::uint16_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto g = static_cast<std::uint16_t>(std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto b = static_cast<std::uint16_t>(std::round(clamp(scanline[col].a, 0.0f, 1.0f)));
		curData[i] = static_cast<std::uint16_t>(a | (b << 1) | (g << 6) | (r << 11));
	}
}

void B5G5R5A1Converter::process(unsigned int x, unsigned int)
{
	std::uint16_t* curData = reinterpret_cast<std::uint16_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto a = static_cast<std::uint16_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto r = static_cast<std::uint16_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto g = static_cast<std::uint16_t>(std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto b = static_cast<std::uint16_t>(std::round(clamp(scanline[col].a, 0.0f, 1.0f)));
		curData[i] = static_cast<std::uint16_t>(a | (r << 1) | (g << 6) | (b << 11));
	}
}

void A1R5G5B5Converter::process(unsigned int x, unsigned int)
{
	std::uint16_t* curData = reinterpret_cast<std::uint16_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto a = static_cast<std::uint16_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto r = static_cast<std::uint16_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto g = static_cast<std::uint16_t>(std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0x1F)) & 0x1F;
		auto b = static_cast<std::uint16_t>(std::round(clamp(scanline[col].a, 0.0f, 1.0f)));
		curData[i] = static_cast<std::uint16_t>(b | (g << 5) | (r << 10) | (a << 15));
	}
}

void B8G8R8Converter::process(unsigned int x, unsigned int)
{
	std::uint8_t* curData = reinterpret_cast<std::uint8_t*>(data().data() + x*batchSize*3);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		curData[i*3] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0xFF));
		curData[i*3 + 1] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0xFF));
		curData[i*3 + 2] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0xFF));
	}
}

void B8G8R8A8Converter::process(unsigned int x, unsigned int)
{
	std::uint8_t* curData = reinterpret_cast<std::uint8_t*>(data().data() + x*batchSize*4);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		curData[i*4] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0xFF));
		curData[i*4 + 1] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0xFF));
		curData[i*4 + 2] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0xFF));
		curData[i*4 + 3] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].a, 0.0f, 1.0f)*0xFF));
	}
}

void A8B8G8R8Converter::process(unsigned int x, unsigned int)
{
	std::uint8_t* curData = reinterpret_cast<std::uint8_t*>(data().data() + x*batchSize*4);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		curData[i*4] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].a, 0.0f, 1.0f)*0xFF));
		curData[i*4 + 1] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0xFF));
		curData[i*4 + 2] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0xFF));
		curData[i*4 + 3] = static_cast<std::uint8_t>(
			std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0xFF));
	}
}

void A2R10G10B10UNormConverter::process(unsigned int x, unsigned int)
{
	std::uint32_t* curData = reinterpret_cast<std::uint32_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint32_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0x3FF)) & 0x3FF;
		auto g = static_cast<std::uint32_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0x3FF)) & 0x3FF;
		auto b = static_cast<std::uint32_t>(std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0x3FF)) & 0x3FF;
		auto a = static_cast<std::uint32_t>(std::round(clamp(scanline[col].a, 0.0f, 1.0f)*0x3)) & 0x3;
		curData[i] = static_cast<std::uint32_t>(b | (g << 10) | (r << 20) | (a << 30));
	}
}

void A2R10G10B10UIntConverter::process(unsigned int x, unsigned int)
{
	std::uint32_t* curData = reinterpret_cast<std::uint32_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint32_t>(std::round(clamp(scanline[col].r, 0, 0x3FF)));
		auto g = static_cast<std::uint32_t>(std::round(clamp(scanline[col].g, 0, 0x3FF)));
		auto b = static_cast<std::uint32_t>(std::round(clamp(scanline[col].b, 0, 0x3FF)));
		auto a = static_cast<std::uint32_t>(std::round(clamp(scanline[col].a, 0, 0x3)));
		curData[i] = static_cast<std::uint32_t>(b | (g << 10) | (r << 20) | (a << 30));
	}
}

void A2B10G10R10UNormConverter::process(unsigned int x, unsigned int)
{
	std::uint32_t* curData = reinterpret_cast<std::uint32_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint32_t>(std::round(clamp(scanline[col].r, 0.0f, 1.0f)*0x3FF)) & 0x3FF;
		auto g = static_cast<std::uint32_t>(std::round(clamp(scanline[col].g, 0.0f, 1.0f)*0x3FF)) & 0x3FF;
		auto b = static_cast<std::uint32_t>(std::round(clamp(scanline[col].b, 0.0f, 1.0f)*0x3FF)) & 0x3FF;
		auto a = static_cast<std::uint32_t>(std::round(clamp(scanline[col].a, 0.0f, 1.0f)*0x3)) & 0x3;
		curData[i] = static_cast<std::uint32_t>(r | (g << 10) | (b << 20) | (a << 30));
	}
}

void A2B10G10R10UIntConverter::process(unsigned int x, unsigned int)
{
	std::uint32_t* curData = reinterpret_cast<std::uint32_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		auto r = static_cast<std::uint32_t>(std::round(clamp(scanline[col].r, 0, 0x3FF)));
		auto g = static_cast<std::uint32_t>(std::round(clamp(scanline[col].g, 0, 0x3FF)));
		auto b = static_cast<std::uint32_t>(std::round(clamp(scanline[col].b, 0, 0x3FF)));
		auto a = static_cast<std::uint32_t>(std::round(clamp(scanline[col].a, 0, 0x3)));
		curData[i] = static_cast<std::uint32_t>(r | (g << 10) | (b << 20) | (a << 30));
	}
}

void B10R11R11UFloatConverter::process(unsigned int x, unsigned int)
{
	std::uint32_t* curData = reinterpret_cast<std::uint32_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		curData[i] = glm::packF2x11_1x10(*reinterpret_cast<const glm::vec3*>(scanline + col));
	}
}

void E5B9G9R9UFloatConverter::process(unsigned int x, unsigned int)
{
	std::uint32_t* curData = reinterpret_cast<std::uint32_t*>(data().data() + x*batchSize);
	unsigned int row = x*batchSize/image().width();
	const ColorRGBAf* scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
	for (unsigned int i = 0; i < batchSize; ++i)
	{
		unsigned int curRow = (x*batchSize + i)/image().width();
		if (curRow != row)
		{
			if (curRow >= image().height())
				break;
			row = curRow;
			scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(row));
		}

		unsigned int col = (x*batchSize + i) % image().width();
		curData[i] = glm::packF3x9_E1x5(*reinterpret_cast<const glm::vec3*>(scanline + col));
	}
}

} // namespace cuttlefish
