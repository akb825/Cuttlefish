/*
 * Copyright 2017-2024 Aaron Barany
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

#include <cuttlefish/Image.h>

#include "Shared.h"
#include <cuttlefish/Color.h>
#include <FreeImage.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>

namespace cuttlefish
{

namespace
{

class FreeImageInitialize
{
public:
	FreeImageInitialize()
	{
		FreeImage_Initialise();
	}

	~FreeImageInitialize()
	{
		FreeImage_DeInitialise();
	}
};

void freeImageInitialize()
{
	static FreeImageInitialize initializer;
}

Image::Format getFormat(FIBITMAP* image)
{
	switch (FreeImage_GetImageType(image))
	{
		case FIT_BITMAP:
			switch (FreeImage_GetBPP(image))
			{
				case 8:
					return Image::Format::Gray8;
				case 16:
					switch (FreeImage_GetGreenMask(image))
					{
						case FI16_555_GREEN_MASK:
							return Image::Format::RGB5;
						case FI16_565_GREEN_MASK:
							return Image::Format::RGB565;
						default:
							return Image::Format::Invalid;
					}
				case 24:
					return Image::Format::RGB8;
				case 32:
					return Image::Format::RGBA8;
				default:
					return Image::Format::Invalid;
			}
		case FIT_UINT16:
			return Image::Format::Gray16;
		case FIT_INT16:
			return Image::Format::Int16;
		case FIT_UINT32:
			return Image::Format::UInt32;
		case FIT_INT32:
			return Image::Format::Int32;
		case FIT_FLOAT:
			return Image::Format::Float;
		case FIT_DOUBLE:
			return Image::Format::Double;
		case FIT_COMPLEX:
			return Image::Format::Complex;
		case FIT_RGB16:
			return Image::Format::RGB16;
		case FIT_RGBA16:
			return Image::Format::RGBA16;;
		case FIT_RGBF:
			return Image::Format::RGBF;
		case FIT_RGBAF:
			return Image::Format::RGBAF;
		default:
			return Image::Format::Invalid;
	}
}

FREE_IMAGE_TYPE getFreeImageFormat(Image::Format format, unsigned int& bpp, unsigned int& redMask,
	unsigned int& greenMask, unsigned int& blueMask)
{
	bpp = 0;
	redMask = 0;
	greenMask = 0;
	blueMask = 0;
	switch (format)
	{
		case Image::Format::Gray8:
			bpp = 8;
			return FIT_BITMAP;
		case Image::Format::Gray16:
			return FIT_UINT16;
		case Image::Format::RGB5:
			bpp = 16;
			redMask = FI16_555_RED_MASK;
			greenMask = FI16_555_GREEN_MASK;
			blueMask = FI16_555_BLUE_MASK;
			return FIT_BITMAP;
		case Image::Format::RGB565:
			bpp = 16;
			redMask = FI16_565_RED_MASK;
			greenMask = FI16_565_GREEN_MASK;
			blueMask = FI16_565_BLUE_MASK;
			return FIT_BITMAP;
		case Image::Format::RGB8:
			bpp = 24;
			return FIT_BITMAP;
		case Image::Format::RGB16:
			return FIT_RGB16;
		case Image::Format::RGBF:
			return FIT_RGBF;
		case Image::Format::RGBA8:
			bpp = 32;
			return FIT_BITMAP;
		case Image::Format::RGBA16:
			return FIT_RGBA16;
		case Image::Format::RGBAF:
			return FIT_RGBAF;
		case Image::Format::Int16:
			return FIT_INT16;
		case Image::Format::UInt16:
			return FIT_UINT16;
		case Image::Format::Int32:
			return FIT_INT32;
		case Image::Format::UInt32:
			return FIT_UINT32;
		case Image::Format::Float:
			return FIT_FLOAT;
		case Image::Format::Double:
			return FIT_DOUBLE;
		case Image::Format::Complex:
			return FIT_COMPLEX;
		case Image::Format::Invalid:
			return FIT_UNKNOWN;
	}
	return FIT_UNKNOWN;
}

bool needFloatConvertFallback(Image::Format format)
{
	switch (format)
	{
		case Image::Format::UInt16:
		case Image::Format::RGBF:
		case Image::Format::RGBAF:
		case Image::Format::Float:
			return true;
		default:
			return false;
	}
}

bool isGrayscaleFormat(Image::Format format)
{
	switch (format)
	{
		case Image::Format::Gray8:
		case Image::Format::Gray16:
		case Image::Format::Float:
		case Image::Format::Double:
			return true;
		default:
			return false;
	}
}

unsigned int readIStream(void* buffer, unsigned int size, unsigned int count, fi_handle handle)
{
	auto stream = reinterpret_cast<std::istream*>(handle);
	stream->read(reinterpret_cast<char*>(buffer), size*count);
	return static_cast<unsigned int>(stream->gcount());
}

unsigned int writeIStream(void*, unsigned int, unsigned int, fi_handle)
{
	return 0;
}

int seekIStream(fi_handle handle, long offset, int origin)
{
	auto stream = reinterpret_cast<std::istream*>(handle);
	std::ios_base::seekdir dir;
	switch (origin)
	{
		case SEEK_SET:
			dir = std::ios_base::beg;
			break;
		case SEEK_CUR:
			dir = std::ios_base::cur;
			break;
		case SEEK_END:
			dir = std::ios_base::end;
			break;
		default:
			return -1;
	}
	stream->seekg(offset, dir);
	return stream->good() ? 0 : -1;
}

long tellIStream(fi_handle handle)
{
	auto stream = reinterpret_cast<std::istream*>(handle);
	return static_cast<long>(stream->tellg());
}

FreeImageIO istreamIO{&readIStream, &writeIStream, &seekIStream, tellIStream};

unsigned int readOStream(void*, unsigned int, unsigned int, fi_handle)
{
	return 0;
}

unsigned int writeOStream(void* buffer, unsigned int size, unsigned int count, fi_handle handle)
{
	auto stream = reinterpret_cast<std::ostream*>(handle);
	stream->write(reinterpret_cast<char*>(buffer), size*count);
	return count*size;
}

int seekOStream(fi_handle handle, long offset, int origin)
{
	auto stream = reinterpret_cast<std::ostream*>(handle);
	std::ios_base::seekdir dir;
	switch (origin)
	{
		case SEEK_SET:
			dir = std::ios_base::beg;
			break;
		case SEEK_CUR:
			dir = std::ios_base::cur;
			break;
		case SEEK_END:
			dir = std::ios_base::end;
			break;
		default:
			return -1;
	}
	stream->seekp(offset, dir);
	return stream->good() ? 0 : -1;
}

long tellOStream(fi_handle handle)
{
	auto stream = reinterpret_cast<std::ostream*>(handle);
	return static_cast<long>(stream->tellp());
}

FreeImageIO ostreamIO{&readOStream, &writeOStream, &seekOStream, tellOStream};

double toDoubleNorm(std::uint8_t value)
{
	return value/255.0;
}

double toDoubleNorm(std::uint16_t value)
{
	return value/65535.0;
}

double toDoubleNorm5(unsigned int value)
{
	return value/31.0;
}

double toDoubleNorm6(unsigned int value)
{
	return value/63.0;
}

double clamp(double d)
{
	return std::max(std::min(d, 1.0), 0.0);
}

template <typename T>
T clamp(T v)
{
	return std::max(std::min(v, std::numeric_limits<T>::max()), std::numeric_limits<T>::lowest());
}

template <typename T>
T fromDoubleNorm(double d)
{
	return static_cast<T>(std::round(clamp(d)*std::numeric_limits<T>::max()));
}

std::uint8_t fromDoubleNorm5(double d)
{
	return static_cast<std::uint8_t>(std::round(clamp(d)*31));
}

std::uint8_t fromDoubleNorm6(double d)
{
	return static_cast<std::uint8_t>(std::round(clamp(d)*63));
}

void* getScanlineImpl(FIBITMAP* image, unsigned int height, unsigned int y)
{
	return FreeImage_GetScanLine(image, height - y - 1);
}

bool getPixelImpl(ColorRGBAd& outColor, Image::Format format, const void* scanline, unsigned int x)
{
	switch (format)
	{
		case Image::Format::Invalid:
			return false;
		case Image::Format::Gray8:
			outColor.r = outColor.g = outColor.b = toDoubleNorm(
				reinterpret_cast<const std::uint8_t*>(scanline)[x]);
			outColor.a = 1.0;
			return true;
		case Image::Format::Gray16:
			outColor.r = outColor.g = outColor.b =
				toDoubleNorm(reinterpret_cast<const std::uint16_t*>(scanline)[x]);
			outColor.a = 1.0;
			return true;
		case Image::Format::RGB5:
		{
			std::uint16_t pixel = static_cast<const std::uint16_t*>(scanline)[x];
			outColor.r = toDoubleNorm5((pixel & FI16_555_RED_MASK) >> FI16_555_RED_SHIFT);
			outColor.g = toDoubleNorm5((pixel & FI16_555_GREEN_MASK) >> FI16_555_GREEN_SHIFT);
			outColor.b = toDoubleNorm5((pixel & FI16_555_BLUE_MASK) >> FI16_555_BLUE_SHIFT);
			outColor.a = 1.0;
			return true;
		}
		case Image::Format::RGB565:
		{
			std::uint16_t pixel = static_cast<const std::uint16_t*>(scanline)[x];
			outColor.r = toDoubleNorm5((pixel & FI16_565_RED_MASK) >> FI16_565_RED_SHIFT);
			outColor.g = toDoubleNorm6((pixel & FI16_565_GREEN_MASK) >> FI16_565_GREEN_SHIFT);
			outColor.b = toDoubleNorm5((pixel & FI16_565_BLUE_MASK) >> FI16_565_BLUE_SHIFT);
			outColor.a = 1.0;
			return true;
		}
		case Image::Format::RGB8:
		{
			const RGBTRIPLE& pixel = static_cast<const RGBTRIPLE*>(scanline)[x];
			outColor.r = toDoubleNorm(pixel.rgbtRed);
			outColor.g = toDoubleNorm(pixel.rgbtGreen);
			outColor.b = toDoubleNorm(pixel.rgbtBlue);
			outColor.a = 1.0;
			return true;
		}
		case Image::Format::RGB16:
		{
			const FIRGB16& pixel = static_cast<const FIRGB16*>(scanline)[x];
			outColor.r = toDoubleNorm(pixel.red);
			outColor.g = toDoubleNorm(pixel.green);
			outColor.b = toDoubleNorm(pixel.blue);
			outColor.a = 1.0;
			return true;
		}
		case Image::Format::RGBF:
		{
			const FIRGBF& pixel = static_cast<const FIRGBF*>(scanline)[x];
			outColor.r = pixel.red;
			outColor.g = pixel.green;
			outColor.b = pixel.blue;
			outColor.a = 1.0;
			return true;
		}
		case Image::Format::RGBA8:
		{
			const RGBQUAD& pixel = static_cast<const RGBQUAD*>(scanline)[x];
			outColor.r = toDoubleNorm(pixel.rgbRed);
			outColor.g = toDoubleNorm(pixel.rgbGreen);
			outColor.b = toDoubleNorm(pixel.rgbBlue);
			outColor.a = toDoubleNorm(pixel.rgbReserved);
			return true;
		}
		case Image::Format::RGBA16:
		{
			const FIRGBA16& pixel = static_cast<const FIRGBA16*>(scanline)[x];
			outColor.r = toDoubleNorm(pixel.red);
			outColor.g = toDoubleNorm(pixel.green);
			outColor.b = toDoubleNorm(pixel.blue);
			outColor.a = toDoubleNorm(pixel.alpha);
			return true;
		}
		case Image::Format::RGBAF:
		{
			const FIRGBAF& pixel = static_cast<const FIRGBAF*>(scanline)[x];
			outColor.r = pixel.red;
			outColor.g = pixel.green;
			outColor.b = pixel.blue;
			outColor.a = pixel.alpha;
			return true;
		}
		case Image::Format::Int16:
			outColor.r = outColor.g = outColor.b =
				reinterpret_cast<const std::int16_t*>(scanline)[x];
			outColor.a = 1.0;
			return true;
		case Image::Format::UInt16:
			outColor.r = outColor.g = outColor.b =
				reinterpret_cast<const std::uint16_t*>(scanline)[x];
			outColor.a = 1.0;
			return true;
		case Image::Format::Int32:
			outColor.r = outColor.g = outColor.b =
				reinterpret_cast<const std::int32_t*>(scanline)[x];
			outColor.a = 1.0;
			return true;
		case Image::Format::UInt32:
			outColor.r = outColor.g = outColor.b =
				reinterpret_cast<const std::uint32_t*>(scanline)[x];
			outColor.a = 1.0;
			return true;
		case Image::Format::Float:
			outColor.r = outColor.g = outColor.b =
				reinterpret_cast<const float*>(scanline)[x];
			outColor.a = 1.0;
			return true;
		case Image::Format::Double:
			outColor.r = outColor.g = outColor.b =
				reinterpret_cast<const double*>(scanline)[x];
			outColor.a = 1.0;
			return true;
		case Image::Format::Complex:
		{
			const FICOMPLEX& pixel = static_cast<const FICOMPLEX*>(scanline)[x];
			outColor.r = pixel.r;
			outColor.g = pixel.i;
			outColor.b = 0.0;
			outColor.a = 1.0;
			return true;
		}
	}
	return false;
}

bool setPixelImpl(Image::Format format, void* scanline, unsigned int x, const ColorRGBAd& color)
{
	switch (format)
	{
		case Image::Format::Invalid:
			return false;
		case Image::Format::Gray8:
			reinterpret_cast<std::uint8_t*>(scanline)[x] =
				fromDoubleNorm<std::uint8_t>(toGrayscale(color.r, color.g, color.b));
			return true;
		case Image::Format::Gray16:
			reinterpret_cast<std::uint16_t*>(scanline)[x] =
				fromDoubleNorm<std::uint16_t>(toGrayscale(color.r, color.g, color.b));
			return true;
		case Image::Format::RGB5:
		{
			std::uint16_t& pixel = static_cast<std::uint16_t*>(scanline)[x];
			pixel = static_cast<std::uint16_t>(
				(fromDoubleNorm5(color.r) << FI16_555_RED_SHIFT) |
				(fromDoubleNorm5(color.g) << FI16_555_GREEN_SHIFT) |
				(fromDoubleNorm5(color.b) << FI16_555_BLUE_SHIFT));
			return true;
		}
		case Image::Format::RGB565:
		{
			std::uint16_t& pixel = static_cast<std::uint16_t*>(scanline)[x];
			pixel = static_cast<std::uint16_t>(
				(fromDoubleNorm5(color.r) << FI16_565_RED_SHIFT) |
				(fromDoubleNorm6(color.g) << FI16_565_GREEN_SHIFT) |
				(fromDoubleNorm5(color.b) << FI16_565_BLUE_SHIFT));
			return true;
		}
		case Image::Format::RGB8:
		{
			RGBTRIPLE& pixel = static_cast<RGBTRIPLE*>(scanline)[x];
			pixel.rgbtRed = fromDoubleNorm<std::uint8_t>(color.r);
			pixel.rgbtGreen = fromDoubleNorm<std::uint8_t>(color.g);
			pixel.rgbtBlue = fromDoubleNorm<std::uint8_t>(color.b);
			return true;
		}
		case Image::Format::RGB16:
		{
			FIRGB16& pixel = static_cast<FIRGB16*>(scanline)[x];
			pixel.red = fromDoubleNorm<std::uint16_t>(color.r);
			pixel.green = fromDoubleNorm<std::uint16_t>(color.g);
			pixel.blue = fromDoubleNorm<std::uint16_t>(color.b);
			return true;
		}
		case Image::Format::RGBF:
		{
			FIRGBF& pixel = static_cast<FIRGBF*>(scanline)[x];
			pixel.red = static_cast<float>(color.r);
			pixel.green = static_cast<float>(color.g);
			pixel.blue = static_cast<float>(color.b);
			return true;
		}
		case Image::Format::RGBA8:
		{
			RGBQUAD& pixel = static_cast<RGBQUAD*>(scanline)[x];
			pixel.rgbRed = fromDoubleNorm<std::uint8_t>(color.r);
			pixel.rgbGreen = fromDoubleNorm<std::uint8_t>(color.g);
			pixel.rgbBlue = fromDoubleNorm<std::uint8_t>(color.b);
			pixel.rgbReserved = fromDoubleNorm<std::uint8_t>(color.a);
			return true;
		}
		case Image::Format::RGBA16:
		{
			FIRGBA16& pixel = static_cast<FIRGBA16*>(scanline)[x];
			pixel.red = fromDoubleNorm<std::uint16_t>(color.r);
			pixel.green = fromDoubleNorm<std::uint16_t>(color.g);
			pixel.blue = fromDoubleNorm<std::uint16_t>(color.b);
			pixel.alpha = fromDoubleNorm<std::uint16_t>(color.a);
			return true;
		}
		case Image::Format::RGBAF:
		{
			FIRGBAF& pixel = static_cast<FIRGBAF*>(scanline)[x];
			pixel.red = static_cast<float>(color.r);
			pixel.green = static_cast<float>(color.g);
			pixel.blue = static_cast<float>(color.b);
			pixel.alpha = static_cast<float>(color.a);
			return true;
		}
		case Image::Format::Int16:
			reinterpret_cast<std::int16_t*>(scanline)[x] = clamp(
				static_cast<std::int16_t>(color.r));
			return true;
		case Image::Format::UInt16:
			reinterpret_cast<std::uint16_t*>(scanline)[x] = clamp(
				static_cast<std::int16_t>(color.r));
			return true;
		case Image::Format::Int32:
			reinterpret_cast<std::int32_t*>(scanline)[x] = clamp(
				static_cast<std::int32_t>(color.r));
			return true;
		case Image::Format::UInt32:
			reinterpret_cast<std::uint32_t*>(scanline)[x] = clamp(
				static_cast<std::uint32_t>(color.r));
			return true;
		case Image::Format::Float:
			reinterpret_cast<float*>(scanline)[x] =
				static_cast<float>(toGrayscale(color.r, color.g, color.b));
			return true;
		case Image::Format::Double:
			reinterpret_cast<double*>(scanline)[x] = toGrayscale(color.r, color.g, color.b);
			return true;
		case Image::Format::Complex:
		{
			FICOMPLEX& pixel = static_cast<FICOMPLEX*>(scanline)[x];
			pixel.r = color.r;
			pixel.i = color.g;
			return true;
		}
	}
	return false;
}

bool setPixelNoGrayscaleImpl(Image::Format format, void* scanline, unsigned int x,
	const ColorRGBAd& color)
{
	switch (format)
	{
		case Image::Format::Invalid:
			return false;
		case Image::Format::Gray8:
			reinterpret_cast<std::uint8_t*>(scanline)[x] = fromDoubleNorm<std::uint8_t>(color.r);
			return true;
		case Image::Format::Gray16:
			reinterpret_cast<std::uint16_t*>(scanline)[x] = fromDoubleNorm<std::uint16_t>(color.r);
			return true;
		case Image::Format::RGB5:
		{
			std::uint16_t& pixel = static_cast<std::uint16_t*>(scanline)[x];
			pixel = static_cast<std::uint16_t>(
				(fromDoubleNorm5(color.r) << FI16_555_RED_SHIFT) |
				(fromDoubleNorm5(color.g) << FI16_555_GREEN_SHIFT) |
				(fromDoubleNorm5(color.b) << FI16_555_BLUE_SHIFT));
			return true;
		}
		case Image::Format::RGB565:
		{
			std::uint16_t& pixel = static_cast<std::uint16_t*>(scanline)[x];
			pixel = static_cast<std::uint16_t>(
				(fromDoubleNorm5(color.r) << FI16_565_RED_SHIFT) |
				(fromDoubleNorm6(color.g) << FI16_565_GREEN_SHIFT) |
				(fromDoubleNorm5(color.b) << FI16_565_BLUE_SHIFT));
			return true;
		}
		case Image::Format::RGB8:
		{
			RGBTRIPLE& pixel = static_cast<RGBTRIPLE*>(scanline)[x];
			pixel.rgbtRed = fromDoubleNorm<std::uint8_t>(color.r);
			pixel.rgbtGreen = fromDoubleNorm<std::uint8_t>(color.g);
			pixel.rgbtBlue = fromDoubleNorm<std::uint8_t>(color.b);
			return true;
		}
		case Image::Format::RGB16:
		{
			FIRGB16& pixel = static_cast<FIRGB16*>(scanline)[x];
			pixel.red = fromDoubleNorm<std::uint16_t>(color.r);
			pixel.green = fromDoubleNorm<std::uint16_t>(color.g);
			pixel.blue = fromDoubleNorm<std::uint16_t>(color.b);
			return true;
		}
		case Image::Format::RGBF:
		{
			FIRGBF& pixel = static_cast<FIRGBF*>(scanline)[x];
			pixel.red = static_cast<float>(color.r);
			pixel.green = static_cast<float>(color.g);
			pixel.blue = static_cast<float>(color.b);
			return true;
		}
		case Image::Format::RGBA8:
		{
			RGBQUAD& pixel = static_cast<RGBQUAD*>(scanline)[x];
			pixel.rgbRed = fromDoubleNorm<std::uint8_t>(color.r);
			pixel.rgbGreen = fromDoubleNorm<std::uint8_t>(color.g);
			pixel.rgbBlue = fromDoubleNorm<std::uint8_t>(color.b);
			pixel.rgbReserved = fromDoubleNorm<std::uint8_t>(color.a);
			return true;
		}
		case Image::Format::RGBA16:
		{
			FIRGBA16& pixel = static_cast<FIRGBA16*>(scanline)[x];
			pixel.red = fromDoubleNorm<std::uint16_t>(color.r);
			pixel.green = fromDoubleNorm<std::uint16_t>(color.g);
			pixel.blue = fromDoubleNorm<std::uint16_t>(color.b);
			pixel.alpha = fromDoubleNorm<std::uint16_t>(color.a);
			return true;
		}
		case Image::Format::RGBAF:
		{
			FIRGBAF& pixel = static_cast<FIRGBAF*>(scanline)[x];
			pixel.red = static_cast<float>(color.r);
			pixel.green = static_cast<float>(color.g);
			pixel.blue = static_cast<float>(color.b);
			pixel.alpha = static_cast<float>(color.a);
			return true;
		}
		case Image::Format::Int16:
			reinterpret_cast<std::int16_t*>(scanline)[x] = clamp(
				static_cast<std::int16_t>(color.r));
			return true;
		case Image::Format::UInt16:
			reinterpret_cast<std::uint16_t*>(scanline)[x] = clamp(
				static_cast<std::uint16_t>(color.r));
			return true;
		case Image::Format::Int32:
			reinterpret_cast<std::int32_t*>(scanline)[x] = clamp(
				static_cast<std::int32_t>(color.r));
			return true;
		case Image::Format::UInt32:
			reinterpret_cast<std::uint32_t*>(scanline)[x] = clamp(
				static_cast<std::uint32_t>(color.r));
			return true;
		case Image::Format::Float:
			reinterpret_cast<float*>(scanline)[x] = static_cast<float>(color.r);
			return true;
		case Image::Format::Double:
			reinterpret_cast<double*>(scanline)[x] = color.r;
			return true;
		case Image::Format::Complex:
		{
			FICOMPLEX& pixel = static_cast<FICOMPLEX*>(scanline)[x];
			pixel.r = color.r;
			pixel.i = color.g;
			return true;
		}
	}
	return false;
}

} // namespace

struct Image::Impl
{
	static Impl* create(FIBITMAP* image, ColorSpace colorSpace, Format format)
	{
		if (!image)
			return nullptr;

		if (FreeImage_GetColorType(image) == FIC_PALETTE)
		{
			FIBITMAP* newImage;
			if (FreeImage_IsTransparent(image))
				newImage = FreeImage_ConvertTo32Bits(image);
			else
				newImage = FreeImage_ConvertTo24Bits(image);

			FreeImage_Unload(image);
			image = newImage;
			if (!image)
				return nullptr;
		}

		if (format == Format::Invalid)
			format = getFormat(image);
		if (format == Format::Invalid)
		{
			FreeImage_Unload(image);
			return nullptr;
		}

		return new Impl(image, format, colorSpace);
	}

	Impl(FIBITMAP* img, Format format, ColorSpace colorSpace)
		: image(img), format(format), colorSpace(colorSpace), bitsPerPixel(FreeImage_GetBPP(img)),
		width(FreeImage_GetWidth(img)), height(FreeImage_GetHeight(img)),
		redMask(FreeImage_GetRedMask(img)), greenMask(FreeImage_GetGreenMask(img)),
		blueMask(FreeImage_GetGreenMask(img)),
		alphaMask(format == Format::RGBA8 ? FI_RGBA_ALPHA_MASK : 0), redShift(0),
		greenShift(0), blueShift(0), alphaShift(0)
	{
		if (redMask == FI_RGBA_RED)
			redShift = FI_RGBA_RED_SHIFT;
		else if (redMask == FI16_555_RED_MASK)
			redShift = FI16_555_RED_SHIFT;
		else if (redMask == FI16_565_RED_MASK)
			redShift = FI16_565_RED_SHIFT;

		if (greenMask == FI_RGBA_GREEN)
			greenShift = FI_RGBA_GREEN_SHIFT;
		else if (greenMask == FI16_555_GREEN_MASK)
			greenShift = FI16_555_GREEN_SHIFT;
		else if (greenMask == FI16_565_GREEN_MASK)
			greenShift = FI16_565_GREEN_SHIFT;

		if (blueMask == FI_RGBA_BLUE)
			blueShift = FI_RGBA_BLUE_SHIFT;
		else if (blueMask == FI16_555_BLUE_MASK)
			blueShift = FI16_555_BLUE_SHIFT;
		else if (blueMask == FI16_565_BLUE_MASK)
			blueShift = FI16_565_BLUE_SHIFT;

		if (alphaMask == FI_RGBA_ALPHA_MASK)
			alphaShift = FI_RGBA_ALPHA_SHIFT;
	}

	~Impl()
	{
		if (image)
			FreeImage_Unload(image);
	}

	Impl(const Impl& other) = delete;
	Impl& operator=(const Impl& other) = delete;

	FIBITMAP* image;
	Format format;
	ColorSpace colorSpace;
	unsigned int bitsPerPixel;
	unsigned int width;
	unsigned int height;
	std::uint32_t redMask;
	std::uint32_t greenMask;
	std::uint32_t blueMask;
	std::uint32_t alphaMask;
	unsigned int redShift;
	unsigned int greenShift;
	unsigned int blueShift;
	unsigned int alphaShift;
};

Image::Image()
{
	freeImageInitialize();
}

Image::Image(const char* fileName, ColorSpace colorSpace)
	: Image()
{
	load(fileName, colorSpace);
}

Image::Image(std::istream& stream, ColorSpace colorSpace)
	: Image()
{
	load(stream, colorSpace);
}

Image::Image(const void* data, std::size_t size, ColorSpace colorSpace)
	: Image()
{
	load(data, size, colorSpace);
}

Image::Image(Format format, unsigned int width, unsigned int height, ColorSpace colorSpace)
	: Image()
{
	initialize(format, width, height, colorSpace);
}

Image::~Image() = default;

Image::Image(const Image& other)
{
	if (other.m_impl)
	{
		FIBITMAP* image = FreeImage_Clone(other.m_impl->image);
		if (image)
			m_impl.reset(Impl::create(image, other.m_impl->colorSpace, other.m_impl->format));
	}
}

Image::Image(Image&& other) noexcept = default;

Image& Image::operator=(const Image& other)
{
	if (this == &other)
		return *this;

	reset();
	if (other.m_impl)
	{
		FIBITMAP* image = FreeImage_Clone(other.m_impl->image);
		if (image)
			m_impl.reset(Impl::create(image, other.m_impl->colorSpace, other.m_impl->format));
	}
	return *this;
}

Image& Image::operator=(Image&& other) noexcept = default;

bool Image::isValid() const
{
	return m_impl != nullptr;
}

Image::operator bool() const
{
	return m_impl != nullptr;
}

bool Image::load(const char* fileName, ColorSpace colorSpace)
{
	reset();

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName);
	if (format == FIF_UNKNOWN)
		return false;

	m_impl.reset(Impl::create(FreeImage_Load(format, fileName), colorSpace, Format::Invalid));
	return m_impl != nullptr;
}

bool Image::load(std::istream& stream, ColorSpace colorSpace)
{
	// Seeking is needed to detect the file type.
	if (stream.tellg() < 0)
	{
		std::vector<std::uint8_t> data;
		readStreamData(data, stream);
		return load(data.data(), data.size(), colorSpace);
	}

	reset();

	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromHandle(&istreamIO, &stream);
	if (format == FIF_UNKNOWN)
		return false;

	m_impl.reset(Impl::create(FreeImage_LoadFromHandle(format, &istreamIO, &stream), colorSpace,
		Format::Invalid));
	return m_impl != nullptr;
}

bool Image::load(const void* data, std::size_t size, ColorSpace colorSpace)
{
	reset();

	FIMEMORY* memoryStream = FreeImage_OpenMemory((BYTE*)data, (DWORD)size);
	if (!memoryStream)
		return false;

	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(memoryStream);
	if (format == FIF_UNKNOWN)
	{
		FreeImage_CloseMemory(memoryStream);
		return false;
	}

	m_impl.reset(Impl::create(FreeImage_LoadFromMemory(format, memoryStream), colorSpace,
		Format::Invalid));
	FreeImage_CloseMemory(memoryStream);
	return m_impl != nullptr;
}

bool Image::save(const char* fileName)
{
	if (!m_impl)
		return false;

	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(fileName);
	if (format == FIF_UNKNOWN)
		return false;

	return FreeImage_Save(format, m_impl->image, fileName) != false;
}

bool Image::save(std::ostream& stream, const char* fileName)
{
	if (!m_impl)
		return false;

	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(fileName);
	if (format == FIF_UNKNOWN)
		return false;

	return FreeImage_SaveToHandle(format, m_impl->image, &ostreamIO, &stream) != false;
}

bool Image::save(std::vector<std::uint8_t>& outData, const char* fileName)
{
	// Implementations of stringstream typically don't differentiate between text and binary, but
	// the standard doesn't guarantee this.
	std::stringstream stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	if (!save(stream, fileName))
		return false;

	readStreamData(outData, stream);
	return true;
}

bool Image::initialize(Format format, unsigned int width, unsigned int height,
	ColorSpace colorSpace)
{
	reset();

	unsigned int bpp, redMask, greenMask, blueMask;
	FREE_IMAGE_TYPE type = getFreeImageFormat(format, bpp, redMask, greenMask, blueMask);
	if (type == FIT_UNKNOWN)
		return false;

	m_impl.reset(Impl::create(FreeImage_AllocateT(type, width, height, bpp, redMask, greenMask,
		blueMask), colorSpace, format));
	return m_impl != nullptr;
}

void Image::reset()
{
	m_impl.reset();
}

Image::Format Image::format() const
{
	if (!m_impl)
		return Format::Invalid;

	return m_impl->format;
}

ColorSpace Image::colorSpace() const
{
	if (!m_impl)
		return ColorSpace::Linear;

	return m_impl->colorSpace;
}

unsigned int Image::bitsPerPixel() const
{
	if (!m_impl)
		return 0;

	return m_impl->bitsPerPixel;
}

unsigned int Image::width() const
{
	if (!m_impl)
		return 0;

	return m_impl->width;
}

unsigned int Image::height() const
{
	if (!m_impl)
		return 0;

	return m_impl->height;
}

std::uint32_t Image::redMask() const
{
	if (!m_impl)
		return 0;

	return m_impl->redMask;
}

unsigned int Image::redShift() const
{
	if (!m_impl)
		return 0;

	return m_impl->redShift;
}

std::uint32_t Image::greenMask() const
{
	if (!m_impl)
		return 0;

	return m_impl->greenMask;
}

unsigned int Image::greenShift() const
{
	if (!m_impl)
		return 0;

	return m_impl->greenShift;
}

std::uint32_t Image::blueMask() const
{
	if (!m_impl)
		return 0;

	return m_impl->blueMask;
}

unsigned int Image::blueShift() const
{
	if (!m_impl)
		return 0;

	return m_impl->blueShift;
}

std::uint32_t Image::alphaMask() const
{
	if (!m_impl)
		return 0;

	return m_impl->alphaMask;
}

unsigned int Image::alphaShift() const
{
	if (!m_impl)
		return 0;

	return m_impl->alphaShift;
}

void* Image::scanline(unsigned int y)
{
	if (!m_impl || y >= m_impl->height)
		return nullptr;

	return getScanlineImpl(m_impl->image, m_impl->height, y);
}

const void* Image::scanline(unsigned int y) const
{
	if (!m_impl || y >= m_impl->height)
		return nullptr;

	return getScanlineImpl(m_impl->image, m_impl->height, y);
}

bool Image::getPixel(ColorRGBAd& outColor, unsigned int x, unsigned int y) const
{
	if (!m_impl || x >= m_impl->width || y >= m_impl->height)
		return false;

	return getPixelImpl(outColor, m_impl->format, getScanlineImpl(m_impl->image, m_impl->height, y),
		x);
}

bool Image::setPixel(unsigned int x, unsigned int y, const ColorRGBAd& color, bool convertGrayscale)
{
	if (!m_impl || x >= m_impl->width || y >= m_impl->height)
		return false;

	void* scanline = getScanlineImpl(m_impl->image, m_impl->height, y);
	if (convertGrayscale)
		return setPixelImpl(m_impl->format, scanline, x, color);
	return setPixelNoGrayscaleImpl(m_impl->format, scanline, x, color);
}

Image Image::convert(Format dstFormat, bool convertGrayscale) const
{
	Image image;
	if (!m_impl)
		return image;

	Format srcFormat = m_impl->format;
	if (srcFormat == dstFormat)
	{
		image = *this;
		return image;
	}

	unsigned int bpp, redMask, greenMask, blueMask;
	FREE_IMAGE_TYPE type = getFreeImageFormat(dstFormat, bpp, redMask, greenMask, blueMask);
	if (type == FIT_UNKNOWN)
		return image;

	// Special case: UInt16 type gets treated by FreeImage as Gray16, so always use fallback
	// for UInt16.
	if (srcFormat != Format::UInt16)
	{
		switch (dstFormat)
		{
			case Format::Gray8:
				if (convertGrayscale || isGrayscaleFormat(srcFormat))
				{
					image.m_impl.reset(Impl::create(
						FreeImage_ConvertTo8Bits(m_impl->image), m_impl->colorSpace, dstFormat));
				}
				break;
			case Format::Gray16:
				if (convertGrayscale || isGrayscaleFormat(srcFormat))
				{
					image.m_impl.reset(Impl::create(
						FreeImage_ConvertToType(m_impl->image, FIT_UINT16), m_impl->colorSpace,
						dstFormat));
				}
				break;
			case Format::RGB5:
				image.m_impl.reset(Impl::create(
					FreeImage_ConvertTo16Bits555(m_impl->image), m_impl->colorSpace, dstFormat));
				break;
			case Format::RGB565:
				image.m_impl.reset(Impl::create(
					FreeImage_ConvertTo16Bits565(m_impl->image), m_impl->colorSpace, dstFormat));
				break;
			case Format::RGB8:
				image.m_impl.reset(Impl::create(
					FreeImage_ConvertTo24Bits(m_impl->image), m_impl->colorSpace, dstFormat));
				break;
			case Format::RGB16:
				image.m_impl.reset(Impl::create(
					FreeImage_ConvertToRGB16(m_impl->image), m_impl->colorSpace, dstFormat));
				break;
			case Format::RGBF:
				// NOTE: Don't use FreeImage conversion for float to float conversion to avoid
				// clamping HDR images.
				if (!needFloatConvertFallback(srcFormat))
				{
					image.m_impl.reset(Impl::create(
						FreeImage_ConvertToRGBF(m_impl->image), m_impl->colorSpace, dstFormat));
				}
				break;
			case Format::RGBA8:
				image.m_impl.reset(Impl::create(
					FreeImage_ConvertTo32Bits(m_impl->image), m_impl->colorSpace, dstFormat));
				break;
			case Format::RGBA16:
				image.m_impl.reset(Impl::create(
					FreeImage_ConvertToRGBA16(m_impl->image), m_impl->colorSpace, dstFormat));
				break;
			case Format::RGBAF:
				// NOTE: Don't use FreeImage conversion for float to float conversion to avoid
				// clamping HDR images.
				if (!needFloatConvertFallback(srcFormat))
				{
					image.m_impl.reset(Impl::create(
						FreeImage_ConvertToRGBAF(m_impl->image), m_impl->colorSpace, dstFormat));
				}
				break;
			case Format::Int16:
				image.m_impl.reset(Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_INT16),
					m_impl->colorSpace, dstFormat));
				break;
			case Format::UInt16:
				// NOTE: FreeImage treates UInt16 as a normalized integer type.
				break;
			case Format::Int32:
				image.m_impl.reset(Impl::create(
					FreeImage_ConvertToType(m_impl->image, FIT_INT32), m_impl->colorSpace,
					dstFormat));
				break;
			case Format::UInt32:
				image.m_impl.reset(Impl::create( FreeImage_ConvertToType(m_impl->image, FIT_UINT32),
					m_impl->colorSpace, dstFormat));
				break;
			case Format::Float:
				// NOTE: Don't use FreeImage conversion for float to float conversion to avoid
				// clamping HDR images. Also need to use fallback if not converting to grayscale.
				if ((convertGrayscale || isGrayscaleFormat(srcFormat)) &&
					!needFloatConvertFallback(srcFormat))
				{
					image.m_impl.reset(Impl::create(
						FreeImage_ConvertToType(m_impl->image, FIT_FLOAT), m_impl->colorSpace,
						dstFormat));
				}
				break;
			case Format::Double:
				// NOTE: Don't use FreeImage conversion for float to float conversion to avoid
				// clamping HDR images. Also need to use fallback if not converting to grayscale.
				if ((convertGrayscale || isGrayscaleFormat(srcFormat)) &&
					!needFloatConvertFallback(srcFormat))
				{
					image.m_impl.reset(Impl::create(
						FreeImage_ConvertToType(m_impl->image, FIT_DOUBLE), m_impl->colorSpace,
						dstFormat));
				}
				break;
			case Format::Complex:
				// NOTE: Complex conversions within FreeImage will only convert the first color
				// channel.
				break;
			case Format::Invalid:
				break;
		}
	}

	if (!image)
	{
		// Fall back to generic conversion if FreeImage couldn't perform the conversion.
		image.initialize(dstFormat, m_impl->width, m_impl->height, m_impl->colorSpace);
		if (!image)
			return image;

		ColorRGBAd color = {0.0, 0.0, 0.0, 0.0};
		// Never convert grayscale when going from complex type.
		if (convertGrayscale && srcFormat != Format::Complex)
		{
			for (unsigned int y = 0; y < m_impl->height; ++y)
			{
				const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
				void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, y);
				for (unsigned int x = 0; x < m_impl->width; ++x)
				{
					getPixelImpl(color, m_impl->format, srcScanline, x);
					setPixelImpl(image.m_impl->format, dstScanline, x, color);
				}
			}
		}
		else
		{
			for (unsigned int y = 0; y < m_impl->height; ++y)
			{
				const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
				void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, y);
				for (unsigned int x = 0; x < m_impl->width; ++x)
				{
					getPixelImpl(color, m_impl->format, srcScanline, x);
					setPixelNoGrayscaleImpl(image.m_impl->format, dstScanline, x, color);
				}
			}
		}
	}

	return image;
}

Image Image::resize(unsigned int width, unsigned int height, ResizeFilter filter) const
{
	Image image;
	if (!m_impl || width == 0 || height == 0)
		return image;

	if (width == m_impl->width && height == m_impl->height)
	{
		image = *this;
		return image;
	}

	// Resize in linear space.
	if (m_impl->colorSpace != ColorSpace::Linear)
	{
		image = *this;
		image.changeColorSpace(ColorSpace::Linear);
		image = image.resize(width, height, filter);
		image.changeColorSpace(m_impl->colorSpace);
		return image;
	}

	FREE_IMAGE_FILTER fiFilter = FILTER_BOX;
	switch (filter)
	{
		case ResizeFilter::Box:
			fiFilter = FILTER_BOX;
			break;
		case ResizeFilter::Linear:
			fiFilter = FILTER_BILINEAR;
			break;
		case ResizeFilter::Cubic:
			fiFilter = FILTER_BICUBIC;
			break;
		case ResizeFilter::CatmullRom:
			fiFilter = FILTER_CATMULLROM;
			break;
		case ResizeFilter::BSpline:
			fiFilter = FILTER_BSPLINE;
			break;
	}

	switch (m_impl->format)
	{
		case Format::Int16:
		case Format::Int32:
		case Format::UInt32:
		case Format::Double:
		case Format::Complex:
			break;
		default:
			image.m_impl.reset(Impl::create(
				FreeImage_Rescale(m_impl->image, width, height, fiFilter), m_impl->colorSpace,
				m_impl->format));
			break;
	}

	if (!image)
	{
		// Fallback behavior
		double invScaleX = static_cast<double>(m_impl->width)/width;
		double invScaleY = static_cast<double>(m_impl->height)/height;
		double offsetX = std::max(invScaleX, 1.0);
		double offsetY = std::max(invScaleY, 1.0);
		double filterScaleX = 1.0/offsetX;
		double filterScaleY = 1.0/offsetY;

		switch (filter)
		{
			case ResizeFilter::Box:
				image.initialize(m_impl->format, width, height, m_impl->colorSpace);
				if (!image)
					return image;

				offsetX *= 0.5;
				offsetY *= 0.5;
				for (unsigned int y = 0; y < height; ++y)
				{
					double centerY = (y + 0.5)*invScaleY;
					unsigned int top = std::max(static_cast<int>(centerY - offsetY + 0.5), 0);
					unsigned int bottom = std::min(
						static_cast<unsigned int>(centerY + offsetY + 0.5), m_impl->height);

					void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, y);
					for (unsigned int x = 0; x < width; ++x)
					{
						double centerX = (x + 0.5)*invScaleX;
						unsigned int left = std::max(
							static_cast<int>(centerX - offsetX + 0.5), 0);
						unsigned int right = std::min(
							static_cast<unsigned int>(centerX + offsetX + 0.5), m_impl->width);

						ColorRGBAd color = {0, 0, 0, 0};
						unsigned int totalScale = 0;
						for (unsigned int i = top; i < bottom; ++i)
						{
							if (std::abs(i + 0.5 - centerY)*filterScaleY > 0.5)
								continue;

							const void* srcScanline = FreeImage_GetScanLine(m_impl->image, i);
							for (unsigned int j = left; j < right; ++j)
							{
								if (std::abs(j + 0.5 - centerX)*filterScaleX > 0.5)
									continue;

								ColorRGBAd curColor;
								getPixelImpl(curColor, m_impl->format, srcScanline, j);

								color.r += curColor.r;
								color.g += curColor.g;
								color.b += curColor.b;
								color.a += curColor.a;
								++totalScale;
							}
						}

						color.r /= totalScale;
						color.g /= totalScale;
						color.b /= totalScale;
						color.a /= totalScale;
						setPixelImpl(image.m_impl->format, dstScanline, x, color);
					}
				}
				break;
			case ResizeFilter::Linear:
				image.initialize(m_impl->format, width, height, m_impl->colorSpace);
				if (!image)
					return image;

				for (unsigned int y = 0; y < height; ++y)
				{
					double centerY = (y + 0.5)*invScaleY;
					unsigned int top = std::max(static_cast<int>(centerY - offsetY + 0.5), 0);
					unsigned int bottom = std::min(
						static_cast<unsigned int>(centerY + offsetY + 0.5), m_impl->height);

					void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, y);
					for (unsigned int x = 0; x < width; ++x)
					{
						double centerX = (x + 0.5)*invScaleX;
						unsigned int left = std::max(
							static_cast<int>(centerX - offsetX + 0.5), 0);
						unsigned int right = std::min(
							static_cast<unsigned int>(centerX + offsetX + 0.5), m_impl->width);

						ColorRGBAd color = {0, 0, 0, 0};
						double totalScale = 0;
						for (unsigned int i = top; i < bottom; ++i)
						{
							double scaleY = std::max(
								1.0 - std::abs(i + 0.5 - centerY)*filterScaleY, 0.0);
							if (scaleY == 0.0)
								continue;

							const void* srcScanline = FreeImage_GetScanLine(m_impl->image, i);
							for (unsigned int j = left; j < right; ++j)
							{
								double scaleX = std::max(
									1.0 - std::abs(j + 0.5 - centerX)*filterScaleX, 0.0);
								if (scaleX == 0.0)
									continue;

								ColorRGBAd curColor;
								getPixelImpl(curColor, m_impl->format, srcScanline, j);

								double scale = scaleX*scaleY;
								color.r += curColor.r*scale;
								color.g += curColor.g*scale;
								color.b += curColor.b*scale;
								color.a += curColor.a*scale;
								totalScale += scale;
							}
						}

						color.r /= totalScale;
						color.g /= totalScale;
						color.b /= totalScale;
						color.a /= totalScale;
						setPixelImpl(image.m_impl->format, dstScanline, x, color);
					}
				}
				break;
			default:
				return image;
		}
	}
	return image;
}

Image Image::rotate(RotateAngle angle) const
{
	Image image;
	if (!m_impl)
		return image;

	double degrees = 0.0;
	switch (angle)
	{
		case RotateAngle::CCW90:
		case RotateAngle::CW270:
			degrees = 90.0;
			break;
		case RotateAngle::CCW180:
		case RotateAngle::CW180:
			degrees = 180.0;
			break;
		case RotateAngle::CCW270:
		case RotateAngle::CW90:
			degrees = 270.0;
			break;
	}

	image.m_impl.reset(Impl::create(FreeImage_Rotate(m_impl->image, degrees, nullptr),
		m_impl->colorSpace, m_impl->format));
	if (!image)
	{
		// Fallback behavior
		ColorRGBAd color = {};
		switch (angle)
		{
			case RotateAngle::CCW90:
			case RotateAngle::CW270:
				image.initialize(m_impl->format, m_impl->height, m_impl->width, m_impl->colorSpace);
				if (!image)
					return image;

				for (unsigned int y = 0; y < m_impl->height; ++y)
				{
					const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
					for (unsigned int x = 0; x < m_impl->width; ++x)
					{
						getPixelImpl(color, m_impl->format, srcScanline, x);
						void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, x);
						setPixelImpl(image.m_impl->format, dstScanline, image.m_impl->width - y - 1,
							color);
					}
				}
				break;
			case RotateAngle::CCW180:
			case RotateAngle::CW180:
				image.initialize(m_impl->format, m_impl->width, m_impl->height, m_impl->colorSpace);
				if (!image)
					return image;

				for (unsigned int y = 0; y < m_impl->height; ++y)
				{
					const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
					void* dstScanline = FreeImage_GetScanLine(image.m_impl->image,
						m_impl->height - y - 1);
					for (unsigned int x = 0; x < m_impl->width; ++x)
					{
						getPixelImpl(color, m_impl->format, srcScanline, x);
						setPixelImpl(image.m_impl->format, dstScanline, m_impl->width - x - 1,
							color);
					}
				}
				break;
			case RotateAngle::CCW270:
			case RotateAngle::CW90:
				image.initialize(m_impl->format, m_impl->height, m_impl->width, m_impl->colorSpace);
				if (!image)
					return image;

				for (unsigned int y = 0; y < m_impl->height; ++y)
				{
					const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
					for (unsigned int x = 0; x < m_impl->width; ++x)
					{
						getPixelImpl(color, m_impl->format, srcScanline,
							m_impl->width - x - 1);
						void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, x);
						setPixelImpl(image.m_impl->format, dstScanline, y, color);
					}
				}
				break;
		}
	}

	return image;
}

bool Image::flipHorizontal()
{
	if (!m_impl)
		return false;

	return FreeImage_FlipHorizontal(m_impl->image) != 0;
}

bool Image::flipVertical()
{
	if (!m_impl)
		return false;

	return FreeImage_FlipVertical(m_impl->image) != 0;
}

bool Image::preMultiplyAlpha()
{
	if (!m_impl)
		return false;

	ColorRGBAd color = {0.0, 0.0, 0.0, 0.0};
	switch (m_impl->format)
	{
		case Format::RGBA8:
		case Format::RGBA16:
		case Format::RGBAF:
			for (unsigned int y = 0; y < m_impl->height; ++y)
			{
				void* scanline = FreeImage_GetScanLine(m_impl->image, y);
				for (unsigned int x = 0; x < m_impl->width; ++x)
				{
					getPixelImpl(color, m_impl->format, scanline, x);

					// Pre-multiply in linear space.
					if (m_impl->colorSpace == ColorSpace::sRGB)
					{
						color.r = sRGBToLinear(color.r);
						color.g = sRGBToLinear(color.g);
						color.b = sRGBToLinear(color.b);
					}

					color.r *= color.a;
					color.g *= color.a;
					color.b *= color.a;

					if (m_impl->colorSpace == ColorSpace::sRGB)
					{
						color.r = linearToSRGB(color.r);
						color.g = linearToSRGB(color.g);
						color.b = linearToSRGB(color.b);
					}

					setPixelImpl(m_impl->format, scanline, x, color);
				}
			}
		default:
			break;
	}
	return true;
}

bool Image::changeColorSpace(ColorSpace colorSpace)
{
	if (!m_impl)
		return false;

	if (colorSpace == m_impl->colorSpace)
		return true;

	ColorRGBAd color = {0.0, 0.0, 0.0, 0.0};
	if (colorSpace == ColorSpace::Linear)
	{
		assert(m_impl->colorSpace == ColorSpace::sRGB);
		for (unsigned int y = 0; y < m_impl->height; ++y)
		{
			void* scanline = FreeImage_GetScanLine(m_impl->image, y);
			for (unsigned int x = 0; x < m_impl->width; ++x)
			{
				getPixelImpl(color, m_impl->format, scanline, x);
				color.r = sRGBToLinear(color.r);
				color.g = sRGBToLinear(color.g);
				color.b = sRGBToLinear(color.b);
				setPixelImpl(m_impl->format, scanline, x, color);
			}
		}
	}
	else
	{
		assert(colorSpace == ColorSpace::sRGB);
		assert(m_impl->colorSpace == ColorSpace::Linear);
		for (unsigned int y = 0; y < m_impl->height; ++y)
		{
			void* scanline = FreeImage_GetScanLine(m_impl->image, y);
			for (unsigned int x = 0; x < m_impl->width; ++x)
			{
				getPixelImpl(color, m_impl->format, scanline, x);
				color.r = linearToSRGB(color.r);
				color.g = linearToSRGB(color.g);
				color.b = linearToSRGB(color.b);
				setPixelImpl(m_impl->format, scanline, x, color);
			}
		}
	}
	m_impl->colorSpace = colorSpace;

	return true;
}

bool Image::grayscale()
{
	if (!m_impl)
		return false;

	ColorRGBAd color = {0.0, 0.0, 0.0, 0.0};
	for (unsigned int y = 0; y < m_impl->height; ++y)
	{
		void* scanline = FreeImage_GetScanLine(m_impl->image, y);
		for (unsigned int x = 0; x < m_impl->width; ++x)
		{
			getPixelImpl(color, m_impl->format, scanline, x);

			// Do the conversion in linear space.
			if (m_impl->colorSpace == ColorSpace::sRGB)
			{
				color.r = sRGBToLinear(color.r);
				color.g = sRGBToLinear(color.g);
				color.b = sRGBToLinear(color.b);
			}

			double grayscale = toGrayscale(color.r, color.g, color.b);

			if (m_impl->colorSpace == ColorSpace::sRGB)
				grayscale = linearToSRGB(grayscale);
			color.r = color.g = color.b = grayscale;

			setPixelImpl(m_impl->format, scanline, x, color);
		}
	}

	return true;
}

bool Image::swizzle(Channel red, Channel green, Channel blue, Channel alpha)
{
	if (!m_impl)
		return false;

	for (unsigned int y = 0; y < m_impl->height; ++y)
	{
		void* scanline = FreeImage_GetScanLine(m_impl->image, y);
		for (unsigned int x = 0; x < m_impl->width; ++x)
		{
			ColorRGBAd color, swzl;
			getPixelImpl(color, m_impl->format, scanline, x);
			if (static_cast<unsigned int>(red) < 4)
				swzl.r = reinterpret_cast<double*>(&color)[static_cast<unsigned int>(red)];
			else
				swzl.r = 0;
			if (static_cast<unsigned int>(green) < 4)
				swzl.g = reinterpret_cast<double*>(&color)[static_cast<unsigned int>(green)];
			else
				swzl.g = 0;
			if (static_cast<unsigned int>(blue) < 4)
				swzl.b = reinterpret_cast<double*>(&color)[static_cast<unsigned int>(blue)];
			else
				swzl.b = 0;
			if (static_cast<unsigned int>(alpha) < 4)
				swzl.a = reinterpret_cast<double*>(&color)[static_cast<unsigned int>(alpha)];
			else
				swzl.a = 1;
			setPixelImpl(m_impl->format, scanline, x, swzl);
		}
	}

	return true;
}

Image Image::createNormalMap(NormalOptions options, double height, Format dstFormat)
{
	Image image;
	if (!m_impl)
		return image;

	if (!image.initialize(dstFormat, m_impl->width, m_impl->height, m_impl->colorSpace))
		return image;

	for (unsigned int y = 0; y < m_impl->height; ++y)
	{
		const void* scanline1 = FreeImage_GetScanLine(m_impl->image, y);

		double distY = 2.0;
		const void* scanline0;
		if (y == 0)
		{
			if (options & NormalOptions::WrapY)
				scanline0 = FreeImage_GetScanLine(m_impl->image, m_impl->height - 1);
			else
			{
				scanline0 = scanline1;
				distY = 1.0;
			}
		}
		else
			scanline0 = FreeImage_GetScanLine(m_impl->image, y - 1);

		const void* scanline2;
		if (y == m_impl->height - 1)
		{
			if (options & NormalOptions::WrapY)
				scanline2 = FreeImage_GetScanLine(m_impl->image, 0);
			else
			{
				scanline2 = scanline1;
				distY = 1.0;
			}
		}
		else
			scanline2 = FreeImage_GetScanLine(m_impl->image, y + 1);

		void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, y);

		for (unsigned int x = 0; x < m_impl->width; ++x)
		{
			ColorRGBAd curColor0, curColor1;
			getPixelImpl(curColor0, m_impl->format, scanline0, x);
			getPixelImpl(curColor1, m_impl->format, scanline2, x);
			double dy = (curColor0.r - curColor1.r)*height/distY;

			double distX = 2.0;
			if (x == 0)
			{
				if (options & NormalOptions::WrapX)
					getPixelImpl(curColor0, m_impl->format, scanline1, m_impl->width - 1);
				else
				{
					getPixelImpl(curColor0, m_impl->format, scanline1, x);
					distX = 1.0;
				}
			}
			else
				getPixelImpl(curColor0, m_impl->format, scanline1, x - 1);

			if (x == m_impl->width - 1)
			{
				if (options & NormalOptions::WrapX)
					getPixelImpl(curColor1, m_impl->format, scanline1, 0);
				else
				{
					getPixelImpl(curColor1, m_impl->format, scanline1, x);
					distX = 1.0;
				}
			}
			else
				getPixelImpl(curColor1, m_impl->format, scanline1, x + 1);

			double dx = (curColor0.r - curColor1.r)*height/distX;

			ColorRGBAd normal;
			double len = std::sqrt(dx*dx + dy*dy + 1);
			normal.r = dx/len;
			normal.g = dy/len;
			normal.b = 1.0/len;
			normal.a = 1.0;
			if (!(options & NormalOptions::KeepSign))
			{
				normal.r = normal.r*0.5 + 0.5;
				normal.g = normal.g*0.5 + 0.5;
				normal.b = normal.b*0.5 + 0.5;
			}
			setPixelImpl(dstFormat, dstScanline, x, normal);
		}
	}

	return image;
}

} // namespace cuttlefish
