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
#include <FreeImage.h>
#include <cmath>
#include <atomic>

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

static Image::PixelFormat getPixelFormat(FIBITMAP* image)
{
	switch (FreeImage_GetImageType(image))
	{
		case FIT_BITMAP:
			switch (FreeImage_GetBPP(image))
			{
				case 8:
					return Image::PixelFormat::Gray8;
				case 16:
					switch (FreeImage_GetGreenMask(image))
					{
						case FI16_555_GREEN_MASK:
							return Image::PixelFormat::RGB5;
						case FI16_565_GREEN_MASK:
							return Image::PixelFormat::RGB565;
						default:
							return Image::PixelFormat::Invalid;
					}
				case 24:
					return Image::PixelFormat::RGB8;
				case 32:
					return Image::PixelFormat::RGBA8;
				default:
					return Image::PixelFormat::Invalid;
			}
		case FIT_UINT16:
			return Image::PixelFormat::UInt16;
		case FIT_INT16:
			return Image::PixelFormat::Int16;
		case FIT_UINT32:
			return Image::PixelFormat::UInt32;
		case FIT_INT32:
			return Image::PixelFormat::Int32;
		case FIT_FLOAT:
			return Image::PixelFormat::Float;
		case FIT_DOUBLE:
			return Image::PixelFormat::Double;
		case FIT_COMPLEX:
			return Image::PixelFormat::Complex;
		case FIT_RGB16:
			return Image::PixelFormat::RGB16;
		case FIT_RGBA16:
			return Image::PixelFormat::RGBA16;;
		case FIT_RGBF:
			return Image::PixelFormat::RGBF;
		case FIT_RGBAF:
			return Image::PixelFormat::RGBAF;
		default:
			return Image::PixelFormat::Invalid;
	}
}

static FREE_IMAGE_TYPE getFreeImageFormat(Image::PixelFormat format, unsigned int& bpp,
	unsigned int& redMask, unsigned int& greenMask, unsigned int& blueMask)
{
	bpp = 0;
	redMask = 0;
	greenMask = 0;
	blueMask = 0;
	switch (format)
	{
		case Image::PixelFormat::Gray8:
			bpp = 8;
			return FIT_BITMAP;
		case Image::PixelFormat::RGB5:
			bpp = 16;
			redMask = FI16_555_RED_MASK;
			greenMask = FI16_555_GREEN_MASK;
			blueMask = FI16_555_BLUE_MASK;
			return FIT_BITMAP;
		case Image::PixelFormat::RGB565:
			bpp = 16;
			redMask = FI16_565_RED_MASK;
			greenMask = FI16_565_GREEN_MASK;
			blueMask = FI16_565_BLUE_MASK;
			return FIT_BITMAP;
		case Image::PixelFormat::RGB8:
			bpp = 24;
			return FIT_BITMAP;
		case Image::PixelFormat::RGB16:
			return FIT_RGB16;
		case Image::PixelFormat::RGBF:
			return FIT_RGBF;
		case Image::PixelFormat::RGBA8:
			bpp = 32;
			return FIT_BITMAP;
		case Image::PixelFormat::RGBA16:
			return FIT_RGBA16;
		case Image::PixelFormat::RGBAF:
			return FIT_RGBAF;
		case Image::PixelFormat::Int16:
			return FIT_INT16;
		case Image::PixelFormat::UInt16:
			return FIT_UINT16;
		case Image::PixelFormat::Int32:
			return FIT_INT32;
		case Image::PixelFormat::UInt32:
			return FIT_UINT32;
		case Image::PixelFormat::Float:
			return FIT_FLOAT;
		case Image::PixelFormat::Double:
			return FIT_DOUBLE;
		case Image::PixelFormat::Complex:
			return FIT_COMPLEX;
		case Image::PixelFormat::Invalid:
		default:
			return FIT_UNKNOWN;
	}
}

struct Image::Impl
{
	static Impl* create(FIBITMAP* image)
	{
		if (!image)
			return nullptr;

		if (FreeImage_GetPalette(image))
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

		PixelFormat format = getPixelFormat(image);
		if (format == PixelFormat::Invalid)
		{
			FreeImage_Unload(image);
			return nullptr;
		}

		return new Impl(image, format);
	}

	Impl(FIBITMAP* img, PixelFormat format)
		: image(img), pixelFormat(format), bitsPerPixel(FreeImage_GetBPP(img)),
		width(FreeImage_GetWidth(img)), height(FreeImage_GetHeight(img)),
		redMask(FreeImage_GetRedMask(img)), greenMask(FreeImage_GetGreenMask(img)),
		blueMask(FreeImage_GetGreenMask(img)),
		alphaMask(format == PixelFormat::RGBA8 ? FI_RGBA_ALPHA_MASK : 0), redShift(0),
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
	PixelFormat pixelFormat;
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

Image::Image(PixelFormat format, unsigned int width, unsigned int height)
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

bool Image::initialize(PixelFormat format, unsigned int width, unsigned int height)
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

Image::PixelFormat Image::pixelFormat() const
{
	if (!m_impl)
		return PixelFormat::Invalid;

	return m_impl->pixelFormat;
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

Image Image::convert(PixelFormat format) const
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
		case PixelFormat::Gray8:
			image.m_impl = Impl::create(FreeImage_ConvertTo8Bits(m_impl->image));
			break;
		case PixelFormat::RGB5:
			image.m_impl = Impl::create(FreeImage_ConvertTo16Bits555(m_impl->image));
			break;
		case PixelFormat::RGB565:
			image.m_impl = Impl::create(FreeImage_ConvertTo16Bits565(m_impl->image));
			break;
		case PixelFormat::RGB8:
			image.m_impl = Impl::create(FreeImage_ConvertTo24Bits(m_impl->image));
			break;
		case PixelFormat::RGB16:
			image.m_impl = Impl::create(FreeImage_ConvertToRGB16(m_impl->image));
			break;
		case PixelFormat::RGBF:
			image.m_impl = Impl::create(FreeImage_ConvertToRGBF(m_impl->image));
			break;
		case PixelFormat::RGBA8:
			image.m_impl = Impl::create(FreeImage_ConvertTo32Bits(m_impl->image));
			break;
		case PixelFormat::RGBA16:
			image.m_impl = Impl::create(FreeImage_ConvertToRGBA16(m_impl->image));
			break;
		case PixelFormat::RGBAF:
			image.m_impl = Impl::create(FreeImage_ConvertToRGBAF(m_impl->image));
			break;
		case PixelFormat::Int16:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_INT16));
			break;
		case PixelFormat::UInt16:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_UINT16));
			break;
		case PixelFormat::Int32:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_INT32));
			break;
		case PixelFormat::UInt32:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_UINT32));
			break;
		case PixelFormat::Float:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_FLOAT));
			break;
		case PixelFormat::Double:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_DOUBLE));
			break;
		case PixelFormat::Complex:
			image.m_impl = Impl::create(FreeImage_ConvertToType(m_impl->image, FIT_COMPLEX));
			break;
		default:
			break;
	}

	return image;
}

Image Image::resize(unsigned int width, unsigned int height, ResizeFilter filter) const
{
	Image image;
	if (!m_impl)
		return image;

	FREE_IMAGE_FILTER fiFilter = FILTER_BOX;
	switch (filter)
	{
		case ResizeFilter::Nearest:
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

	image.m_impl = Impl::create(FreeImage_Rescale(m_impl->image, width, height, fiFilter));
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
		case RotateAngle::CW90:
			degrees = 90.0;
			break;
		case RotateAngle::CW180:
			degrees = 180.0;
			break;
		case RotateAngle::CW270:
			degrees = 270.0;
			break;
		case RotateAngle::CCW90:
			degrees = -90.0;
			break;
		case RotateAngle::CCW180:
			degrees = -180.0;
			break;
		case RotateAngle::CCW270:
			degrees = -270.0;
			break;
	}

	image.m_impl = Impl::create(FreeImage_Rotate(m_impl->image, degrees, nullptr));
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

} // namespace cuttlefish
