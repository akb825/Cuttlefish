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

#include <cuttlefish/Image.h>
#include <cuttlefish/Texture.h>
#include <string>
#include <vector>

struct CommandLine
{
	enum class ImageType
	{
		Image,
		Array,
		Cube,
		CubeArray
	};

	enum class Log
	{
		Normal,
		Quiet,
		Verbose
	};

	enum Size
	{
		OriginalSize = -1,
		NextPO2 = -2,
		NearestPO2 = -3
	};

	bool parse(int argc, const char** argv);

	unsigned int jobs = 1;
	Log log = Log::Normal;
	ImageType imageType = ImageType::Image;
	std::vector<std::string> images;
	int width = OriginalSize;
	int height = OriginalSize;
	cuttlefish::Image::ResizeFilter resizeFilter = cuttlefish::Image::ResizeFilter::CatmullRom;
	unsigned int mipLevels = 1;
	cuttlefish::Image::ResizeFilter mipFilter = cuttlefish::Image::ResizeFilter::CatmullRom;
	bool flipX = false;
	bool flipY = false;
	bool rotate = false;
	cuttlefish::Image::RotateAngle rotateAngle = cuttlefish::Image::RotateAngle::CW90;
	bool normalMap = false;
	double normalHeight = 1.0;
	bool grayscale = false;
	bool swizzle = false;
	cuttlefish::Image::Channel redSwzl = cuttlefish::Image::Channel::Red;
	cuttlefish::Image::Channel greenSwzl = cuttlefish::Image::Channel::Green;
	cuttlefish::Image::Channel blueSwzl = cuttlefish::Image::Channel::Blue;
	cuttlefish::Image::Channel alphaSwzl = cuttlefish::Image::Channel::Alpha;
	cuttlefish::Texture::ColorMask colorMask;
	cuttlefish::ColorSpace imageColorSpace = cuttlefish::ColorSpace::Linear;
	cuttlefish::ColorSpace textureColorSpace = cuttlefish::ColorSpace::Linear;
	bool preMultiply = false;
	cuttlefish::Texture::Dimension dimension = cuttlefish::Texture::Dimension::Dim2D;
	cuttlefish::Texture::Format format = cuttlefish::Texture::Format::Unknown;
	cuttlefish::Texture::Type type = cuttlefish::Texture::Type::UNorm;
	cuttlefish::Texture::Alpha alpha = cuttlefish::Texture::Alpha::Standard;
	cuttlefish::Texture::Quality quality = cuttlefish::Texture::Quality::Normal;
	const char* output = nullptr;
	cuttlefish::Texture::FileType fileType = cuttlefish::Texture::FileType::Auto;
};
