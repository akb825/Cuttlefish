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

#include "CommandLine.h"
#include <cuttlefish/Image.h>
#include <cuttlefish/Texture.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <vector>
#include <iostream>

#if CUTTLEFISH_WINDOWS
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif

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

// Intentionally pass by value since the path will be modified.
static bool createParentDir(std::string path)
{
#if CUTTLEFISH_WINDOWS
	const char* pathSep = "\\/";
#else
	const char* pathSep = "/";
#endif

	std::size_t prevPos = 0;
	do
	{
		std::size_t nextPos = path.find_first_of(pathSep, prevPos + 1);
		if (nextPos == std::string::npos)
			return true;

		char prevChar = path[nextPos];
		path[nextPos] = 0;
		int result = mkdir(path.c_str(), 0755);
		path[nextPos] = prevChar;
		if (result < 0 && errno != EEXIST)
			return false;

		prevPos = nextPos;
	} while (true);
}

static unsigned int getDimension(unsigned int curDim, unsigned int width, unsigned int height,
	int size)
{
	switch (size)
	{
		case CommandLine::OriginalSize:
			return curDim;
		case CommandLine::NextPO2:
			return nextPO2(curDim);
		case CommandLine::NearestPO2:
			return nearestPO2(curDim);
		case CommandLine::Width:
			return width;
		case CommandLine::WidthNextPO2:
			return nextPO2(width);
		case CommandLine::WidthNearestPO2:
			return nearestPO2(width);
		case CommandLine::Height:
			return height;
		case CommandLine::HeightNextPO2:
			return nextPO2(height);
		case CommandLine::HeightNearestPO2:
			return nearestPO2(height);
		case CommandLine::Min:
			return std::min(width, height);
		case CommandLine::MinNextPO2:
			return nextPO2(std::min(width, height));
		case CommandLine::MinNearestPO2:
			return nearestPO2(std::min(width, height));
		case CommandLine::Max:
			return std::max(width, height);
		case CommandLine::MaxNextPO2:
			return nextPO2(std::max(width, height));
		case CommandLine::MaxNearestPO2:
			return nearestPO2(std::max(width, height));
		default:
			return size;
	}
}

static bool loadImages(std::vector<Image>& images, CommandLine& args)
{
	images.resize(args.images.size());
	for (std::size_t i = 0; i < args.images.size(); ++i)
	{
		if (args.log == CommandLine::Log::Verbose)
			std::cout << "loading image '" << args.images[i] << "'" << std::endl;
		if (!images[i].load(args.images[i].c_str(), args.imageColorSpace))
		{
			std::cerr << "error: couldn't load image '" << args.images[i] << "'" << std::endl;
			return false;
		}

		Image::Format origImageFormat = images[i].format();
		if (origImageFormat != Image::Format::RGBAF)
		{
			if (args.log == CommandLine::Log::Verbose)
				std::cout << "converting image '" << args.images[i] << "' to RGBAF" << std::endl;
			images[i] = images[i].convert(Image::Format::RGBAF);
		}

		if (args.textureColorSpace != args.imageColorSpace)
		{
			if (args.log == CommandLine::Log::Verbose)
			{
				std::cout << "converting image '" << args.images[i] << "' from sRGB to linear" <<
					std::endl;
			}
			images[i].changeColorSpace(args.textureColorSpace);
		}

		unsigned int width =
			getDimension(images[0].width(), images[0].width(), images[0].height(), args.width);
		unsigned int height =
			getDimension(images[0].height(), images[0].width(), images[0].height(), args.height);

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
			{
				std::cout << "converting image '" << args.images[i] << "' to grayscale" <<
					std::endl;
			}
			images[i].grayscale();
		}

		if (args.normalMap)
		{
			if (args.log == CommandLine::Log::Verbose)
			{
				std::cout << "generating normalmap for image '" << args.images[i] << "'" <<
					std::endl;
			}
			Image::NormalOptions options = args.normalOptions;
			if (isSigned(args.type))
				options |= Image::NormalOptions::KeepSign;
			images[i] = images[i].createNormalMap(options, args.normalHeight);

			// Image no longer matches the original input.
			origImageFormat = images[i].format();
		}

		if (args.flipX)
		{
			if (args.log == CommandLine::Log::Verbose)
			{
				std::cout << "flipping image '" << args.images[i] << "' along the X axis" <<
					std::endl;
			}
			images[i].flipHorizontal();
		}

		if (args.flipY)
		{
			if (args.log == CommandLine::Log::Verbose)
			{
				std::cout << "flipping image '" << args.images[i] << "' along the Y axis" <<
					std::endl;
			}
			images[i].flipVertical();
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
			{
				std::cout << "pre-multiplying alpha for image '" << args.images[i] << "'" <<
					std::endl;
			}
			images[i].preMultiplyAlpha();
		}

		Texture::adjustImageValueRange(images[i], args.type, origImageFormat);
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

	Texture texture(args.dimension, images[0].width(), images[0].height(), depth, 1,
		args.textureColorSpace);
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
	if (!texture.convert(args.format, args.type, args.quality, args.alpha, args.colorMask,
		args.jobs))
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
		{
			if (args.createOutputDir)
			{
				// Try to create the directory and save again. We do this off of the initial save
				// so another error like an invalid format won't leave directories behind.
				if (!createParentDir(args.output))
				{
					std::cerr << "error: couldn't create parent directory for '" <<
						args.output << "'" << std::endl;
					return false;
				}

				if (texture.save(args.output, args.fileType) == Texture::SaveResult::Success)
					return true;
			}
			std::cerr << "error: couldn't write file '" << args.output << "'" << std::endl;
			return false;
		}
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
