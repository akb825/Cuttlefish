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

/**
 * @file
 * @brief File containing the Image class to load and manipulate images.
 */

#include <cuttlefish/Config.h>
#include <cuttlefish/Export.h>
#include <cstddef>
#include <cstdint>

namespace cuttlefish
{

struct ColorRGBAd;

/**
 * @brief Class to store the data for and manipulate source images.
 *
 * Loading an image is the first step for creating a texture. It is loaded from a source image file
 * (such as a PNG) then transformed as desired, such as resizing or flipping the image. Afterward,
 * one or more images will be added to a Texture.
 *
 * @remark The coordinate (0, 0) is the upper-left of the image.
 */
class CUTTLEFISH_EXPORT Image
{
public:
	/**
	 * @brief Enum for the pixel format of the image.
	 */
	enum class PixelFormat
	{
		Invalid, ///< Invalid format, used for errors.
		Gray8,   ///< 8-bit grayscale.
		RGB5,    ///< 6-bit per channel RGB.
		RGB565,  ///< 5, 6, and 5 bits per channel RGB.
		RGB8,    ///< 8-bit per channel RGB.
		RGB16,   ///< 16-bit per channel RGB.
		RGBF,    ///< Floating point RGB.
		RGBA8,   ///< 8-bit per channel RGBA.
		RGBA16,  ///< 16-bit per channel RGBA.
		RGBAF,   ///< Floating point RGBA.
		Int16,   ///< Signed 16-bit integer.
		UInt16,  ///< Unsigned 16-bit integer.
		Int32,   ///< Signed 32-bit integer.
		UInt32,  ///< Unsigned 32-bit integer.
		Float,   ///< Float.
		Double,  ///< Double.
		Complex  ///< Two doubles as a complex number.
	};

	/**
	 * @brief Enum for the filter to use when resizing.
	 */
	enum class ResizeFilter
	{
		Box,       ///< Averages all pixels in the box.
		Linear,    ///< Linear sampling.
		Cubic,     ///< Cubic curve sampling.
		CatmullRom ///< Catmull-Rom curve fitting.
	};

	/**
	 * @brief Enum for the angle to rotate by.
	 */
	enum class RotateAngle
	{
		CW90,   ///< 90 degrees clockwise.
		CW180,  ///< 180 degrees clockwise.
		CW270,  ///< 270 degrees clockwise.
		CCW90,  ///< 90 degrees counter-clockwise.
		CCW180, ///< 180 degrees counter-clockwise.
		CCW270  ///< 270 degrees counter-clockwise.
	};

	Image();

	/**
	 * @brief Loads an image from file.
	 * @param fileName The name of the file to load.
	 * @remark The image will be invalid if it failed to load.
	 */
	Image(const char* fileName);

	/**
	 * @brief Loads an image from data.
	 * @param data The data to load from.
	 * @param size The size of the data.
	 * @remark The image will be invalid if it failed to load.
	 */
	Image(const std::uint8_t* data, std::size_t size);

	/**
	 * @brief Initializes an empty image.
	 * @param format The pixel format.
	 * @param width The width of the image.
	 * @param height The height of the image.
	 * @remark The image will be invalid if it failed to initialize.
	 */
	Image(PixelFormat format, unsigned int width, unsigned int height);

	~Image();

	/// \{
	Image(const Image& other);
	Image(Image&& other);

	Image& operator=(const Image& other);
	Image& operator=(Image&& other);
	/// \}

	/**
	 * @brief Returns whether or not an image is valid.
	 * @return True if valid.
	 */
	bool isValid() const;

	/**
	 * @brief Returns whether or not an image is valid.
	 * @return True if valid.
	 */
	explicit operator bool() const;

	/**
	 * @brief Loads an image from file.
	 * @param fileName The name of the file to load.
	 * @return False if the image couldn't be loaded.
	 */
	bool load(const char* fileName);

	/**
	 * @brief Loads an image from data.
	 * @param data The data to load from.
	 * @param size The size of the data.
	 * @return False if the image couldn't be loaded.
	 */
	bool load(const std::uint8_t* data, std::size_t size);

	/**
	 * @brief Initializes an empty image.
	 * @param format The pixel format.
	 * @param width The width of the image.
	 * @param height The height of the image.
	 * @return False if the image couldn't be initialized.
	 */
	bool initialize(PixelFormat format, unsigned int width, unsigned int height);

	/**
	 * @brief Resets the image to an unitialized state.
	 */
	void reset();

	/**
	 * @brief Gets the pixel format of the image.
	 * @return The pixel format.
	 */
	PixelFormat pixelFormat() const;

	/**
	 * @brief Gets the number of bits per pixel in the image.
	 * @return The bits per pixel.
	 */
	unsigned int bitsPerPixel() const;

	/**
	 * @brief Gets the width of the image.
	 * @return The width in pixels.
	 */
	unsigned int width() const;

	/**
	 * @brief Gets the height of the image.
	 * @return The height in pixels.
	 */
	unsigned int height() const;

	/**
	 * @brief Gets the mask for the red channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The red channel mask.
	 */
	std::uint32_t redMask() const;

	/**
	 * @brief Gets the bit shift for the red channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The red channel shift.
	 */
	unsigned int redShift() const;

	/**
	 * @brief Gets the mask for the green channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The green channel mask.
	 */
	std::uint32_t greenMask() const;

	/**
	 * @brief Gets the bit shift for the green channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The green channel shift.
	 */
	unsigned int greenShift() const;

	/**
	 * @brief Gets the mask for the blue channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The blue channel mask.
	 */
	std::uint32_t blueMask() const;

	/**
	 * @brief Gets the bit shift for the blue channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The blue channel shift.
	 */
	unsigned int blueShift() const;

	/**
	 * @brief Gets the mask for the alpha channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The alpha channel mask.
	 */
	std::uint32_t alphaMask() const;

	/**
	 * @brief Gets the bit shift for the alpha channel for a 16 or 32 BPP RGB or RGBA format.
	 * @return The alpha channel shift.
	 */
	unsigned int alphaShift() const;

	/**
	 * @brief Gets a scanline of the image.
	 * @param y The Y coordinate of the scanline.
	 * @return The data for the scanline.
	 */
	void* scanline(unsigned int y);

	/** @copydoc scanline() */
	const void* scanline(unsigned int y) const;

	/**
	 * @brief Gets a pixel as a floating point value.
	 * @remark This will work with any image format.
	 * @param[out] outColor The color of the pixel.
	 * @param x The X coordinate of the image.
	 * @param y The Y coordinate of the image.
	 * @return False if the pixel is out of range.
	 */
	bool getPixel(ColorRGBAd& outColor, unsigned int x, unsigned int y) const;

	/**
	 * @brief Sets a pixel as a floating point value.
	 * @remark This will work with any image format.
	 * @remark Conversion to grayscale will be automatic for PixelFormat::Gray8.
	 * @param x The X coordinate of the image.
	 * @param y The Y coordinate of the image.
	 * @param color The color of the pixel.
	 * @return False if the pixel is out of range.
	 */
	bool setPixel(unsigned int x, unsigned int y, const ColorRGBAd& color);

	/**
	 * @brief Converts the image to another pixel format.
	 * @param format The new pixel format.
	 * @return The converted image.
	 */
	Image convert(PixelFormat format) const;

	/**
	 * @brief Resizes an image.
	 * @remark In some situations the format may change.
	 * @remark Int*, UInt32, Double, and Complex types may only use Nearest and Linear filters.
	 * @param width The new width of the image.
	 * @param height The new height of the image.
	 * @param filter The filter to use for resizing.
	 * @return The resized image, or an invalid image if the format cannot be resized.
	 */
	Image resize(unsigned int width, unsigned int height, ResizeFilter filter) const;

	/**
	 * @brief Rotates an image.
	 * @param angle The angle to rotate by.
	 * @return The rotated image.
	 * @return The rotated image, or an invalid image if the format cannot be rotated.
	 */
	Image rotate(RotateAngle angle) const;

	/**
	 * @brief Flips the image horizontally in place.
	 * @return False if the image was invalid.
	 */
	bool flipHorizontal();

	/**
	 * @brief Flips the image vertically in place.
	 * @return False if the image was invalid.
	 */
	bool flipVertical();

	/**
	 * @brief Pre-multiplies the alpha values with the color values in place.
	 * @return False if the image was invalid.
	 */
	bool preMultiplyAlpha();

	/**
	 * @brief Converts from sRGB color space to linear color space.
	 *
	 * This will affect every channel except the alpha channel.
	 *
	 * @remark This will reduce the precision for percieved color values, so it's not recommended
	 * for 8 bits per channel or less.
	 * @return False if the image was invalid.
	 */
	bool linearize();

private:
	struct Impl;
	Impl* m_impl;
};

} // namespace cuttlefish
