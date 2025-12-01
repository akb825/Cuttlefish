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

namespace
{

constexpr auto unset = static_cast<unsigned int>(-1);

unsigned int nextPO2(unsigned int size)
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

unsigned int nearestPO2(unsigned int size)
{
	unsigned int next = nextPO2(size);
	unsigned int prev = next >> 1;
	if (prev == 0)
		return next;

	return next - size < size - prev ? next : prev;
}

bool isSigned(Texture::Type type)
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
			return true;
	}

	assert(false);
	return false;
}

// Intentionally pass by value since the path will be modified.
bool createParentDir(std::string path)
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

unsigned int getDimension(unsigned int curDim, unsigned int width, unsigned int height, int size)
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

bool loadAndProcessImage(Image& image, CommandLine& args, const std::string& path,
	unsigned int& width, unsigned int& height, unsigned int mipLevel = 0)
{
	if (args.log == CommandLine::Log::Verbose)
		std::cout << "loading image '" << path << "'" << std::endl;
	if (!image.load(path.c_str(), args.imageColorSpace))
	{
		std::cerr << "error: couldn't load image '" << path << "'" << std::endl;
		return false;
	}

	if (width == unset && height == unset)
	{
		width = getDimension(image.width(), image.width(), image.height(), args.width);
		height = getDimension(image.height(), image.width(), image.height(), args.height);
	}

	Image::Format origImageFormat = image.format();
	if (origImageFormat != Image::Format::RGBAF)
	{
		if (args.log == CommandLine::Log::Verbose)
			std::cout << "converting image '" << path << "' to RGBAF" << std::endl;
		image = image.convert(Image::Format::RGBAF);
	}

	if (args.textureColorSpace != args.imageColorSpace)
	{
		if (args.log == CommandLine::Log::Verbose)
		{
			std::cout << "converting image '" << path << "' from sRGB to linear" <<
				std::endl;
		}
		image.changeColorSpace(args.textureColorSpace);
	}

	// When resizing for a specific mip level, if normalmaps are generated use the original target
	// size first. This ensures consistent resu.ts for the same normal height value.
	unsigned int normalWidth = width, normalHeight = height;
	unsigned int thisWidth = std::max(width >> mipLevel, 1U);
	unsigned int thisHeight = std::max(height >> mipLevel, 1U);
	if (!args.normalMap)
	{
		normalWidth = thisWidth;
		normalHeight = thisHeight;
	}
	if (normalWidth != image.width() || normalHeight != image.height())
	{
		if (args.log == CommandLine::Log::Verbose)
		{
			std::cout << "resizing image '" << path << "' to " << normalWidth << " x " <<
				normalHeight << std::endl;
		}
		image = image.resize(normalWidth, normalHeight, args.resizeFilter);
	}

	if (args.rotate)
	{
		if (args.log == CommandLine::Log::Verbose)
			std::cout << "rotating image '" << path << "'" << std::endl;
		image = image.rotate(args.rotateAngle);
	}

	if (args.grayscale)
	{
		if (args.log == CommandLine::Log::Verbose)
		{
			std::cout << "converting image '" << path << "' to grayscale" <<
				std::endl;
		}
		image.grayscale();
	}

	if (args.normalMap)
	{
		if (args.log == CommandLine::Log::Verbose)
		{
			std::cout << "generating normalmap for image '" << path << "'" <<
				std::endl;
		}
		Image::NormalOptions options = args.normalOptions;
		if (isSigned(args.type))
			options |= Image::NormalOptions::KeepSign;
		image = image.createNormalMap(options, args.normalHeight);

		if (normalWidth != thisWidth || normalHeight != thisHeight)
			image = image.resize(thisWidth, thisHeight, args.resizeFilter);

		// Image no longer matches the original input.
		origImageFormat = image.format();
	}

	if (args.flipX)
	{
		if (args.log == CommandLine::Log::Verbose)
		{
			std::cout << "flipping image '" << path << "' along the X axis" <<
				std::endl;
		}
		image.flipHorizontal();
	}

	if (args.flipY)
	{
		if (args.log == CommandLine::Log::Verbose)
		{
			std::cout << "flipping image '" << path << "' along the Y axis" <<
				std::endl;
		}
		image.flipVertical();
	}

	if (args.swizzle)
	{
		if (args.log == CommandLine::Log::Verbose)
			std::cout << "swizzling image '" << path << "'" << std::endl;
		image.swizzle(args.redSwzl, args.greenSwzl, args.blueSwzl, args.alphaSwzl);
	}

	if (args.preMultiply)
	{
		if (args.log == CommandLine::Log::Verbose)
		{
			std::cout << "pre-multiplying alpha for image '" << path << "'" <<
				std::endl;
		}
		image.preMultiplyAlpha();
	}

	Texture::adjustImageValueRange(image, args.type, origImageFormat);
	return true;
}

bool loadImages(
	std::vector<Image>& images, Texture::CustomMipImages& customMipImages, CommandLine& args)
{
	images.resize(args.images.size());
	unsigned int width = unset, height = unset;
	for (std::size_t i = 0; i < args.images.size(); ++i)
	{
		if (!loadAndProcessImage(images[i], args, args.images[i], width, height, 0))
			return false;
	}

	unsigned int mipLevels = std::min(args.mipLevels,
		Texture::maxMipmapLevels(args.dimension, width, height,
			static_cast<unsigned int>(images.size())));
	for (const auto& customMip : args.customMipImages)
	{
		// Didn't have the information to validate this earlier.
		if (customMip.first.mipLevel >= mipLevels)
		{
			std::cerr << "error: level " << customMip.first.mipLevel <<
				" for custom mip out of range" << std::endl;
			return false;
		}

		if (customMip.first.cubeFace != Texture::CubeFace::PosX &&
			args.dimension != Texture::Dimension::Cube)
		{
			std::cerr << "error: custom mip cube face used for non-cubemap texture" << std::endl;
			return false;
		}

		Image image;
		if (!loadAndProcessImage(
				image, args, customMip.second.image, width, height, customMip.first.mipLevel))
		{
			return false;
		}

		customMipImages.emplace(customMip.first,
			Texture::CustomMipImage(std::move(image), customMip.second.replacement));
	}

	return true;
}

bool saveTexture(
	std::vector<Image>& images, Texture::CustomMipImages customMipImages, const CommandLine& args)
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
				texture.setImage(
					std::move(images[i]), static_cast<Texture::CubeFace>(i % 6), 0, i/6);
			}
			break;
		default:
			break;
	}

	if (args.mipLevels > 1)
	{
		if (args.log == CommandLine::Log::Verbose)
			std::cout << "generating mipmaps" << std::endl;
		texture.generateMipmaps(args.mipFilter, args.mipLevels, customMipImages);
		customMipImages.clear();
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

} // namespace

int main(int argc, const char** argv)
{
	CommandLine commandLine;
	if (!commandLine.parse(argc, argv))
		return 1;

	std::vector<Image> images;
	Texture::CustomMipImages customMipImages;
	if (!loadImages(images, customMipImages, commandLine))
		return 2;

	if (!saveTexture(images, customMipImages, commandLine))
		return 3;

	return 0;
}
