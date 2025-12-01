/*
 * Copyright 2017-2025 Aaron Barany
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
#include <cuttlefish/Texture.h>
#include <gtest/gtest.h>
#include <limits>
#include <tuple>
#include <vector>

// Handle different versions of gtest.
#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

namespace cuttlefish
{

struct TextureConvertTestInfo
{
	TextureConvertTestInfo(Texture::Format _format, std::vector<Texture::Type> _types)
		: format(_format)
		, types(std::move(_types))
	{
	}

	Texture::Format format;
	std::vector<Texture::Type> types;
};

class TextureConvertTest : public testing::TestWithParam<TextureConvertTestInfo>
{
};

class TextureConvertSpecialTest : public testing::TestWithParam<TextureConvertTestInfo>
{
};

static ColorRGBAd getTestColor(const Image& image, unsigned int x, unsigned int y)
{
	ColorRGBAd color;
	color.r = x / static_cast<double>(image.width() - 1);
	color.g = y / static_cast<double>(image.height() - 1);
	color.b = (image.width() - x - 1) / static_cast<double>(image.width() - 1);
	color.a = (image.height() - y - 1) / static_cast<double>(image.height() - 1);
	return color;
}

TEST(TextureTest, AdjustImageValueRangeUNorm)
{
	std::vector<std::tuple<Image::Format, Image::Format, double>> formats =
	{
		{Image::Format::Gray8, Image::Format::Gray8, 1e-2},
		{Image::Format::Gray16, Image::Format::Gray16, 1e-4},
		{Image::Format::Float, Image::Format::Gray8, 1e-6},
		{Image::Format::Double, Image::Format::Gray16, 1e-6}
	};
	for (const auto& formatInfo : formats)
	{
		Image::Format format = std::get<0>(formatInfo);
		Image::Format origFormat = std::get<1>(formatInfo);
		double epsilon = std::get<2>(formatInfo);

		Image image(format, 14, 15);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, getTestColor(image, x, y), false));
		}
		Texture::adjustImageValueRange(image, Texture::Type::UNorm, origFormat);
		EXPECT_EQ(format, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.r, actualColor.g, epsilon);
				EXPECT_NEAR(expectedColor.r, actualColor.b, epsilon);
				EXPECT_EQ(1.0, actualColor.a);
			}
		}

		Texture::adjustImageValueRange(image, Texture::Type::SNorm, origFormat);
		EXPECT_EQ(Image::Format::Float, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				expectedColor.r = expectedColor.r*2.0 - 1.0;

				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.r, actualColor.g, epsilon);
				EXPECT_NEAR(expectedColor.r, actualColor.b, epsilon);
				EXPECT_EQ(1.0, actualColor.a);
			}
		}
	}

	formats =
	{
		{Image::Format::Complex, Image::Format::RGB8, 1e-6}
	};
	for (const auto& formatInfo : formats)
	{
		Image::Format format = std::get<0>(formatInfo);
		Image::Format origFormat = std::get<1>(formatInfo);
		double epsilon = std::get<2>(formatInfo);

		Image image(format, 14, 15);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, getTestColor(image, x, y), false));
		}
		Texture::adjustImageValueRange(image, Texture::Type::UNorm, origFormat);
		EXPECT_EQ(format, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.g, actualColor.g, epsilon);
				EXPECT_EQ(0.0, actualColor.b);
				EXPECT_EQ(1.0, actualColor.a);
			}
		}

		Texture::adjustImageValueRange(image, Texture::Type::SNorm, origFormat);
		EXPECT_EQ(Image::Format::RGBF, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				expectedColor.r = expectedColor.r*2.0 - 1.0;
				expectedColor.g = expectedColor.g*2.0 - 1.0;

				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.g, actualColor.g, epsilon);
				EXPECT_EQ(-1.0, actualColor.b);
				EXPECT_EQ(1.0, actualColor.a);
			}
		}
	}

	formats =
	{
		{Image::Format::RGB5, Image::Format::RGB5, 1e-1},
		{Image::Format::RGB565, Image::Format::RGB565, 1e-1},
		{Image::Format::RGB8, Image::Format::RGB8, 1e-2},
		{Image::Format::RGB16, Image::Format::RGB16, 1e-4},
		{Image::Format::RGBF, Image::Format::RGB8, 1e-6}
	};
	for (const auto& formatInfo : formats)
	{
		Image::Format format = std::get<0>(formatInfo);
		Image::Format origFormat = std::get<1>(formatInfo);
		double epsilon = std::get<2>(formatInfo);

		Image image(format, 14, 15);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, getTestColor(image, x, y)));
		}
		Texture::adjustImageValueRange(image, Texture::Type::UNorm, origFormat);
		EXPECT_EQ(format, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.g, actualColor.g, epsilon);
				EXPECT_NEAR(expectedColor.b, actualColor.b, epsilon);
				EXPECT_EQ(1.0, actualColor.a);
			}
		}

		Texture::adjustImageValueRange(image, Texture::Type::SNorm, origFormat);
		EXPECT_EQ(Image::Format::RGBF, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				expectedColor.r = expectedColor.r*2.0 - 1.0;
				expectedColor.g = expectedColor.g*2.0 - 1.0;
				expectedColor.b = expectedColor.b*2.0 - 1.0;

				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.g, actualColor.g, epsilon);
				EXPECT_NEAR(expectedColor.b, actualColor.b, epsilon);
				EXPECT_EQ(1.0, actualColor.a);
			}
		}
	}

	formats =
	{
		{Image::Format::RGBA8, Image::Format::RGBA8, 1e-2},
		{Image::Format::RGBA16, Image::Format::RGBA16, 1e-4},
		{Image::Format::RGBAF, Image::Format::RGBA8, 1e-6}
	};
	for (const auto& formatInfo : formats)
	{
		Image::Format format = std::get<0>(formatInfo);
		Image::Format origFormat = std::get<1>(formatInfo);
		double epsilon = std::get<2>(formatInfo);

		Image image(format, 14, 15);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, getTestColor(image, x, y)));
		}
		Texture::adjustImageValueRange(image, Texture::Type::UNorm, origFormat);
		EXPECT_EQ(format, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.g, actualColor.g, epsilon);
				EXPECT_NEAR(expectedColor.b, actualColor.b, epsilon);
				EXPECT_NEAR(expectedColor.a, actualColor.a, epsilon);
			}
		}

		Texture::adjustImageValueRange(image, Texture::Type::SNorm, origFormat);
		EXPECT_EQ(Image::Format::RGBAF, image.format());

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				expectedColor.r = expectedColor.r*2.0 - 1.0;
				expectedColor.g = expectedColor.g*2.0 - 1.0;
				expectedColor.b = expectedColor.b*2.0 - 1.0;
				expectedColor.a = expectedColor.a*2.0 - 1.0;

				ColorRGBAd actualColor;
				EXPECT_TRUE(image.getPixel(actualColor, x, y));
				EXPECT_NEAR(expectedColor.r, actualColor.r, epsilon);
				EXPECT_NEAR(expectedColor.g, actualColor.g, epsilon);
				EXPECT_NEAR(expectedColor.b, actualColor.b, epsilon);
				EXPECT_NEAR(expectedColor.a, actualColor.a, epsilon);
			}
		}
	}
}

TEST(TextureTest, AdjustImageValueRangeUInt)
{
	const double epsilon = 1e-6;
	std::vector<std::tuple<Image::Format, unsigned int, unsigned int, unsigned int>> formats =
	{
		{Image::Format::Gray8, std::numeric_limits<std::uint8_t>::max(), 0, 1},
		{Image::Format::Gray16, std::numeric_limits<std::uint16_t>::max(), 0, 1},
		{Image::Format::RGB5, (1 << 5) - 1, 0, 3},
		{Image::Format::RGB565, (1 << 5) - 1, (1 << 6) - 1, 3},
		{Image::Format::RGB8, std::numeric_limits<std::uint8_t>::max(), 0, 3},
		{Image::Format::RGB16, std::numeric_limits<std::uint16_t>::max(), 0, 3},
		{Image::Format::RGBF, 0, 0, 3},
		{Image::Format::RGBA8, std::numeric_limits<std::uint8_t>::max(), 0, 4},
		{Image::Format::RGBA16, std::numeric_limits<std::uint16_t>::max(), 0, 4},
		{Image::Format::RGBAF, 0, 0, 4},
		{Image::Format::Float, 0, 0, 1},
		{Image::Format::Double, 0, 0, 1},
		{Image::Format::Complex, 0, 0, 2}
	};
	for (const auto& formatInfo : formats)
	{
		Image::Format format = std::get<0>(formatInfo);
		unsigned int maxValue = std::get<1>(formatInfo);
		unsigned int greenMaxValue = std::get<2>(formatInfo);
		unsigned int channelCount = std::get<3>(formatInfo);

		Image image(format, 14, 15);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, getTestColor(image, x, y), false));
		}
		Texture::adjustImageValueRange(image, Texture::Type::UInt);

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				ColorRGBAd color;
				EXPECT_TRUE(image.getPixel(color, x, y));

				auto expectedColorValues = reinterpret_cast<const double*>(&expectedColor);
				auto colorValues = reinterpret_cast<const double*>(&color);
				for (unsigned int c = 0; c < channelCount; ++c)
				{
					double expectedValue = expectedColorValues[c];
					if (c == 1 && greenMaxValue > 0)
						expectedValue = std::round(expectedValue*greenMaxValue);
					else if (maxValue > 0)
						expectedValue = std::round(expectedValue*maxValue);

					if (maxValue > 0)
						EXPECT_DOUBLE_EQ(expectedValue, colorValues[c]);
					else
						EXPECT_NEAR(expectedValue, colorValues[c], epsilon);
				}
			}
		}
	}
}

TEST(TextureTest, AdjustImageValueRangeInt)
{
	const double epsilon = 1e-6;
	std::vector<std::tuple<Image::Format, unsigned int, unsigned int, unsigned int>> formats =
	{
		{Image::Format::Gray8, 8, 0, 1},
		{Image::Format::Gray16, 16, 0, 1},
		{Image::Format::RGB5, 5, 0, 3},
		{Image::Format::RGB565, 5, 6, 3},
		{Image::Format::RGB8, 8, 0, 3},
		{Image::Format::RGB16, 16, 0, 3},
		{Image::Format::RGBF, 0, 0, 3},
		{Image::Format::RGBA8, 8, 0, 4},
		{Image::Format::RGBA16, 16, 0, 4},
		{Image::Format::RGBAF, 0, 0, 4},
		{Image::Format::Float, 0, 0, 1},
		{Image::Format::Double, 0, 0, 1},
		{Image::Format::Complex, 0, 0, 2}
	};
	for (const auto& formatInfo : formats)
	{
		Image::Format format = std::get<0>(formatInfo);
		unsigned int bitCount = std::get<1>(formatInfo);
		unsigned int greenBitCount = std::get<2>(formatInfo);
		unsigned int channelCount = std::get<3>(formatInfo);

		Image image(format, 14, 15);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, getTestColor(image, x, y), false));
		}
		Texture::adjustImageValueRange(image, Texture::Type::Int);

		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
			{
				ColorRGBAd expectedColor = getTestColor(image, x, y);
				ColorRGBAd color;
				EXPECT_TRUE(image.getPixel(color, x, y));

				auto expectedColorValues = reinterpret_cast<const double*>(&expectedColor);
				auto colorValues = reinterpret_cast<const double*>(&color);
				for (unsigned int c = 0; c < channelCount; ++c)
				{
					double expectedValue = expectedColorValues[c];
					unsigned int channelBitCount = bitCount;
					if (c == 1 && greenBitCount > 0)
						channelBitCount = greenBitCount;

					if (channelBitCount > 0)
					{
						expectedValue = std::round(expectedValue*((1 << channelBitCount) - 1));
						expectedValue -= 1 << (channelBitCount - 1);
						EXPECT_DOUBLE_EQ(expectedValue, colorValues[c]);
					}
					else
						EXPECT_NEAR(expectedValue, colorValues[c], epsilon);
				}
			}
		}
	}
}

TEST(TextureTest, Create)
{
	Texture texture(Texture::Dimension::Dim2D, 10, 15, 0);
	EXPECT_EQ(Texture::Dimension::Dim2D, texture.dimension());
	EXPECT_EQ(10U, texture.width());
	EXPECT_EQ(15U, texture.height());
	EXPECT_EQ(1U, texture.depth());
	EXPECT_FALSE(texture.isArray());
	EXPECT_EQ(1U, texture.mipLevelCount());
	EXPECT_EQ(1U, texture.faceCount());

	EXPECT_TRUE(texture.initialize(Texture::Dimension::Cube, 15, 10, 5, Texture::allMipLevels));
	EXPECT_EQ(Texture::Dimension::Cube, texture.dimension());
	EXPECT_EQ(15U, texture.width());
	EXPECT_EQ(10U, texture.height());
	EXPECT_EQ(5U, texture.depth());
	EXPECT_TRUE(texture.isArray());
	EXPECT_EQ(4U, texture.mipLevelCount());
}

TEST(TextureTest, SetImages)
{
	Texture texture(Texture::Dimension::Dim2D, 15, 10, 5);

	EXPECT_FALSE(texture.setImage(Image(Image::Format::RGBAF, 10, 15)));

	for (unsigned int i = 0; i < 5; ++i)
	{
		EXPECT_FALSE(texture.imagesComplete());
		EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10), 0, i));
	}

	EXPECT_TRUE(texture.imagesComplete());
}

TEST(TextureTest, SetImagesCube)
{
	Texture texture(Texture::Dimension::Cube, 15, 10, 5);

	EXPECT_FALSE(texture.setImage(Image(Image::Format::RGBAF, 10, 15), Texture::CubeFace::PosX));

	for (unsigned int i = 0; i < 6; ++i)
	{
		for (unsigned int j = 0; j < 5; ++j)
		{
			EXPECT_FALSE(texture.imagesComplete());
			EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10),
				static_cast<Texture::CubeFace>(i), 0, j));
		}
	}

	EXPECT_TRUE(texture.imagesComplete());
}

TEST(TextureTest, SetImagesConvert)
{
	Texture texture(Texture::Dimension::Dim2D, 15, 10, 5, 1, ColorSpace::sRGB);
	EXPECT_EQ(ColorSpace::sRGB, texture.colorSpace());

	for (unsigned int i = 0; i < 5; ++i)
	{
		EXPECT_FALSE(texture.imagesComplete());
		EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBA8, 15, 10), 0, i));
		EXPECT_EQ(Image::Format::RGBAF, texture.getImage(0, i).format());
		EXPECT_EQ(ColorSpace::sRGB, texture.getImage(0, i).colorSpace());
	}

	EXPECT_TRUE(texture.imagesComplete());
}

TEST(TextureTest, SetImagesCubeConvert)
{
	Texture texture(Texture::Dimension::Cube, 15, 10, 5, 1, ColorSpace::sRGB);
	EXPECT_EQ(ColorSpace::sRGB, texture.colorSpace());

	for (unsigned int i = 0; i < 6; ++i)
	{
		auto face = static_cast<Texture::CubeFace>(i);
		for (unsigned int j = 0; j < 5; ++j)
		{
			EXPECT_FALSE(texture.imagesComplete());
			EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBA8, 15, 10), face, 0, j));
			EXPECT_EQ(Image::Format::RGBAF, texture.getImage(face, 0, j).format());
			EXPECT_EQ(ColorSpace::sRGB, texture.getImage(face, 0, j).colorSpace());
		}
	}

	EXPECT_TRUE(texture.imagesComplete());
}

TEST(TextureTest, CustomMipImageStorage)
{
	Image testImage(Image::Format::RGBAF, 10, 15);

	Texture::CustomMipImage mipImage(Image(testImage), Texture::MipReplacement::Once);
	EXPECT_TRUE(mipImage.imageStorage.isValid());
	EXPECT_EQ(&mipImage.imageStorage, mipImage.image);

	Texture::CustomMipImage mipImageCopy(mipImage);
	EXPECT_TRUE(mipImageCopy.imageStorage.isValid());
	EXPECT_EQ(&mipImageCopy.imageStorage, mipImageCopy.image);

	Texture::CustomMipImage mipImageMoved(std::move(mipImageCopy));
	EXPECT_FALSE(mipImageCopy.imageStorage.isValid());
	EXPECT_TRUE(mipImageMoved.imageStorage.isValid());
	EXPECT_EQ(&mipImageMoved.imageStorage, mipImageMoved.image);

	mipImageCopy = mipImage;
	EXPECT_TRUE(mipImageCopy.imageStorage.isValid());
	EXPECT_EQ(&mipImageCopy.imageStorage, mipImageCopy.image);

	mipImageMoved.image = nullptr;
	mipImageMoved.imageStorage = Image();
	mipImageMoved = std::move(mipImageCopy);
	EXPECT_FALSE(mipImageCopy.imageStorage.isValid());
	EXPECT_TRUE(mipImageMoved.imageStorage.isValid());
	EXPECT_EQ(&mipImageMoved.imageStorage, mipImageMoved.image);
}

TEST(TextureTest, GenerateMipmaps)
{
	Texture texture(Texture::Dimension::Cube, 15, 10, 5);

	EXPECT_FALSE(texture.setImage(Image(Image::Format::RGBAF, 10, 15)));

	for (unsigned int i = 0; i < 6; ++i)
	{
		for (unsigned int j = 0; j < 5; ++j)
		{
			EXPECT_FALSE(texture.imagesComplete());
			EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10),
				static_cast<Texture::CubeFace>(i), 0, j));
		}
	}

	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_TRUE(texture.generateMipmaps());
	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_EQ(4U, texture.mipLevelCount());

	EXPECT_EQ(15U, texture.getImage(Texture::CubeFace::PosX, 0, 1).width());
	EXPECT_EQ(10U, texture.getImage(Texture::CubeFace::PosX, 0, 1).height());
	EXPECT_EQ(7U, texture.getImage(Texture::CubeFace::PosX, 1, 1).width());
	EXPECT_EQ(5U, texture.getImage(Texture::CubeFace::PosX, 1, 1).height());
	EXPECT_EQ(3U, texture.getImage(Texture::CubeFace::PosX, 2, 1).width());
	EXPECT_EQ(2U, texture.getImage(Texture::CubeFace::PosX, 2, 1).height());
	EXPECT_EQ(1U, texture.getImage(Texture::CubeFace::PosX, 3, 1).width());
	EXPECT_EQ(1U, texture.getImage(Texture::CubeFace::PosX, 3, 1).height());
}

TEST(TextureTest, GenerateMipmapsCustomMips)
{
	unsigned int size = 32;
	Texture texture(Texture::Dimension::Dim2D, size, size);

	Image redImage(Image::Format::RGBAF, size, size);
	Image greenImage(Image::Format::RGBAF, size, size);
	Image blueImage(Image::Format::RGBAF, size, size);
	for (unsigned int y = 0; y < size; ++y)
	{
		for (unsigned int x = 0; x < size; ++x)
		{
			EXPECT_TRUE(redImage.setPixel(x, y, ColorRGBAd(1.0, 0.0, 0.0, 1.0)));
			EXPECT_TRUE(greenImage.setPixel(x, y, ColorRGBAd(0.0, 1.0, 0.0, 1.0)));
			EXPECT_TRUE(blueImage.setPixel(x, y, ColorRGBAd(0.0, 0.0, 1.0, 1.0)));
		}
	}
	EXPECT_TRUE(texture.setImage(redImage));

	Texture::CustomMipImages mipImages =
	{
		{Texture::ImageIndex(1),
			Texture::CustomMipImage(greenImage, Texture::MipReplacement::Continue)},
		{Texture::ImageIndex(2),
			Texture::CustomMipImage(std::move(blueImage), Texture::MipReplacement::Once)},
		{Texture::ImageIndex(3),
			Texture::CustomMipImage(redImage, Texture::MipReplacement::Once)},
	};

	EXPECT_FALSE(blueImage.isValid());

	EXPECT_TRUE(texture.generateMipmaps(
		Image::ResizeFilter::Box, Texture::allMipLevels, mipImages));
	EXPECT_TRUE(texture.imagesComplete());
	ASSERT_EQ(6U, texture.mipLevelCount());

	ColorRGBAd color;
	EXPECT_EQ(size/2, texture.getImage(1).width());
	EXPECT_EQ(size/2, texture.getImage(1).height());
	ASSERT_TRUE(texture.getImage(1).getPixel(color, 0, 0));
	EXPECT_EQ(0, color.r);
	EXPECT_EQ(1, color.g);
	EXPECT_EQ(0, color.b);

	EXPECT_EQ(size/4, texture.getImage(2).width());
	EXPECT_EQ(size/4, texture.getImage(2).height());
	ASSERT_TRUE(texture.getImage(2).getPixel(color, 0, 0));
	EXPECT_EQ(0, color.r);
	EXPECT_EQ(0, color.g);
	EXPECT_EQ(1, color.b);

	EXPECT_EQ(size/8, texture.getImage(3).width());
	EXPECT_EQ(size/8, texture.getImage(3).height());
	ASSERT_TRUE(texture.getImage(3).getPixel(color, 0, 0));
	EXPECT_EQ(1, color.r);
	EXPECT_EQ(0, color.g);
	EXPECT_EQ(0, color.b);

	EXPECT_EQ(size/16, texture.getImage(4).width());
	EXPECT_EQ(size/16, texture.getImage(4).height());
	ASSERT_TRUE(texture.getImage(4).getPixel(color, 0, 0));
	EXPECT_EQ(0, color.r);
	EXPECT_EQ(1, color.g);
	EXPECT_EQ(0, color.b);

	EXPECT_EQ(size/32, texture.getImage(5).width());
	EXPECT_EQ(size/32, texture.getImage(5).height());
	ASSERT_TRUE(texture.getImage(5).getPixel(color, 0, 0));
	EXPECT_EQ(0, color.r);
	EXPECT_EQ(1, color.g);
	EXPECT_EQ(0, color.b);

	auto foundMipImage = mipImages.find(Texture::ImageIndex(1));
	ASSERT_NE(mipImages.end(), foundMipImage);
	foundMipImage->second.image = nullptr;
	EXPECT_FALSE(texture.generateMipmaps(
		Image::ResizeFilter::Box, Texture::allMipLevels, mipImages));
}

TEST(TextureTest, Generate3DMipmaps)
{
	Texture texture(Texture::Dimension::Dim3D, 15, 10, 5);

	EXPECT_FALSE(texture.setImage(Image(Image::Format::RGBAF, 10, 15)));

	for (unsigned int j = 0; j < 5; ++j)
	{
		EXPECT_FALSE(texture.imagesComplete());
		EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10), 0, j));
	}

	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_TRUE(texture.generateMipmaps());
	EXPECT_TRUE(texture.imagesComplete());
	EXPECT_EQ(4U, texture.mipLevelCount());

	EXPECT_EQ(15U, texture.getImage(0, 0).width());
	EXPECT_EQ(10U, texture.getImage(0, 0).height());
	EXPECT_EQ(7U, texture.getImage(1, 0).width());
	EXPECT_EQ(5U, texture.getImage(1, 0).height());
	EXPECT_EQ(3U, texture.getImage(2, 0).width());
	EXPECT_EQ(2U, texture.getImage(2, 0).height());
	EXPECT_EQ(1U, texture.getImage(3, 0).width());
	EXPECT_EQ(1U, texture.getImage(3, 0).height());

	EXPECT_FALSE(texture.getImage(1, 2).isValid());
	EXPECT_TRUE(texture.getImage(1, 1).isValid());
	EXPECT_FALSE(texture.getImage(2, 1).isValid());
	EXPECT_TRUE(texture.getImage(2, 0).isValid());
	EXPECT_FALSE(texture.getImage(3, 1).isValid());
	EXPECT_TRUE(texture.getImage(3, 0).isValid());
}

TEST(TextureTest, Generate3DMipmapsCustomMips)
{
	unsigned int size = 32;
	Texture texture(Texture::Dimension::Dim3D, size, size, size);

	Image redImage(Image::Format::RGBAF, size, size);
	Image greenImage(Image::Format::RGBAF, size, size);
	Image blueImage(Image::Format::RGBAF, size, size);
	for (unsigned int y = 0; y < size; ++y)
	{
		for (unsigned int x = 0; x < size; ++x)
		{
			EXPECT_TRUE(redImage.setPixel(x, y, ColorRGBAd(1.0, 0.0, 0.0, 1.0)));
			EXPECT_TRUE(greenImage.setPixel(x, y, ColorRGBAd(0.0, 1.0, 0.0, 1.0)));
			EXPECT_TRUE(blueImage.setPixel(x, y, ColorRGBAd(0.0, 0.0, 1.0, 1.0)));
		}
	}

	for (unsigned int d = 0; d < size; ++d)
		EXPECT_TRUE(texture.setImage(redImage, 0, d));

	Texture::CustomMipImages mipImages =
	{
		{Texture::ImageIndex(1),
			Texture::CustomMipImage(greenImage, Texture::MipReplacement::Continue)},
		{Texture::ImageIndex(2),
			Texture::CustomMipImage(blueImage, Texture::MipReplacement::Once)},
		{Texture::ImageIndex(3),
			Texture::CustomMipImage(redImage, Texture::MipReplacement::Once)},
	};

	EXPECT_FALSE(texture.generateMipmaps(
		Image::ResizeFilter::Box, Texture::allMipLevels, mipImages));

	for (unsigned int d = 1; d < size/2; ++d)
	{
		mipImages.emplace(Texture::ImageIndex(1, d),
			Texture::CustomMipImage(greenImage, Texture::MipReplacement::Once));
	}

	for (unsigned int d = 1; d < size/4; ++d)
	{
		mipImages.emplace(Texture::ImageIndex(2, d),
			Texture::CustomMipImage(blueImage, Texture::MipReplacement::Once));
	}

	for (unsigned int d = 1; d < size/8; ++d)
	{
		mipImages.emplace(Texture::ImageIndex(3, d),
			Texture::CustomMipImage(redImage, Texture::MipReplacement::Once));
	}

	EXPECT_FALSE(texture.generateMipmaps(
		Image::ResizeFilter::Box, Texture::allMipLevels, mipImages));

	for (unsigned int d = 1; d < size/2; ++d)
	{
		auto foundMipImage = mipImages.find(Texture::ImageIndex(1, d));
		ASSERT_NE(mipImages.end(), foundMipImage);
		foundMipImage->second.replacement = Texture::MipReplacement::Continue;
	}

	EXPECT_TRUE(texture.generateMipmaps(
		Image::ResizeFilter::Box, Texture::allMipLevels, mipImages));
	EXPECT_TRUE(texture.imagesComplete());
	ASSERT_EQ(6U, texture.mipLevelCount());

	ColorRGBAd color;
	for (unsigned int d = 0; d < size/2; ++d)
	{
		const Image& image = texture.getImage(1, d);
		EXPECT_EQ(size/2, image.width());
		EXPECT_EQ(size/2, image.height());
		ASSERT_TRUE(image.getPixel(color, 0, 0));
		EXPECT_EQ(0, color.r);
		EXPECT_EQ(1, color.g);
		EXPECT_EQ(0, color.b);
	}

	for (unsigned int d = 0; d < size/4; ++d)
	{
		const Image& image = texture.getImage(2, d);
		EXPECT_EQ(size/4, image.width());
		EXPECT_EQ(size/4, image.height());
		ASSERT_TRUE(image.getPixel(color, 0, 0));
		EXPECT_EQ(0, color.r);
		EXPECT_EQ(0, color.g);
		EXPECT_EQ(1, color.b);
	}

	for (unsigned int d = 0; d < size/8; ++d)
	{
		const Image& image = texture.getImage(3, d);
		EXPECT_EQ(size/8, image.width());
		EXPECT_EQ(size/8, image.height());
		ASSERT_TRUE(image.getPixel(color, 0, 0));
		EXPECT_EQ(1, color.r);
		EXPECT_EQ(0, color.g);
		EXPECT_EQ(0, color.b);
	}

	for (unsigned int d = 0; d < size/16; ++d)
	{
		const Image& image = texture.getImage(4, d);
		EXPECT_EQ(size/16, image.width());
		EXPECT_EQ(size/16, image.height());
		ASSERT_TRUE(image.getPixel(color, 0, 0));
		EXPECT_EQ(0, color.r);
		EXPECT_EQ(1, color.g);
		EXPECT_EQ(0, color.b);
	}

	for (unsigned int d = 0; d < size/32; ++d)
	{
		const Image& image = texture.getImage(5, d);
		EXPECT_EQ(size/32, image.width());
		EXPECT_EQ(size/32, image.height());
		ASSERT_TRUE(image.getPixel(color, 0, 0));
		EXPECT_EQ(0, color.r);
		EXPECT_EQ(1, color.g);
		EXPECT_EQ(0, color.b);
	}
}

TEST(TextureTest, ConvertSRGB)
{
	{
		Texture texture(Texture::Dimension::Dim2D, 15, 10, 0, 1, ColorSpace::sRGB);
		EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10, ColorSpace::sRGB)));
		EXPECT_TRUE(texture.imagesComplete());
		EXPECT_TRUE(texture.convert(Texture::Format::R8G8B8A8, Texture::Type::UNorm));
	}

	{
		Texture texture(Texture::Dimension::Dim2D, 15, 10, 0, 1, ColorSpace::sRGB);
		EXPECT_TRUE(texture.setImage(Image(Image::Format::RGBAF, 15, 10, ColorSpace::sRGB)));
		EXPECT_TRUE(texture.imagesComplete());
		EXPECT_FALSE(texture.convert(Texture::Format::R5G6B5, Texture::Type::UNorm));
	}
}

TEST_P(TextureConvertTest, Convert)
{
	const TextureConvertTestInfo& info = GetParam();
	for (Texture::Type type : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, type));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());
	}
}

TEST_P(TextureConvertSpecialTest, Convert)
{
	const TextureConvertTestInfo& info = GetParam();
	for (Texture::Type type : info.types)
	{
		Texture texture(Texture::Dimension::Dim2D, 16, 16);
		Image image(Image::Format::RGBAF, 16, 16);
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for (unsigned int x = 0; x < image.width(); ++x)
				EXPECT_TRUE(image.setPixel(x, y, ColorRGBAd{0.0, 0.0, 0.0, 1.0}));
		}
		EXPECT_TRUE(texture.setImage(image));

		EXPECT_TRUE(texture.convert(info.format, type));
		unsigned int blockX = (texture.width() + Texture::blockWidth(info.format) - 1)/
			Texture::blockWidth(info.format);
		unsigned int blockY = (texture.height() + Texture::blockHeight(info.format) - 1)/
			Texture::blockHeight(info.format);
		EXPECT_EQ(blockX*blockY*Texture::blockSize(info.format), texture.dataSize());
	}
}

INSTANTIATE_TEST_SUITE_P(TextureConvertTestTypes,
	TextureConvertTest,
	testing::Values(
		TextureConvertTestInfo(Texture::Format::R4G4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R4G4B4A4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::B4G4R4A4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A4R4G4B4, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R5G6B5, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::B5G6R5, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R5G5B5A1, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::B5G5R5A1, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A1R5G5B5, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R8, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::R8G8, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::R8G8B8, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::B8G8R8, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::R8G8B8A8, {Texture::Type::UNorm,
			Texture::Type::SNorm, Texture::Type::UInt, Texture::Type::Int}),
		TextureConvertTestInfo(Texture::Format::B8G8R8A8, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A8B8G8R8, {Texture::Type::UNorm}),
		TextureConvertTestInfo(Texture::Format::A2R10G10B10, {Texture::Type::UNorm,
			Texture::Type::UInt}),
		TextureConvertTestInfo(Texture::Format::A2B10G10R10, {Texture::Type::UNorm,
			Texture::Type::UInt}),
		TextureConvertTestInfo(Texture::Format::R16, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R16G16, {Texture::Type::UNorm, Texture::Type::SNorm,
			Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R16G16B16, {Texture::Type::UNorm,
			Texture::Type::SNorm, Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R16G16B16A16, {Texture::Type::UNorm,
			Texture::Type::SNorm, Texture::Type::UInt, Texture::Type::Int, Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32, {Texture::Type::UInt, Texture::Type::Int,
			Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32G32, {Texture::Type::UInt, Texture::Type::Int,
			Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32G32B32, {Texture::Type::UInt, Texture::Type::Int,
			Texture::Type::Float}),
		TextureConvertTestInfo(Texture::Format::R32G32B32A32, {Texture::Type::UInt,
			Texture::Type::Int, Texture::Type::Float})));

#if CUTTLEFISH_HAS_S3TC
#define S3TC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::BC1_RGB, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC1_RGBA, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC2, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC3, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::BC4, {Texture::Type::UNorm, Texture::Type::SNorm}), \
	TextureConvertTestInfo(Texture::Format::BC5, {Texture::Type::UNorm, Texture::Type::SNorm}), \
	TextureConvertTestInfo(Texture::Format::BC6H, {Texture::Type::UFloat, Texture::Type::Float}), \
	TextureConvertTestInfo(Texture::Format::BC7, {Texture::Type::UNorm})
#else
#define S3TC_CONVERSION_TESTS
#endif

#if CUTTLEFISH_HAS_ETC
#define ETC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::ETC1, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::ETC2_R8G8B8, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::ETC2_R8G8B8A1, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::ETC2_R8G8B8A8, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::EAC_R11, {Texture::Type::UNorm, Texture::Type::SNorm}), \
	TextureConvertTestInfo(Texture::Format::EAC_R11G11, {Texture::Type::UNorm, Texture::Type::SNorm})
#else
#define ETC_CONVERSION_TESTS
#endif

#if CUTTLEFISH_HAS_ASTC
#define ASTC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::ASTC_4x4, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_5x4, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_5x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_6x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_8x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_8x6, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_8x8, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x5, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x6, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x8, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_10x10, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_12x10, {Texture::Type::UNorm, Texture::Type::UFloat}), \
	TextureConvertTestInfo(Texture::Format::ASTC_12x12, {Texture::Type::UNorm, Texture::Type::UFloat})
#else
#define ASTC_CONVERSION_TESTS
#endif

#if CUTTLEFISH_HAS_PVRTC
#define PVRTC_CONVERSION_TESTS \
	, TextureConvertTestInfo(Texture::Format::PVRTC1_RGB_2BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC1_RGBA_2BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC1_RGB_4BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC1_RGBA_4BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC2_RGBA_2BPP, {Texture::Type::UNorm}), \
	TextureConvertTestInfo(Texture::Format::PVRTC2_RGBA_4BPP, {Texture::Type::UNorm})
#else
#define PVRTC_CONVERSION_TESTS
#endif

INSTANTIATE_TEST_SUITE_P(TextureConvertTestTypes,
	TextureConvertSpecialTest,
	testing::Values(
		TextureConvertTestInfo(Texture::Format::B10G11R11_UFloat, {Texture::Type::UFloat}),
		TextureConvertTestInfo(Texture::Format::E5B9G9R9_UFloat, {Texture::Type::UFloat})
		S3TC_CONVERSION_TESTS
		ETC_CONVERSION_TESTS
		ASTC_CONVERSION_TESTS
		PVRTC_CONVERSION_TESTS
		));

} // namespace cuttlefish
