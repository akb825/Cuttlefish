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

#include <cuttlefish/Color.h>
#include <cuttlefish/Image.h>
#include <gtest/gtest.h>
#include <cmath>
#include <sstream>

// Handle different versions of gtest.
#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

namespace cuttlefish
{

struct ImageTestInfo
{
	ImageTestInfo(Image::Format _format, double _epsilon, unsigned int _channels)
		: format(_format)
		, epsilon(_epsilon)
		, channels(_channels)
	{
	}

	Image::Format format;
	double epsilon;
	unsigned int channels;
};

class ImageTest : public testing::TestWithParam<ImageTestInfo>
{
};

class ImageColorTest : public testing::TestWithParam<ImageTestInfo>
{
};

class ImageSRGBColorTest : public testing::TestWithParam<ImageTestInfo>
{
};

static ColorRGBAd getTestColor(const Image& image, unsigned int x, unsigned int y, bool divide)
{
	ColorRGBAd color;
	if (x < image.width()/2)
	{
		color.r = x;
		color.g = y;
	}
	else
	{
		color.r = y;
		color.g = x;
	}
	color.b = x + y;
	color.a = x + 2*y;
	if (divide)
	{
		if (x < image.width()/2)
		{
			color.r /= static_cast<double>(image.width() - 1);
			color.g /= static_cast<double>(image.height() - 1);
		}
		else
		{
			color.g /= static_cast<double>(image.width() - 1);
			color.r /= static_cast<double>(image.height() - 1);
		}
		color.b /= static_cast<double>(image.width() + image.height() - 2);
		color.a /= static_cast<double>(image.width() + 2*image.height() - 3);
	}

	return color;
}

static bool epsilonEqual(double expected, double value, double epsilon)
{
	return std::abs(value - expected) <= epsilon;
}

static bool colorsEqual(const ColorRGBAd& expected, const ColorRGBAd& color,
	const ImageTestInfo& info)
{
	if (info.channels == 0)
	{
		return epsilonEqual(toGrayscale(expected.r, expected.g, expected.b),
			color.r, info.epsilon);
	}

	if (!epsilonEqual(expected.r, color.r, info.epsilon))
		return false;
	if (info.channels >= 2 && !epsilonEqual(expected.g, color.g, info.epsilon))
		return false;
	if (info.channels >= 3 && !epsilonEqual(expected.b, color.b, info.epsilon))
		return false;
	if (info.channels >= 4 && !epsilonEqual(expected.a, color.a, info.epsilon))
		return false;

	return true;
}

static bool shouldDivide(Image::Format format)
{
	switch (format)
	{
		case Image::Format::Int16:
		case Image::Format::UInt16:
		case Image::Format::Int32:
		case Image::Format::UInt32:
			return false;
		default:
			return true;
	}
}

TEST(ColorTest, SRGBConversion)
{
	EXPECT_DOUBLE_EQ(0.0, linearToSRGB(0.0));
	EXPECT_DOUBLE_EQ(0.0998528227341283, linearToSRGB(0.01));
	EXPECT_DOUBLE_EQ(0.537098730483194, linearToSRGB(0.25));
	EXPECT_DOUBLE_EQ(0.8808250210903, linearToSRGB(0.75));
	EXPECT_DOUBLE_EQ(1.0, linearToSRGB(1.0));

	EXPECT_DOUBLE_EQ(0.0, sRGBToLinear(0.0));
	EXPECT_DOUBLE_EQ(0.000773993808049536, sRGBToLinear(0.01));
	EXPECT_DOUBLE_EQ(0.0508760881715568, sRGBToLinear(0.25));
	EXPECT_DOUBLE_EQ(0.522521553968392, sRGBToLinear(0.75));
	EXPECT_DOUBLE_EQ(1.0, sRGBToLinear(1.0));
}

TEST_P(ImageTest, Initialize)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));
	EXPECT_TRUE(image.isValid());
	EXPECT_EQ(imageInfo.format, image.format());
	EXPECT_EQ(10U, image.width());
	EXPECT_EQ(15U, image.height());

	image.reset();
	EXPECT_FALSE(image.isValid());
}

TEST(ImageSaveTest, SaveLoadStream)
{
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(Image::Format::RGBA8, 10, 15));
	EXPECT_TRUE(image.isValid());
	EXPECT_EQ(Image::Format::RGBA8, image.format());
	EXPECT_EQ(10U, image.width());
	EXPECT_EQ(15U, image.height());

	// Implementations of stringstream typically don't differentiate between text and binary, but
	// the standard doesn't guarantee this.
	std::stringstream stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	EXPECT_TRUE(image.save(stream, "png"));

	Image otherImage;
	EXPECT_TRUE(otherImage.load(stream));
	EXPECT_TRUE(otherImage.isValid());
	EXPECT_EQ(image.format(), otherImage.format());
	EXPECT_EQ(image.width(), otherImage.width());
	EXPECT_EQ(image.height(), otherImage.height());
}

TEST(ImageSaveTest, SaveLoadBuffer)
{
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(Image::Format::RGBA8, 10, 15));
	EXPECT_TRUE(image.isValid());
	EXPECT_EQ(Image::Format::RGBA8, image.format());
	EXPECT_EQ(10U, image.width());
	EXPECT_EQ(15U, image.height());

	std::vector<std::uint8_t> data;
	EXPECT_TRUE(image.save(data, "png"));

	Image otherImage;
	EXPECT_TRUE(otherImage.load(data.data(), data.size()));
	EXPECT_TRUE(otherImage.isValid());
	EXPECT_EQ(image.format(), otherImage.format());
	EXPECT_EQ(image.width(), otherImage.width());
	EXPECT_EQ(image.height(), otherImage.height());
}

TEST_P(ImageTest, GetSetPixel)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));

	ColorRGBAd color;
	EXPECT_FALSE(image.getPixel(color, image.width(), 0));
	EXPECT_FALSE(image.getPixel(color, 0, image.height()));
	EXPECT_FALSE(image.setPixel(image.width(), 0, color));
	EXPECT_FALSE(image.setPixel(0, image.height(), color));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			EXPECT_TRUE(image.getPixel(color, x, y));
			EXPECT_TRUE(colorsEqual(getTestColor(image, x, y, divide), color, imageInfo));
		}
	}
}

TEST(ResizeFallbackTest, Box)
{
	Image floatImage;
	EXPECT_TRUE(floatImage.initialize(Image::Format::Float, 12, 16));
	Image doubleImage;
	EXPECT_TRUE(doubleImage.initialize(Image::Format::Double, 12, 16));

	for (unsigned int y = 0; y < floatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < floatImage.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(floatImage, x, y, true);
			EXPECT_TRUE(floatImage.setPixel(x, y, color));
			EXPECT_TRUE(doubleImage.setPixel(x, y, color));
		}
	}

	Image resizedFloatImage = floatImage.resize(19, 23, Image::ResizeFilter::Box);
	Image resizedDoubleImage = doubleImage.resize(19, 23, Image::ResizeFilter::Box);
	EXPECT_EQ(19U, resizedFloatImage.width());
	EXPECT_EQ(23U, resizedFloatImage.height());
	EXPECT_EQ(19U, resizedDoubleImage.width());
	EXPECT_EQ(23U, resizedDoubleImage.height());
	for (unsigned int y = 0; y < resizedFloatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < resizedFloatImage.width(); ++x)
		{
			ColorRGBAd floatColor;
			EXPECT_TRUE(resizedFloatImage.getPixel(floatColor, x, y));
			ColorRGBAd doubleColor;
			EXPECT_TRUE(resizedDoubleImage.getPixel(doubleColor, x, y));

			EXPECT_NEAR(floatColor.r, doubleColor.r, 1e-5);
		}
	}

	resizedFloatImage = floatImage.resize(5, 9, Image::ResizeFilter::Box);
	resizedDoubleImage = doubleImage.resize(5, 9, Image::ResizeFilter::Box);
	EXPECT_EQ(5U, resizedFloatImage.width());
	EXPECT_EQ(9U, resizedFloatImage.height());
	EXPECT_EQ(5U, resizedDoubleImage.width());
	EXPECT_EQ(9U, resizedDoubleImage.height());
	for (unsigned int y = 0; y < resizedFloatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < resizedFloatImage.width(); ++x)
		{
			ColorRGBAd floatColor;
			EXPECT_TRUE(resizedFloatImage.getPixel(floatColor, x, y));
			ColorRGBAd doubleColor;
			EXPECT_TRUE(resizedDoubleImage.getPixel(doubleColor, x, y));

			EXPECT_NEAR(floatColor.r, doubleColor.r, 1e-5);
		}
	}
}

TEST(ResizeFallbackTest, Linear)
{
	Image floatImage;
	EXPECT_TRUE(floatImage.initialize(Image::Format::Float, 12, 16));
	Image doubleImage;
	EXPECT_TRUE(doubleImage.initialize(Image::Format::Double, 12, 16));

	for (unsigned int y = 0; y < floatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < floatImage.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(floatImage, x, y, true);
			EXPECT_TRUE(floatImage.setPixel(x, y, color));
			EXPECT_TRUE(doubleImage.setPixel(x, y, color));
		}
	}

	Image resizedFloatImage = floatImage.resize(19, 23, Image::ResizeFilter::Linear);
	Image resizedDoubleImage = doubleImage.resize(19, 23, Image::ResizeFilter::Linear);
	EXPECT_EQ(19U, resizedFloatImage.width());
	EXPECT_EQ(23U, resizedFloatImage.height());
	EXPECT_EQ(19U, resizedDoubleImage.width());
	EXPECT_EQ(23U, resizedDoubleImage.height());
	for (unsigned int y = 0; y < resizedFloatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < resizedFloatImage.width(); ++x)
		{
			ColorRGBAd floatColor;
			EXPECT_TRUE(resizedFloatImage.getPixel(floatColor, x, y));
			ColorRGBAd doubleColor;
			EXPECT_TRUE(resizedDoubleImage.getPixel(doubleColor, x, y));

			EXPECT_NEAR(floatColor.r, doubleColor.r, 1e-5);
		}
	}

	resizedFloatImage = floatImage.resize(5, 9, Image::ResizeFilter::Linear);
	resizedDoubleImage = doubleImage.resize(5, 9, Image::ResizeFilter::Linear);
	EXPECT_EQ(5U, resizedFloatImage.width());
	EXPECT_EQ(9U, resizedFloatImage.height());
	EXPECT_EQ(5U, resizedDoubleImage.width());
	EXPECT_EQ(9U, resizedDoubleImage.height());
	for (unsigned int y = 0; y < resizedFloatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < resizedFloatImage.width(); ++x)
		{
			ColorRGBAd floatColor;
			EXPECT_TRUE(resizedFloatImage.getPixel(floatColor, x, y));
			ColorRGBAd doubleColor;
			EXPECT_TRUE(resizedDoubleImage.getPixel(doubleColor, x, y));

			EXPECT_NEAR(floatColor.r, doubleColor.r, 1e-5);
		}
	}
}

TEST(RotateFallbackTest, Rotate90)
{
	Image floatImage;
	EXPECT_TRUE(floatImage.initialize(Image::Format::Float, 12, 16));
	Image doubleImage;
	EXPECT_TRUE(doubleImage.initialize(Image::Format::Double, 12, 16));

	for (unsigned int y = 0; y < floatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < floatImage.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(floatImage, x, y, true);
			EXPECT_TRUE(floatImage.setPixel(x, y, color));
			EXPECT_TRUE(doubleImage.setPixel(x, y, color));
		}
	}

	Image rotatedFloatImage = floatImage.rotate(Image::RotateAngle::CCW90);
	Image rotatedDoubleImage = doubleImage.rotate(Image::RotateAngle::CCW90);
	EXPECT_EQ(floatImage.width(), rotatedFloatImage.height());
	EXPECT_EQ(floatImage.height(), rotatedFloatImage.width());
	EXPECT_EQ(doubleImage.width(), rotatedDoubleImage.height());
	EXPECT_EQ(doubleImage.height(), rotatedDoubleImage.width());
	for (unsigned int y = 0; y < rotatedFloatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < rotatedFloatImage.width(); ++x)
		{
			ColorRGBAd floatColor;
			EXPECT_TRUE(rotatedFloatImage.getPixel(floatColor, x, y));
			ColorRGBAd doubleColor;
			EXPECT_TRUE(rotatedDoubleImage.getPixel(doubleColor, x, y));

			EXPECT_NEAR(floatColor.r, doubleColor.r, 1e-5);
		}
	}

	rotatedFloatImage = floatImage.rotate(Image::RotateAngle::CCW180);
	rotatedDoubleImage = doubleImage.rotate(Image::RotateAngle::CCW180);
	EXPECT_EQ(floatImage.width(), rotatedFloatImage.width());
	EXPECT_EQ(floatImage.height(), rotatedFloatImage.height());
	EXPECT_EQ(doubleImage.width(), rotatedDoubleImage.width());
	EXPECT_EQ(doubleImage.height(), rotatedDoubleImage.height());
	for (unsigned int y = 0; y < rotatedFloatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < rotatedFloatImage.width(); ++x)
		{
			ColorRGBAd floatColor;
			EXPECT_TRUE(rotatedFloatImage.getPixel(floatColor, x, y));
			ColorRGBAd doubleColor;
			EXPECT_TRUE(rotatedDoubleImage.getPixel(doubleColor, x, y));

			EXPECT_NEAR(floatColor.r, doubleColor.r, 1e-5);
		}
	}

	rotatedFloatImage = floatImage.rotate(Image::RotateAngle::CCW270);
	rotatedDoubleImage = doubleImage.rotate(Image::RotateAngle::CCW270);
	EXPECT_EQ(floatImage.width(), rotatedFloatImage.height());
	EXPECT_EQ(floatImage.height(), rotatedFloatImage.width());
	EXPECT_EQ(doubleImage.width(), rotatedDoubleImage.height());
	EXPECT_EQ(doubleImage.height(), rotatedDoubleImage.width());
	for (unsigned int y = 0; y < rotatedFloatImage.height(); ++y)
	{
		for (unsigned int x = 0; x < rotatedFloatImage.width(); ++x)
		{
			ColorRGBAd floatColor;
			EXPECT_TRUE(rotatedFloatImage.getPixel(floatColor, x, y));
			ColorRGBAd doubleColor;
			EXPECT_TRUE(rotatedDoubleImage.getPixel(doubleColor, x, y));

			EXPECT_NEAR(floatColor.r, doubleColor.r, 1e-5);
		}
	}
}

TEST_P(ImageTest, FlipHorizontal)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.flipHorizontal());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));
			EXPECT_TRUE(colorsEqual(getTestColor(image, image.width() - x - 1, y, divide), color,
				imageInfo));
		}
	}
}

TEST_P(ImageTest, FlipVertical)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.flipVertical());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));
			EXPECT_TRUE(colorsEqual(getTestColor(image, x, image.height() - y - 1, divide), color,
				imageInfo));
		}
	}
}

TEST_P(ImageTest, PreMultiplyAlpha)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.preMultiplyAlpha());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));
			ColorRGBAd testColor = getTestColor(image, x, y, divide);
			testColor.r *= color.a;
			testColor.g *= color.a;
			testColor.b *= color.a;
			EXPECT_TRUE(colorsEqual(testColor, color, imageInfo));
		}
	}
}

TEST_P(ImageColorTest, Grayscale)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.grayscale());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));

			ColorRGBAd testColor = getTestColor(image, x, y, divide);
			testColor.r = testColor.g = testColor.b = toGrayscale(testColor.r, testColor.g,
				testColor.b);
			testColor.a = color.a;
			EXPECT_TRUE(colorsEqual(testColor, color, imageInfo));
		}
	}
}

TEST_P(ImageColorTest, Swizzle)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.swizzle(Image::Channel::Blue, Image::Channel::Red, Image::Channel::Green,
		Image::Channel::Alpha));
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));

			ColorRGBAd testColor = getTestColor(image, x, y, divide);
			std::swap(testColor.r, testColor.b);
			std::swap(testColor.g, testColor.b);
			testColor.a = color.a;
			EXPECT_TRUE(colorsEqual(testColor, color, imageInfo));
		}
	}
}

TEST_P(ImageSRGBColorTest, LinearToSRGB)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15, ColorSpace::Linear));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.changeColorSpace(ColorSpace::sRGB));
	EXPECT_EQ(ColorSpace::sRGB, image.colorSpace());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));

			ColorRGBAd testColor = getTestColor(image, x, y, divide);
			testColor.r = linearToSRGB(testColor.r);
			testColor.g = linearToSRGB(testColor.g);
			testColor.b = linearToSRGB(testColor.b);
			testColor.a = color.a;
			EXPECT_TRUE(colorsEqual(testColor, color, imageInfo));
		}
	}
}

TEST_P(ImageSRGBColorTest, sRGBToLinear)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15, ColorSpace::sRGB));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.changeColorSpace(ColorSpace::Linear));
	EXPECT_EQ(ColorSpace::Linear, image.colorSpace());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));

			ColorRGBAd testColor = getTestColor(image, x, y, divide);
			testColor.r = sRGBToLinear(testColor.r);
			testColor.g = sRGBToLinear(testColor.g);
			testColor.b = sRGBToLinear(testColor.b);
			testColor.a = color.a;
			EXPECT_TRUE(colorsEqual(testColor, color, imageInfo));
		}
	}
}

TEST_P(ImageSRGBColorTest, Resize)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 12, 16, ColorSpace::sRGB));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	Image resizedImage = image.resize(19, 23, Image::ResizeFilter::CatmullRom);

	Image otherImage = image;
	EXPECT_TRUE(otherImage.changeColorSpace(ColorSpace::Linear));
	Image otherResizedImage = otherImage.resize(19, 23, Image::ResizeFilter::CatmullRom);
	EXPECT_TRUE(otherResizedImage.changeColorSpace(ColorSpace::sRGB));

	ASSERT_EQ(resizedImage.width(), otherResizedImage.width());
	ASSERT_EQ(resizedImage.height(), otherResizedImage.height());
	EXPECT_EQ(ColorSpace::sRGB, resizedImage.colorSpace());
	EXPECT_EQ(ColorSpace::sRGB, otherResizedImage.colorSpace());

	for (unsigned int y = 0; y < resizedImage.height(); ++y)
	{
		for (unsigned int x = 0; x < resizedImage.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(resizedImage.getPixel(color, x, y));
			ColorRGBAd otherColor;
			EXPECT_TRUE(otherResizedImage.getPixel(otherColor, x, y));
			EXPECT_TRUE(colorsEqual(otherColor, color, imageInfo));
		}
	}
}

TEST_P(ImageSRGBColorTest, PreMultiplyAlpha)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 12, 16, ColorSpace::sRGB));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.preMultiplyAlpha());
	EXPECT_EQ(ColorSpace::sRGB, image.colorSpace());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));
			ColorRGBAd testColor = getTestColor(image, x, y, divide);
			testColor.r = linearToSRGB(sRGBToLinear(testColor.r)*color.a);
			testColor.g = linearToSRGB(sRGBToLinear(testColor.g)*color.a);
			testColor.b = linearToSRGB(sRGBToLinear(testColor.b)*color.a);
			EXPECT_TRUE(colorsEqual(testColor, color, imageInfo));
		}
	}
}

TEST_P(ImageSRGBColorTest, Grayscale)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image.isValid());
	EXPECT_TRUE(image.initialize(imageInfo.format, 12, 16, ColorSpace::sRGB));

	bool divide = shouldDivide(imageInfo.format);
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			color = getTestColor(image, x, y, divide);
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	EXPECT_TRUE(image.grayscale());
	EXPECT_EQ(ColorSpace::sRGB, image.colorSpace());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			ColorRGBAd color;
			EXPECT_TRUE(image.getPixel(color, x, y));
			ColorRGBAd testColor = getTestColor(image, x, y, divide);
			testColor.r = sRGBToLinear(testColor.r);
			testColor.g = sRGBToLinear(testColor.g);
			testColor.b = sRGBToLinear(testColor.b);
			double grayscale = linearToSRGB(toGrayscale(testColor.r, testColor.g, testColor.b));
			testColor.r = testColor.g = testColor.b = grayscale;
			EXPECT_TRUE(colorsEqual(testColor, color, imageInfo));
		}
	}
}

static double getHeight(const Image& image, unsigned int x, unsigned int y)
{
	return (x - image.width()/2.0)/(image.width()/2.0)*
		(y - image.height()/2.0)/(image.height()/2.0);
}

TEST(NormalMapTest, CreateNormalMap)
{
	Image image;
	EXPECT_TRUE(image.initialize(Image::Format::RGBF, 9, 9));

	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double h = getHeight(image, x, y);
			ColorRGBAd color = {h, 0.0, 0.0, 1.0};
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	Image normalMap = image.createNormalMap(Image::NormalOptions::KeepSign, 2.5);
	EXPECT_TRUE(normalMap.isValid());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double x0 = getHeight(image, x == 0 ? x : x - 1, y);
			double x1 = getHeight(image, x == image.width() - 1 ? x : x + 1, y);
			double y0 = getHeight(image, x, y == 0 ? y : y - 1);
			double y1 = getHeight(image, x, y == image.height() - 1 ? y : y + 1);

			double width = x == 0 || x == image.width() - 1 ? 1.0 : 2.0;
			double height = y == 0 || y == image.height() - 1 ? 1.0 : 2.0;
			double dx = (x0 - x1)*2.5/width;
			double dy = (y1 - y0)*2.5/height;
			double len = std::sqrt(dx*dx + dy*dy + 1);

			ColorRGBAd color;
			EXPECT_TRUE(normalMap.getPixel(color, x, y));
			EXPECT_GE(1.0, color.r);
			EXPECT_LE(-1.0, color.r);
			EXPECT_GE(1.0, color.g);
			EXPECT_LE(-1.0, color.g);
			EXPECT_GE(1.0, color.b);
			EXPECT_LE(-1.0, color.b);
			EXPECT_NEAR(dx/len, color.r, 1e-4);
			EXPECT_NEAR(dy/len, color.g, 1e-4);
			EXPECT_NEAR(1.0/len, color.b, 1e-4);
		}
	}
}

TEST(NormalMapTest, CreateNormalMapKeepSign)
{
	Image image;
	EXPECT_TRUE(image.initialize(Image::Format::RGBF, 9, 9));

	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double h = getHeight(image, x, y);
			ColorRGBAd color = {h, 0.0, 0.0, 1.0};
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	Image normalMap = image.createNormalMap(Image::NormalOptions::Default, 2.5);
	EXPECT_TRUE(normalMap.isValid());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double x0 = getHeight(image, x == 0 ? x : x - 1, y);
			double x1 = getHeight(image, x == image.width() - 1 ? x : x + 1, y);
			double y0 = getHeight(image, x, y == 0 ? y : y - 1);
			double y1 = getHeight(image, x, y == image.height() - 1 ? y : y + 1);

			double width = x == 0 || x == image.width() - 1 ? 1.0 : 2.0;
			double height = y == 0 || y == image.height() - 1 ? 1.0 : 2.0;
			double dx = (x0 - x1)*2.5/width;
			double dy = (y1 - y0)*2.5/height;
			double len = std::sqrt(dx*dx + dy*dy + 1);

			ColorRGBAd color;
			EXPECT_TRUE(normalMap.getPixel(color, x, y));
			EXPECT_GE(1.0, color.r);
			EXPECT_LE(0.0, color.r);
			EXPECT_GE(1.0, color.g);
			EXPECT_LE(0.0, color.g);
			EXPECT_GE(1.0, color.b);
			EXPECT_LE(0.0, color.b);
			EXPECT_NEAR(dx/len*0.5 + 0.5, color.r, 1e-4);
			EXPECT_NEAR(dy/len*0.5 + 0.5, color.g, 1e-4);
			EXPECT_NEAR(1.0/len*0.5 + 0.5, color.b, 1e-4);
		}
	}
}

TEST(NormalMapTest, CreateNormalMapWrapX)
{
	Image image;
	EXPECT_TRUE(image.initialize(Image::Format::RGBF, 9, 9));

	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double h = getHeight(image, x, y);
			ColorRGBAd color = {h, 0.0, 0.0, 1.0};
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	Image normalMap = image.createNormalMap(Image::NormalOptions::WrapX, 2.5);
	EXPECT_TRUE(normalMap.isValid());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double x0 = getHeight(image, x == 0 ? image.width() - 1 : x - 1, y);
			double x1 = getHeight(image, x == image.width() - 1 ? 0 : x + 1, y);
			double y0 = getHeight(image, x, y == 0 ? y : y - 1);
			double y1 = getHeight(image, x, y == image.height() - 1 ? y : y + 1);

			double width = 2.0;
			double height = y == 0 || y == image.height() - 1 ? 1.0 : 2.0;
			double dx = (x0 - x1)*2.5/width;
			double dy = (y1 - y0)*2.5/height;
			double len = std::sqrt(dx*dx + dy*dy + 1);

			ColorRGBAd color;
			EXPECT_TRUE(normalMap.getPixel(color, x, y));
			EXPECT_GE(1.0, color.r);
			EXPECT_LE(0.0, color.r);
			EXPECT_GE(1.0, color.g);
			EXPECT_LE(0.0, color.g);
			EXPECT_GE(1.0, color.b);
			EXPECT_LE(0.0, color.b);
			EXPECT_NEAR(dx/len*0.5 + 0.5, color.r, 1e-4);
			EXPECT_NEAR(dy/len*0.5 + 0.5, color.g, 1e-4);
			EXPECT_NEAR(1.0/len*0.5 + 0.5, color.b, 1e-4);
		}
	}
}

TEST(NormalMapTest, CreateNormalMapWrapY)
{
	Image image;
	EXPECT_TRUE(image.initialize(Image::Format::RGBF, 9, 9));

	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double h = getHeight(image, x, y);
			ColorRGBAd color = {h, 0.0, 0.0, 1.0};
			EXPECT_TRUE(image.setPixel(x, y, color));
		}
	}

	Image normalMap = image.createNormalMap(Image::NormalOptions::WrapY, 2.5);
	EXPECT_TRUE(normalMap.isValid());
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			double x0 = getHeight(image, x == 0 ? x : x - 1, y);
			double x1 = getHeight(image, x == image.width() - 1 ? x : x + 1, y);
			double y0 = getHeight(image, x, y == 0 ? image.height() - 1 : y - 1);
			double y1 = getHeight(image, x, y == image.height() - 1 ? 0 : y + 1);

			double width = x == 0 || x == image.width() - 1 ? 1.0 : 2.0;
			double height = 2.0;
			double dx = (x0 - x1)*2.5/width;
			double dy = (y1 - y0)*2.5/height;
			double len = std::sqrt(dx*dx + dy*dy + 1);

			ColorRGBAd color;
			EXPECT_TRUE(normalMap.getPixel(color, x, y));
			EXPECT_GE(1.0, color.r);
			EXPECT_LE(0.0, color.r);
			EXPECT_GE(1.0, color.g);
			EXPECT_LE(0.0, color.g);
			EXPECT_GE(1.0, color.b);
			EXPECT_LE(0.0, color.b);
			EXPECT_NEAR(dx/len*0.5 + 0.5, color.r, 1e-4);
			EXPECT_NEAR(dy/len*0.5 + 0.5, color.g, 1e-4);
			EXPECT_NEAR(1.0/len*0.5 + 0.5, color.b, 1e-4);
		}
	}
}

INSTANTIATE_TEST_SUITE_P(ImageTestTypes,
	ImageTest,
	testing::Values(
		ImageTestInfo(Image::Format::Gray8, 1/255.0, 0),
		ImageTestInfo(Image::Format::RGB5, 1/31.0, 3),
		ImageTestInfo(Image::Format::RGB565, 1/31.0, 3),
		ImageTestInfo(Image::Format::RGB8, 1/255.0, 3),
		ImageTestInfo(Image::Format::RGB16, 1/65535.0, 3),
		ImageTestInfo(Image::Format::RGBF, 1e-6, 3),
		ImageTestInfo(Image::Format::RGBA8, 1/255.0, 4),
		ImageTestInfo(Image::Format::RGBA16, 1/65535.0, 4),
		ImageTestInfo(Image::Format::RGBAF, 1e-6, 4),
		ImageTestInfo(Image::Format::Int16, 1, 1),
		ImageTestInfo(Image::Format::UInt16, 1, 1),
		ImageTestInfo(Image::Format::Int32, 1, 1),
		ImageTestInfo(Image::Format::UInt32, 1, 1),
		ImageTestInfo(Image::Format::Float, 1e-6, 1),
		ImageTestInfo(Image::Format::Double, 1e-15, 1),
		ImageTestInfo(Image::Format::Complex, 1e-15, 2)));

INSTANTIATE_TEST_SUITE_P(ImageTestTypes,
	ImageColorTest,
	testing::Values(
		ImageTestInfo(Image::Format::RGB5, 1/31.0, 3),
		ImageTestInfo(Image::Format::RGB565, 1/31.0, 3),
		ImageTestInfo(Image::Format::RGB8, 1/255.0, 3),
		ImageTestInfo(Image::Format::RGB16, 1/65535.0, 3),
		ImageTestInfo(Image::Format::RGBF, 1e-6, 3),
		ImageTestInfo(Image::Format::RGBA8, 1/255.0, 4),
		ImageTestInfo(Image::Format::RGBA16, 1/65535.0, 4),
		ImageTestInfo(Image::Format::RGBAF, 1e-6, 4)));

INSTANTIATE_TEST_SUITE_P(ImageTestTypes,
	ImageSRGBColorTest,
	testing::Values(
		ImageTestInfo(Image::Format::RGB16, 2/65535.0, 3),
		ImageTestInfo(Image::Format::RGBF, 1e-6, 3),
		ImageTestInfo(Image::Format::RGBA16, 2/65535.0, 4),
		ImageTestInfo(Image::Format::RGBAF, 1e-6, 4)));

} // cuttlefish
