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

#include "CommandLine.h"
#include <cuttlefish/Image.h>
#include <cuttlefish/Texture.h>
#include <cassert>
#include <vector>
#include <iostream>

using namespace cuttlefish;

static unsigned int nextPO2(unsigned int size)
{
	// https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
	--size;
	size |= size >> 1;
	size |= size >> 2;
	size |= size >> 4;
	size |= size >> 8;
	size |= size >> 16;
	return ++size;
}

static unsigned int nearestPO2(unsigned int size)
{
	unsigned int next = nextPO2(size);
	unsigned int prev = next >> 1;
	if (prev == 0)
		return next;

	return next - size < size - prev ? next : prev;
}

static bool isSigned(Texture::Type type)
{
	switch (type)
	{
		case Texture::Type::UNorm:
		case Texture::Type::UInt:
		case Texture::Type::UFloat:
			return false;
		case Texture::Type::SNorm:
		case Texture::Type::Int:
		case Texture::Type::Float:
			return false;
	}

	assert(false);
	return false;
}

bool loadImages(std::vector<Image>& images, CommandLine& args)
{
	bool linearize = args.colorSpace == Texture::Color::sRGB && !Texture::hasNativeSRGB(args.format,
		args.type);
	if (linearize)
		args.colorSpace = Texture::Color::Linear;

	images.resize(args.images.size());
	for (std::size_t i = 0; i < args.images.size(); ++i)
	{
		if (args.log == CommandLine::Log::Verbose)
			std::cout << "loading image '" << args.images[i] << "'" << std::endl;
		if (!images[i].load(args.images[i].c_str()))
		{
			std::cerr << "error: couldn't load image '" << args.images[i] << "'" << std::endl;
			return false;
		}

		if (images[i].format() != Image::Format::RGBAF)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "converting image '" << args.images[i] << "' to RGBAF" << std::endl;
			images[i] = images[i].convert(Image::Format::RGBAF);
		}

		unsigned int width;
		switch (args.width)
		{
			case CommandLine::OriginalSize:
				width = images[0].width();
				break;
			case CommandLine::NextPO2:
				width = nextPO2(images[0].width());
				break;
			case CommandLine::NearestPO2:
				width = nearestPO2(images[0].width());
				break;
			default:
				width = images[0].width();
				break;
		}

		unsigned int height;
		switch (args.height)
		{
			case CommandLine::OriginalSize:
				height = images[0].height();
				break;
			case CommandLine::NextPO2:
				height = nextPO2(images[0].height());
				break;
			case CommandLine::NearestPO2:
				height = nearestPO2(images[0].height());
				break;
			default:
				height = images[0].height();
				break;
		}

		if (width != images[i].width() || height != images[i].height())
		{
			if (args.log == CommandLine::Log::Verbose)
			{
				std::cout << "resizing image '" << args.images[i] << "' to " << width << " x " <<
					height << std::endl;
			}
			images[i] = images[i].resize(width, height, args.resizeFilter);
		}

		if (args.rotate)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "rotating image '" << args.images[i] << "'" << std::endl;
			images[i] = images[i].rotate(args.rotateAngle);
		}

		if (args.grayscale)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "converting image '" << args.images[i] << "' to grayscale" << std::endl;
			images[i].grayscale();
		}

		if (args.normalMap)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "generating normalmap for image '" << args.images[i] << "'" << std::endl;
			images[i] = images[i].createNormalMap(isSigned(args.type), args.normalHeight);
		}

		if (args.flipX)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "flipping image '" << args.images[i] << "' along the X axis" << std::endl;
			images[i].flipHorizontal();
		}

		if (args.flipY)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "flipping image '" << args.images[i] << "' along the Y axis" << std::endl;
			images[i].flipVertical();
		}

		if (linearize)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "converting image '" << args.images[i] << "' from sRGB to linear" << std::endl;
			images[i].linearize();
		}

		if (args.swizzle)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "swizzling image '" << args.images[i] << "'" << std::endl;
			images[i].swizzle(args.redSwzl, args.greenSwzl, args.blueSwzl, args.alphaSwzl);
		}

		if (args.preMultiply)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "pre-multiplying alpha for image '" << args.images[i] << "'" << std::endl;
			images[i].preMultiplyAlpha();
		}
	}

	return true;
}

static bool saveTexture(std::vector<Image>& images, const CommandLine& args)
{
	unsigned int depth = 0;
	switch (args.imageType)
	{
		case CommandLine::ImageType::Array:
			depth = static_cast<unsigned int>(images.size());
			break;
		case CommandLine::ImageType::CubeArray:
			depth = static_cast<unsigned int>(images.size()/6);
			break;
		default:
			break;
	}

	Texture texture(args.dimension, images[0].width(), images[0].height(), depth, 1);
	switch (args.imageType)
	{
		case CommandLine::ImageType::Image:
			assert(images.size() == 1);
			texture.setImage(std::move(images[0]));
			break;
		case CommandLine::ImageType::Array:
			for (unsigned int i = 0; i < images.size(); ++i)
				texture.setImage(std::move(images[i]), 0, i);
			break;
		case CommandLine::ImageType::Cube:
			assert(images.size() == 6);
			for (unsigned int i = 0; i < images.size(); ++i)
				texture.setImage(std::move(images[i]), static_cast<Texture::CubeFace>(i));
			break;
		case CommandLine::ImageType::CubeArray:
			assert(images.size() % 6 == 0);
			for (unsigned int i = 0; i < images.size(); ++i)
			{
				texture.setImage(std::move(images[i]), static_cast<Texture::CubeFace>(i % 6), 0,
					i/6);
			}
			break;
		default:
			break;
	}

	if (args.mipLevels > 1)
	{
		if (args.log == CommandLine::Log::Verbose)
			std::cout << "generating mipmaps" << std::endl;
		texture.generateMipmaps(args.mipFilter, args.mipLevels);
	}

	if (args.log == CommandLine::Log::Verbose)
		std::cout << "converting texture" << std::endl;
	if (!texture.convert(args.format, args.type, args.quality, args.colorSpace, args.alpha,
		args.colorMask, args.jobs))
	{
		std::cerr << "error: failed to convert texture" << std::endl;
		return false;
	}

	if (args.log != CommandLine::Log::Quiet)
		std::cout << "saving texture '" << args.output << "'" << std::endl;
	switch (texture.save(args.output, args.fileType))
	{
		case Texture::SaveResult::Success:
			return true;
		case Texture::SaveResult::Invalid:
			std::cerr << "error: texture parameters were invalid" << std::endl;
			return false;
		case Texture::SaveResult::UnknownFormat:
			std::cerr << "error: unknown texture file format" << std::endl;
			return false;
		case Texture::SaveResult::Unsupported:
			std::cerr << "error: texture format unsupported by target file format" << std::endl;
			return false;
		case Texture::SaveResult::WriteError:
			std::cerr << "error: couldn't write file '" << args.output << "'" << std::endl;
			return false;
	}
	assert(false);
	return false;
}

int main(int argc, const char** argv)
{
	CommandLine commandLine;
	if (!commandLine.parse(argc, argv))
		return 1;

	std::vector<Image> images;
	if (!loadImages(images, commandLine))
		return 2;

	if (!saveTexture(images, commandLine))
		return 3;

	return 0;
}
