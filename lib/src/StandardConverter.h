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

#pragma once

#include <cuttlefish/Config.h>

#include "Converter.h"
#include "Shared.h"
#include <cuttlefish/Color.h>
#include <algorithm>
#include <cmath>
#include <limits>

#if CUTTLEFISH_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <glm/gtc/packing.hpp>

#if CUTTLEFISH_GCC
#pragma GCC diagnostic pop
#endif

namespace cuttlefish
{

template <typename T>
inline T toFloat(float f)
{
	return f;
}

template <>
inline std::uint16_t toFloat<std::uint16_t>(float f)
{
	return glm::packHalf(glm::vec1(f)).x;
}

template <typename T, unsigned int C>
class StandardConverter : public Converter
{
public:
	static const unsigned int batchSize = 32;

	explicit StandardConverter(const Image& image)
		: Converter(image)
	{
		data().resize(image.width()*image.height()*sizeof(T)*C);
	}

	unsigned int jobsX() const override
	{
		// Only give jobs in 1 dimension to guarantee minimum alignment for multithreading.
		return (image().width()*image().height() + batchSize - 1)/batchSize;
	}

	unsigned int jobsY() const override
	{
		return 1;
	}
};

template <typename T, unsigned int C>
class UNormConverter : public StandardConverter<T, C>
{
public:
	using StandardConverter<T, C>::batchSize;
	using typename Converter::ThreadData;
	using Converter::data;
	using Converter::image;

	explicit UNormConverter(const Image& image)
		: StandardConverter<T, C>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override
	{
		const T maxVal = std::numeric_limits<T>::max();
		T* curData = reinterpret_cast<T*>(data().data()) + x*batchSize*C;
		unsigned int row = x*batchSize/image().width();
		const float* scanline = reinterpret_cast<const float*>(image().scanline(row));
		for (unsigned int i = 0; i < batchSize; ++i)
		{
			unsigned int curRow = (x*batchSize + i)/image().width();
			if (curRow != row)
			{
				if (curRow >= image().height())
					break;
				row = curRow;
				scanline = reinterpret_cast<const float*>(image().scanline(row));
			}

			unsigned int col = (x*batchSize + i) % image().width();
			for (unsigned int c = 0; c < C; ++c)
			{
				curData[i*C + c] = static_cast<T>(
					std::round(clamp(scanline[col*4 + c], 0.0f, 1.0f)*maxVal));
			}
		}
	}
};

template <typename T, unsigned int C>
class SNormConverter : public StandardConverter<T, C>
{
public:
	using StandardConverter<T, C>::batchSize;
	using typename Converter::ThreadData;
	using Converter::data;
	using Converter::image;

	explicit SNormConverter(const Image& image)
		: StandardConverter<T, C>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override
	{
		const T maxVal = std::numeric_limits<T>::max();
		T* curData = reinterpret_cast<T*>(data().data()) + x*batchSize*C;
		unsigned int row = x*batchSize/image().width();
		const float* scanline = reinterpret_cast<const float*>(image().scanline(row));
		for (unsigned int i = 0; i < batchSize; ++i)
		{
			unsigned int curRow = (x*batchSize + i)/image().width();
			if (curRow != row)
			{
				if (curRow >= image().height())
					break;
				row = curRow;
				scanline = reinterpret_cast<const float*>(image().scanline(row));
			}

			unsigned int col = (x*batchSize + i) % image().width();
			for (unsigned int c = 0; c < C; ++c)
			{
				curData[i*C + c] = static_cast<T>(
					std::round(clamp(scanline[col*4 + c], -1.0f, 1.0f)*maxVal));
			}
		}
	}
};

template <typename T, unsigned int C>
class IntConverter : public StandardConverter<T, C>
{
public:
	using StandardConverter<T, C>::batchSize;
	using typename Converter::ThreadData;
	using Converter::data;
	using Converter::image;

	explicit IntConverter(const Image& image)
		: StandardConverter<T, C>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override
	{
		const float minVal = static_cast<float>(std::numeric_limits<T>::min());
		const float maxVal = static_cast<float>(std::numeric_limits<T>::max());

		T* curData = reinterpret_cast<T*>(data().data()) + x*batchSize*C;
		unsigned int row = x*batchSize/image().width();
		const float* scanline = reinterpret_cast<const float*>(image().scanline(row));
		for (unsigned int i = 0; i < batchSize; ++i)
		{
			unsigned int curRow = (x*batchSize + i)/image().width();
			if (curRow != row)
			{
				if (curRow >= image().height())
					break;
				row = curRow;
				scanline = reinterpret_cast<const float*>(image().scanline(row));
			}

			unsigned int col = (x*batchSize + i) % image().width();
			for (unsigned int c = 0; c < C; ++c)
			{
				curData[i*C + c] = static_cast<T>(
					std::round(clamp(scanline[col*4 + c], minVal, maxVal)));
			}
		}
	}
};

template <typename T, unsigned int C>
class FloatConverter : public StandardConverter<T, C>
{
public:
	using StandardConverter<T, C>::batchSize;
	using typename Converter::ThreadData;
	using Converter::data;
	using Converter::image;

	explicit FloatConverter(const Image& image)
		: StandardConverter<T, C>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override
	{
		T* curData = reinterpret_cast<T*>(data().data()) + x*batchSize*C;
		unsigned int row = x*batchSize/image().width();
		const float* scanline = reinterpret_cast<const float*>(image().scanline(row));
		for (unsigned int i = 0; i < batchSize; ++i)
		{
			unsigned int curRow = (x*batchSize + i)/image().width();
			if (curRow != row)
			{
				if (curRow >= image().height())
					break;
				row = curRow;
				scanline = reinterpret_cast<const float*>(image().scanline(row));
			}

			unsigned int col = (x*batchSize + i) % image().width();
			for (unsigned int c = 0; c < C; ++c)
				curData[i*C + c] = toFloat<T>(scanline[col*4 + c]);
		}
	}
};

class R4G4Converter : public StandardConverter<std::uint8_t, 1>
{
public:
	explicit R4G4Converter(const Image& image)
		: StandardConverter<std::uint8_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class R4G4B4A4Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit R4G4B4A4Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class B4G4R4A4Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit B4G4R4A4Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class A4R4G4B4Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit A4R4G4B4Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class R5G6B5Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit R5G6B5Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class B5G6R5Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit B5G6R5Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class R5G5B5A1Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit R5G5B5A1Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class B5G5R5A1Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit B5G5R5A1Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class A1R5G5B5Converter : public StandardConverter<std::uint16_t, 1>
{
public:
	explicit A1R5G5B5Converter(const Image& image)
		: StandardConverter<std::uint16_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class B8G8R8Converter : public StandardConverter<std::uint8_t, 3>
{
public:
	explicit B8G8R8Converter(const Image& image)
		: StandardConverter<std::uint8_t, 3>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class B8G8R8A8Converter : public StandardConverter<std::uint8_t, 4>
{
public:
	explicit B8G8R8A8Converter(const Image& image)
		: StandardConverter<std::uint8_t, 4>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class A8B8G8R8Converter : public StandardConverter<std::uint8_t, 4>
{
public:
	explicit A8B8G8R8Converter(const Image& image)
		: StandardConverter<std::uint8_t, 4>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class A2R10G10B10UNormConverter : public StandardConverter<std::uint32_t, 1>
{
public:
	explicit A2R10G10B10UNormConverter(const Image& image)
		: StandardConverter<std::uint32_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class A2R10G10B10UIntConverter : public StandardConverter<std::uint32_t, 1>
{
public:
	explicit A2R10G10B10UIntConverter(const Image& image)
		: StandardConverter<std::uint32_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class A2B10G10R10UNormConverter : public StandardConverter<std::uint32_t, 1>
{
public:
	explicit A2B10G10R10UNormConverter(const Image& image)
		: StandardConverter<std::uint32_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class A2B10G10R10UIntConverter : public StandardConverter<std::uint32_t, 1>
{
public:
	explicit A2B10G10R10UIntConverter(const Image& image)
		: StandardConverter<std::uint32_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class B10R11R11UFloatConverter : public StandardConverter<std::uint32_t, 1>
{
public:
	explicit B10R11R11UFloatConverter(const Image& image)
		: StandardConverter<std::uint32_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

class E5B9G9R9UFloatConverter : public StandardConverter<std::uint32_t, 1>
{
public:
	explicit E5B9G9R9UFloatConverter(const Image& image)
		: StandardConverter<std::uint32_t, 1>(image)
	{
	}

	void process(unsigned int x, unsigned int, ThreadData*) override;
};

} // namespace cuttlefish
