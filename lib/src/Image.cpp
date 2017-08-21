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

#include <cuttlefish/Image.h>

#include <cuttlefish/Color.h>
#include <FreeImage.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>

namespace cuttlefish
{

static std::atomic<unsigned int> initializeCount(0);
static void initializeLib()
{
	if (initializeCount++ == 0)
		FreeImage_Initialise();
}

static void shutdownLib()
{
	if (--initializeCount == 0)
		FreeImage_DeInitialise();
}

static Image::Format getFormat(FIBITMAP* image)
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
			return Image::Format::UInt16;
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

static FREE_IMAGE_TYPE getFreeImageFormat(Image::Format format, unsigned int& bpp,
	unsigned int& redMask, unsigned int& greenMask, unsigned int& blueMask)
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
		default:
			return FIT_UNKNOWN;
	}
}

static double toDoubleNorm(std::uint8_t value)
{
	return value/255.0;
}

static double toDoubleNorm(std::uint16_t value)
{
	return value/65535.0;
}

static double toDoubleNorm5(unsigned int value)
{
	return value/31.0;
}

static double toDoubleNorm6(unsigned int value)
{
	return value/63.0;
}

static double clamp(double d)
{
	return std::max(std::min(d, 1.0), 0.0);
}

template <typename T>
static T clamp(T v)
{
	return std::max(std::min(v, std::numeric_limits<T>::max()), std::numeric_limits<T>::lowest());
}

template <typename T>
static T fromDoubleNorm(double d)
{
	return static_cast<T>(std::round(clamp(d)*std::numeric_limits<T>::max()));
}

static std::uint8_t fromDoubleNorm5(double d)
{
	return static_cast<std::uint8_t>(std::round(clamp(d)*31));
}

static std::uint8_t fromDoubleNorm6(double d)
{
	return static_cast<std::uint8_t>(std::round(clamp(d)*63));
}

static double toLinear(double c)
{
	// sRGB to linear
	if (c <= 0.04045)
		return c/12.92;
	return std::pow((c + 0.055)/1.055, 2.4);
}

static bool getPixelImpl(ColorRGBAd& outColor, Image::Format format, const void* scanline,
	unsigned int x)
{
	switch (format)
	{
		case Image::Format::Gray8:
			outColor.r = outColor.g = outColor.b = toDoubleNorm(
				reinterpret_cast<const std::uint8_t*>(scanline)[x]);
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
		default:
			return false;
	}
}

static bool setPixelImpl(Image::Format format, void* scanline, unsigned int x,
	const ColorRGBAd& color)
{
	switch (format)
	{
		case Image::Format::Gray8:
			reinterpret_cast<std::uint8_t*>(scanline)[x] =
				fromDoubleNorm<std::uint8_t>(toGrayscale(color.r, color.g, color.b));
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
		default:
			return false;
	}
}

struct Image::Impl
{
	static Impl* create(FIBITMAP* image)
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

		Format format = getFormat(image);
		if (format == Format::Invalid)
		{
			FreeImage_Unload(image);
			return nullptr;
		}

		return new Impl(image, format);
	}

	Impl(FIBITMAP* img, Format format)
		: image(img), format(format), bitsPerPixel(FreeImage_GetBPP(img)),
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

	FIBITMAP* image;
	Format format;
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
	: m_impl(nullptr)
{
	initializeLib();
}

Image::Image(const char* fileName)
	: Image()
{
	load(fileName);
}

Image::Image(const std::uint8_t* data, std::size_t size)
	: Image()
{
	load(data, size);
}

Image::Image(Format format, unsigned int width, unsigned int height)
	: Image()
{
	initialize(format, width, height);
}

Image::~Image()
{
	shutdownLib();
	delete m_impl;
}

Image::Image(const Image& other)
	: m_impl(nullptr)
{
	initializeLib();
	if (other.m_impl)
	{
		FIBITMAP* image = FreeImage_Clone(other.m_impl->image);
		if (image)
			m_impl = Impl::create(image);
	}
}

Image::Image(Image&& other)
{
	initializeLib();
	m_impl = other.m_impl;
	other.m_impl = nullptr;
}

Image& Image::operator=(const Image& other)
{
	if (this == &other)
		return *this;

	reset();
	if (other.m_impl)
	{
		FIBITMAP* image = FreeImage_Clone(other.m_impl->image);
		if (image)
			m_impl = Impl::create(image);
	}
	return *this;
}

Image& Image::operator=(Image&& other)
{
	reset();
	m_impl = other.m_impl;
	other.m_impl = nullptr;
	return *this;
}

bool Image::isValid() const
{
	return m_impl != nullptr;
}

Image::operator bool() const
{
	return m_impl != nullptr;
}

bool Image::load(const char* fileName)
{
	reset();

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName);
	if (format == FIF_UNKNOWN)
		return false;

	m_impl = Impl::create(FreeImage_Load(format, fileName));
	return m_impl != nullptr;
}

bool Image::load(const std::uint8_t* data, std::size_t size)
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

	m_impl = Impl::create(FreeImage_LoadFromMemory(format, memoryStream));
	FreeImage_CloseMemory(memoryStream);
	return m_impl != nullptr;
}

bool Image::initialize(Format format, unsigned int width, unsigned int height)
{
	reset();

	unsigned int bpp, redMask, greenMask, blueMask;
	FREE_IMAGE_TYPE type = getFreeImageFormat(format, bpp, redMask, greenMask, blueMask);
	if (type == FIT_UNKNOWN)
		return false;

	m_impl = Impl::create(FreeImage_AllocateT(type, width, height, bpp, redMask, greenMask,
		blueMask));
	return m_impl != nullptr;
}

void Image::reset()
{
	delete m_impl;
	m_impl = nullptr;
}

Image::Format Image::format() const
{
	if (!m_impl)
		return Format::Invalid;

	return m_impl->format;
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

	return FreeImage_GetScanLine(m_impl->image, y);
}

const void* Image::scanline(unsigned int y) const
{
	if (!m_impl || y >= m_impl->height)
		return nullptr;

	return FreeImage_GetScanLine(m_impl->image, y);
}

bool Image::getPixel(ColorRGBAd& outColor, unsigned int x, unsigned int y) const
{
	if (!m_impl || x >= m_impl->width || y >= m_impl->height)
		return false;

	return getPixelImpl(outColor, m_impl->format, FreeImage_GetScanLine(m_impl->image, y), x);
}

bool Image::setPixel(unsigned int x, unsigned int y, const ColorRGBAd& color)
{
	if (!m_impl || x >= m_impl->width || y >= m_impl->height)
		return false;

	return setPixelImpl(m_impl->format, FreeImage_GetScanLine(m_impl->image, y), x, color);
}

Image Image::convert(Format format) const
{
	Image image;
	if (!m_impl)
		return image;

	unsigned int bpp, redMask, greenMask, blueMask;
	FREE_IMAGE_TYPE type = getFreeImageFormat(format, bpp, redMask, greenMask, blueMask);
	if (type == FIT_UNKNOWN)
		return image;

	switch (format)
	{
		case Format::Gray8:
			image.m_impl = Impl::create(FreeImage_ConvertTo8Bits(m_impl->image));
			break;
		case Format::RGB5:
			image.m_impl = Impl::create(FreeImage_ConvertTo16Bits555(m_impl->image));
			break;
		case Format::RGB565:
			image.m_impl = Impl::create(FreeImage_ConvertTo16Bits565(m_impl->image));
			break;
		case Format::RGB8:
			image.m_impl = Impl::create(FreeImage_ConvertTo24Bits(m_impl->image));
			break;
		case Format::RGB16:
			image.m_impl = Impl::create(FreeImage_ConvertToRGB16(m_impl->image));
			break;
		case Format::RGBF:
			image.m_impl = Impl::create(FreeImage_ConvertToRGBF(m_impl->image));
			break;
		case Format::RGBA8:
			image.m_impl = Impl::create(FreeImage_ConvertTo32Bits(m_impl->image));
			break;
		case Format::RGBA16:
			image.m_impl = Impl::create(FreeImage_ConvertToRGBA16(m_impl->image));
			break;
		case Format::RGBAF:
			image.m_impl = Impl::create(FreeImage_ConvertToRGBAF(m_impl->image));
			break;
		case Format::Int16:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_INT16));
			break;
		case Format::UInt16:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_UINT16));
			break;
		case Format::Int32:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_INT32));
			break;
		case Format::UInt32:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_UINT32));
			break;
		case Format::Float:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_FLOAT));
			break;
		case Format::Double:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_DOUBLE));
			break;
		case Format::Complex:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_COMPLEX));
			break;
		default:
			break;
	}

	if (!image)
	{
		// Fall back to generic conversion if FreeImage couldn't.
		image.initialize(format, m_impl->width, m_impl->height);
		if (!image)
			return image;

		for (unsigned int y = 0; y < m_impl->height; ++y)
		{
			const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
			void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, y);
			for (unsigned int x = 0; x < m_impl->width; ++x)
			{
				ColorRGBAd color;
				getPixelImpl(color, m_impl->format, srcScanline, x);
				setPixelImpl(image.m_impl->format, dstScanline, x, color);
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
			image.m_impl = Impl::create(FreeImage_Rescale(m_impl->image, width, height, fiFilter));
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
				image.initialize(m_impl->format, width, height);
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
				image.initialize(m_impl->format, width, height);
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

	image.m_impl = Impl::create(FreeImage_Rotate(m_impl->image, degrees, nullptr));
	if (!image)
	{
		// Fallback behavior
		switch (angle)
		{
			case RotateAngle::CCW90:
			case RotateAngle::CW270:
				image.initialize(m_impl->format, m_impl->height, m_impl->width);
				if (!image)
					return image;

				for (unsigned int y = 0; y < m_impl->height; ++y)
				{
					const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
					for (unsigned int x = 0; x < m_impl->width; ++x)
					{
						ColorRGBAd color;
						getPixelImpl(color, m_impl->format, srcScanline, x);
						void* dstScanline = FreeImage_GetScanLine(image.m_impl->image, x);
						setPixelImpl(image.m_impl->format, dstScanline,
							image.m_impl->width - y - 1, color);
					}
				}
				break;
			case RotateAngle::CCW180:
			case RotateAngle::CW180:
				image.initialize(m_impl->format, m_impl->width, m_impl->height);
				if (!image)
					return image;

				for (unsigned int y = 0; y < m_impl->height; ++y)
				{
					const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
					void* dstScanline = FreeImage_GetScanLine(image.m_impl->image,
						m_impl->height - y - 1);
					for (unsigned int x = 0; x < m_impl->width; ++x)
					{
						ColorRGBAd color;
						getPixelImpl(color, m_impl->format, srcScanline, x);
						setPixelImpl(image.m_impl->format, dstScanline, m_impl->width - x - 1,
							color);
					}
				}
				break;
			case RotateAngle::CCW270:
			case RotateAngle::CW90:
				image.initialize(m_impl->format, m_impl->height, m_impl->width);
				if (!image)
					return image;

				for (unsigned int y = 0; y < m_impl->height; ++y)
				{
					const void* srcScanline = FreeImage_GetScanLine(m_impl->image, y);
					for (unsigned int x = 0; x < m_impl->width; ++x)
					{
						ColorRGBAd color;
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
					ColorRGBAd color;
					getPixelImpl(color, m_impl->format, scanline, x);
					color.r *= color.a;
					color.g *= color.a;
					color.b *= color.a;
					setPixelImpl(m_impl->format, scanline, x, color);
				}
			}
		default:
			break;
	}
	return true;
}

bool Image::linearize()
{
	if (!m_impl)
		return false;

	for (unsigned int y = 0; y < m_impl->height; ++y)
	{
		void* scanline = FreeImage_GetScanLine(m_impl->image, y);
		for (unsigned int x = 0; x < m_impl->width; ++x)
		{
			ColorRGBAd color;
			getPixelImpl(color, m_impl->format, scanline, x);
			color.r = toLinear(color.r);
			color.g = toLinear(color.g);
			color.b = toLinear(color.b);
			setPixelImpl(m_impl->format, scanline, x, color);
		}
	}

	return true;
}

} // namespace cuttlefish
