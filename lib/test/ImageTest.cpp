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

#include <cuttlefish/Color.h>
#include <cuttlefish/Image.h>
#include <gtest/gtest.h>

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

TEST_P(ImageTest, Initialize)
{
	const ImageTestInfo& imageInfo = GetParam();
	Image image;
	EXPECT_FALSE(image);
	EXPECT_TRUE(image.initialize(imageInfo.format, 10, 15));
	EXPECT_TRUE(image);
	EXPECT_EQ(imageInfo.format, image.format());
	EXPECT_EQ(10U, image.width());
	EXPECT_EQ(15U, image.height());

	image.reset();
	EXPECT_FALSE(image);
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
	EXPECT_FALSE(image);
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
	EXPECT_FALSE(image);
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

INSTANTIATE_TEST_CASE_P(ImageTestTypes,
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

} // cuttlefish
