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
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>

#if CUTTLEFISH_WINDOWS
#include <string.h>
#define strcasecmp stricmp
#else
#include <strings.h>
#endif

using namespace cuttlefish;

namespace
{

#define FORMAT(f) {Texture::Format:: f , #f }

const std::pair<Texture::Format, const char*> formatMap[] =
{
	FORMAT(R4G4),
	FORMAT(R4G4B4A4),
	FORMAT(B4G4R4A4),
	FORMAT(A4R4G4B4),
	FORMAT(R5G6B5),
	FORMAT(B5G6R5),
	FORMAT(R5G5B5A1),
	FORMAT(B5G5R5A1),
	FORMAT(A1R5G5B5),
	FORMAT(R8),
	FORMAT(R8G8),
	FORMAT(R8G8B8),
	FORMAT(B8G8R8),
	FORMAT(R8G8B8A8),
	FORMAT(B8G8R8A8),
	FORMAT(A8B8G8R8),
	FORMAT(A2R10G10B10),
	FORMAT(A2B10G10R10),
	FORMAT(R16),
	FORMAT(R16G16),
	FORMAT(R16G16B16),
	FORMAT(R16G16B16A16),
	FORMAT(R32),
	FORMAT(R32G32),
	FORMAT(R32G32B32),
	FORMAT(R32G32B32A32),
	FORMAT(B10G11R11_UFloat),
	FORMAT(E5B9G9R9_UFloat),
#if CUTTLEFISH_HAS_S3TC
	FORMAT(BC1_RGB),
	FORMAT(BC1_RGBA),
	FORMAT(BC2),
	FORMAT(BC3),
	FORMAT(BC4),
	FORMAT(BC5),
	FORMAT(BC6H),
	FORMAT(BC7),
#endif
#if CUTTLEFISH_HAS_ETC
	FORMAT(ETC1),
	FORMAT(ETC2_R8G8B8),
	FORMAT(ETC2_R8G8B8A1),
	FORMAT(ETC2_R8G8B8A8),
	FORMAT(EAC_R11),
	FORMAT(EAC_R11G11),
#endif
#if CUTTLEFISH_HAS_ASTC
	FORMAT(ASTC_4x4),
	FORMAT(ASTC_5x4),
	FORMAT(ASTC_5x5),
	FORMAT(ASTC_6x5),
	FORMAT(ASTC_6x6),
	FORMAT(ASTC_8x5),
	FORMAT(ASTC_8x6),
	FORMAT(ASTC_8x8),
	FORMAT(ASTC_10x5),
	FORMAT(ASTC_10x6),
	FORMAT(ASTC_10x8),
	FORMAT(ASTC_10x10),
	FORMAT(ASTC_12x10),
	FORMAT(ASTC_12x12),
#endif
#if CUTTLEFISH_HAS_PVRTC
	FORMAT(PVRTC1_RGB_2BPP),
	FORMAT(PVRTC1_RGBA_2BPP),
	FORMAT(PVRTC1_RGB_4BPP),
	FORMAT(PVRTC1_RGBA_4BPP),
	FORMAT(PVRTC2_RGBA_2BPP),
	FORMAT(PVRTC2_RGBA_4BPP),
#endif
};

const char* programName(const char* programPath)
{
	std::size_t length = std::strlen(programPath);
	for (std::size_t i = length; i-- > 0;)
	{
#if CUTTLEFISH_WINDOWS
		if (programPath[i] == '/' || programPath[i] == '\\')
#else
		if (programPath[i] == '/')
#endif
		{
			return programPath + i + 1;
		}
	}

	return programPath;
}

void printHelp(const char* name)
{
	std::cout << "Usage: " << name << " [options]" << std::endl;

	std::cout << std::endl << "General options:" << std::endl;
	std::cout << "  -h, --help     display this help message" << std::endl;
	std::cout << "  -j, --jobs [n] the number of jobs to convert with; if n is omitted, use all" << std::endl
	          << "                 available cores" << std::endl;
	std::cout << "  -q, --quiet    suppress all non-error output" << std::endl;
	std::cout << "  -v, --verbose  verbose output" << std::endl;

	std::cout << std::endl << "Input files: at least one required, cannot mix different types" << std::endl;
	std::cout << "  -i, --input file               the input image for a standard texture" << std::endl;
	std::cout << "  -a, --array [n] file           the input image for an array or 3D texture:" << std::endl
	          << "                                   n: the index of the image" << std::endl
	          << "                                   file: the path to the image" << std::endl;
	std::cout << "  -c, --cube face file           the input image for a cube map: " << std::endl
	          << "                                   face: the face, which is one of:" << std::endl
	          << "                                     +x, -x, +y, -y, +z, -z" << std::endl
	          << "                                   file: the path to the image" << std::endl;
	std::cout << "  -C, --cube-array n face file   the input image for a cube map: " << std::endl
	          << "                                   n: the index of the image" << std::endl
	          << "                                   face: the face, which is one of:" << std::endl
	          << "                                     +x, -x, +y, -y, +z, -z" << std::endl
	          << "                                   file: the path to the image" << std::endl;
	std::cout << "  -I, --input-list type file     specify a file with a list of image paths:" << std::endl
	          << "                                   type: type of texture, which is one of the " << std::endl
	          << "                                     following: image, array, cube, cube-array;" << std::endl
	          << "                                     cube faces are in the order:" << std::endl
	          << "                                     +x, -x, +y, -y, +z, -z" << std::endl
	          << "                                   file: path to a file containing a list of " << std::endl
	          << "                                     image paths, one per line" << std::endl;

	std::cout << std::endl << "Manipulation options:" << std::endl;
	std::cout << "  -r, --resize w h [filter]     resizes the image:" << std::endl
	          << "                                   w: the width in pixels, or one of:" << std::endl
	          << "                                     nextpo2: the next power of 2" << std::endl
	          << "                                     nearestpo2: the nearest power of 2" << std::endl
	          << "                                     width: the width" << std::endl
	          << "                                     width-nextpo2: the next power of 2 for the" << std::endl
	          << "                                       width" << std::endl
	          << "                                     width-nearestpo2: the nearest power of 2" << std::endl
	          << "                                       for the width" << std::endl
	          << "                                     height: the height" << std::endl
	          << "                                     height-nextpo2: the next power of 2 for the" << std::endl
	          << "                                       height" << std::endl
	          << "                                     height-nearestpo2: the nearest power of 2" << std::endl
	          << "                                       for the height" << std::endl
	          << "                                     min: the minimum between width and height" << std::endl
	          << "                                     min-nextpo2: the next power of 2 for the" << std::endl
	          << "                                       minimum between width and height" << std::endl
	          << "                                     min-nearestpo2: the nearest power of 2 for" << std::endl
	          << "                                       the minimum between width and height" << std::endl
	          << "                                     max: the maximum between width and height" << std::endl
	          << "                                     max-nextpo2: the next power of 2 for the" << std::endl
	          << "                                       maximum between width and height" << std::endl
	          << "                                     max-nearestpo2: the nearest power of 2 for" << std::endl
	          << "                                       the maximum between width and height" << std::endl
	          << "                                   h: the height in pixels or po2 value" << std::endl
	          << "                                   filter: the resizing filter, is one of:" << std::endl
	          << "                                     box, linear, cubic, b-spline," << std::endl
	          << "                                     catmull-rom (default)" << std::endl;
	std::cout << "  -m, --mipmap [levels] [filter] generate mipmaps:" << std::endl
	          << "                                   levels: the number of mipmap levels, or all" << std::endl
	          << "                                     all levels if not specified" << std::endl
	          << "                                   filter: the filter to use for resizing:" << std::endl
	          << "                                     box, linear, cubic, b-spline," << std::endl
	          << "                                     catmull-rom (default)" << std::endl;
	std::cout << "  -M, --custom-mip level [depth] [face] [replace] file" << std::endl
	          << "                                 override a specific mip image:" << std::endl
	          << "                                   level: the mip level to replace" << std::endl
	          << "                                   depth: the depth for 3D textures or texture" << std::endl
	          << "                                     arrays; defaults to 0 to accommodate 1D and" << std::endl
	          << "                                     2D textures" << std::endl
	          << "                                   face: cube face as one of +x, -x, +y, -y, +z," << std::endl
	          << "                                     -z; defaults to +x to accommodate non-" << std::endl
	          << "                                     cubemaps" << std::endl
	          << "                                   replace: how to use the image for following" << std::endl
	          << "                                     mip levels at the same depth and face as"  << std::endl
	          << "                                     one of:" << std::endl
			  << "                                       once: switch back to the previous image" << std::endl
	          << "                                       continue: use for following levels" << std::endl
	          << "                                     defaults to continue" << std::endl
	          << "                                   file: the input image for the mip" << std::endl;
	std::cout << "      --custom-mip-list file     custom mip overrides provided in a file; each" << std::endl
	          << "                                 line of the file follows the same layout as the" << std::endl
	          << "                                 arguments to the --custom-mip argument" << std::endl
	          << "                                 specified above" << std::endl;
	std::cout << "      --flipx                    flip the images in the X direction" << std::endl;
	std::cout << "      --flipy                    flip the images in the Y direction" << std::endl;
	std::cout << "      --rotate degrees           rotate by an angle" << std::endl
	          << "                                   degrees: must be a multiple of 90" << std::endl;
	std::cout << "  -n, --normalmap [wrap] [h]     generate a normalmap from a bitmap" << std::endl
	          << "                                   wrap: set to wrap for tiled textures:" << std::endl
	          << "                                     wrap: wrap along both X and Y axes" << std::endl
	          << "                                     wrapx: wrap along the X axis" << std::endl
	          << "                                     wrapy: wrap along the Y axis" << std::endl
	          << "                                   h: the height scale (default is 1.0)" << std::endl;
	std::cout << "  -g, --grayscale                convert the image to grayscale" << std::endl;
	std::cout << "  -s, --swizzle rgba             swizzle the channels of the image, a " << std::endl
	          << "                                 combination of four of the following:" << std::endl
	          << "                                   r: the red channel" << std::endl
	          << "                                   g: the green channel" << std::endl
	          << "                                   b: the blue channel" << std::endl
	          << "                                   a: the alpha channel" << std::endl
	          << "                                   x: unused channel (0 for color, 1 for alpha)" << std::endl;
	std::cout << "      --srgb                     convert from sRGB to linear space; where" << std::endl
	          << "                                 possible this will use the texture format to do" << std::endl
	          << "                                 the conversion on texture read to avoid" << std::endl
	          << "                                 precision loss" << std::endl;
	std::cout << "      --pre-multiply             pre-multiply the alpha" << std::endl;

	std::cout << std::endl << "Output options: options marked with (*) are required" << std::endl;
	std::cout << "  -d, --dimension d     the texture dimesion; d may be: 1, 2 (default), 3" << std::endl;
	std::cout << "  -f, --format f (*)    the format to store the texture; f may be:" << std::endl;
	for (const auto& format : formatMap)
		std::cout << "                          " << format.second << std::endl;
	std::cout << "  -t, --type t          the type stored in each channel; t may be:" << std::endl
	          << "                          unorm: unsigned normalized value (default)" << std::endl
	          << "                          snorm: signed normalized value" << std::endl
	          << "                          uint: unsigned integer value" << std::endl
	          << "                          int: signed integer value" << std::endl
	          << "                          ufloat: unsigned floating-point value" << std::endl
	          << "                          float: signed floating-point value" << std::endl;
	std::cout << "      --alpha a         the type of alpha; a may be:" << std::endl
	          << "                          none: alpha should be ignored (implicit with 'x' in " << std::endl
	          << "                            swizzle for alpha channel and formats without alpha)" << std::endl
	          << "                          standard: standard alpha (default)" << std::endl
	          << "                          pre-multiplied: alpha is pre-multiplied with the color" << std::endl
	          << "                            (implicit with --pre-multiply option)" << std::endl
	          << "                          encoded: alpha is an encoded value rather than " << std::endl
	          << "                            opacity; this will avoid weighting the colors during" << std::endl
	          << "                            compression for some formats" << std::endl;
	std::cout << "  -Q, --quality q       the quality of compression; may be: lowest, low," << std::endl
	          << "                        normal (default), high, highest; lower qualities are" << std::endl
	          << "                        faster to convert" << std::endl;
	std::cout << "  -o, --output file (*) the output file for the texture" << std::endl;
	std::cout << "      --file-format f   the output file format; may be: dds, ktx, pvr; default" << std::endl
	          << "                        is based on the extension" << std::endl;
	std::cout << "      --create-dir      create the parent directory for the output file if it" << std::endl
			  << "                        doesn't exist" << std::endl;
}

bool matches(const char* command, const char* shortCmd, const char* longCmd)
{
	return std::strcmp(command, shortCmd) == 0 || std::strcmp(command, longCmd) == 0;
}

bool readIndex(std::size_t& index, int& i, int argc, const char** argv)
{
	if (i >= argc - 1)
		return false;

	char* endPtr;
	const char* str = argv[i + 1];
	unsigned long value = std::strtoul(str, &endPtr, 10);
	if (endPtr != str + std::strlen(str))
		return false;

	++i;
	index = value;
	return true;
}

bool strToCubeFace(Texture::CubeFace& face, const char* str)
{
	if (strcasecmp(str, "+x") == 0)
		face = Texture::CubeFace::PosX;
	else if (strcasecmp(str, "-x") == 0)
		face = Texture::CubeFace::NegX;
	else if (strcasecmp(str, "+y") == 0)
		face = Texture::CubeFace::PosY;
	else if (strcasecmp(str, "-y") == 0)
		face = Texture::CubeFace::NegY;
	else if (strcasecmp(str, "+z") == 0)
		face = Texture::CubeFace::PosZ;
	else if (strcasecmp(str, "-z") == 0)
		face = Texture::CubeFace::NegZ;
	else
		return false;
	return true;
}

bool readCubeFace(Texture::CubeFace& face, int& i, int argc, const char** argv,
	bool optional = false)
{
	if (i >= argc - 1)
		return false;

	bool success = strToCubeFace(face, argv[i + 1]);
	if (success)
		++i;
	else if (!optional)
		std::cerr << "error: unknown cube face " << argv[i + 1];

	return success;
}

bool strToMipReplace(Texture::MipReplacement& replace, const char* str)
{
	if (strcasecmp(str, "once") == 0)
		replace = Texture::MipReplacement::Once;
	else if (strcasecmp(str, "continue") == 0)
		replace = Texture::MipReplacement::Continue;
	else
		return false;
	return true;
}

bool readMipReplace(Texture::MipReplacement& replace, int& i, int argc, const char** argv)
{
	if (i >= argc - 1)
		return false;

	bool success = strToMipReplace(replace, argv[i + 1]);
	if (success)
		++i;
	return success;
}

const char* faceName(Texture::CubeFace face)
{
	static const char* faceNames[] = {"+x", "-x", "+y", "-y", "+z", "-z"};
	assert(static_cast<unsigned int>(face) < sizeof(faceNames)/sizeof(*faceNames));
	return faceNames[static_cast<unsigned int>(face)];
}

const char* formatName(Texture::Format format)
{
	for (const auto& formatInfo : formatMap)
	{
		if (formatInfo.first == format)
			return formatInfo.second;
	}
	return "Unknown";
}

const char* typeName(Texture::Type type)
{
	static const char* typeNames[] = {"unorm", "snorm", "uint", "int", "ufloat", "float"};
	assert(static_cast<unsigned int>(type) < sizeof(typeNames)/sizeof(*typeNames));
	return typeNames[static_cast<unsigned int>(type)];
}

const char* fileTypeName(Texture::FileType type)
{
	static const char* typeNames[] = {"unknown", "DDS", "KTX", "PVR"};
	assert(static_cast<unsigned int>(type) < sizeof(typeNames)/sizeof(*typeNames));
	return typeNames[static_cast<unsigned int>(type)];
}

bool isCharInStr(char c, const char* str)
{
	for (const char* s = str; *s; ++s)
	{
		if (c == *s)
			return true;
	}
	return false;
}

int readUntilChar(std::string& outString, std::istream& stream, const char* delimiters)
{
	do
	{
		int c = stream.get();
		if (c < 0 || isCharInStr(static_cast<char>(c), delimiters))
			return c;

		outString.push_back(static_cast<char>(c));
	} while (true);
}

int readNextToken(
	std::string& outString, std::istream& stream, const char* delimiters, const char* terminators,
	int lastDelim = 0, bool append = false)
{
	if (append)
	{
		// May need to keep the previous delimiter.
		if (!outString.empty() && lastDelim >= 0 &&
			!isCharInStr(static_cast<char>(lastDelim), delimiters))
		{
			outString.push_back(static_cast<char>(lastDelim));
		}
	}
	else
		outString.clear();
	while (stream.good() && lastDelim >= 0 &&
		!isCharInStr(static_cast<char>(lastDelim), terminators))
	{
		lastDelim = readUntilChar(outString, stream, delimiters);
		if (!outString.empty())
			return lastDelim;
	}

	return lastDelim;
}

bool readSize(int& size, int& i, int argc, const char** argv)
{
	if (i >= argc - 1)
		return false;

	++i;
	if (strcasecmp(argv[i], "nextpo2") == 0)
		size = CommandLine::NextPO2;
	else if (strcasecmp(argv[i], "nearestpo2") == 0)
		size = CommandLine::NearestPO2;
	else if (strcasecmp(argv[i], "width") == 0)
		size = CommandLine::Width;
	else if (strcasecmp(argv[i], "width-nextpo2") == 0)
		size = CommandLine::WidthNextPO2;
	else if (strcasecmp(argv[i], "width-nearestpo2") == 0)
		size = CommandLine::WidthNearestPO2;
	else if (strcasecmp(argv[i], "height") == 0)
		size = CommandLine::Height;
	else if (strcasecmp(argv[i], "height-nextpo2") == 0)
		size = CommandLine::HeightNextPO2;
	else if (strcasecmp(argv[i], "height-nearestpo2") == 0)
		size = CommandLine::HeightNearestPO2;
	else if (strcasecmp(argv[i], "min") == 0)
		size = CommandLine::Min;
	else if (strcasecmp(argv[i], "min-nextpo2") == 0)
		size = CommandLine::MinNextPO2;
	else if (strcasecmp(argv[i], "min-nearestpo2") == 0)
		size = CommandLine::MinNearestPO2;
	else if (strcasecmp(argv[i], "max") == 0)
		size = CommandLine::Max;
	else if (strcasecmp(argv[i], "max-nextpo2") == 0)
		size = CommandLine::MaxNextPO2;
	else if (strcasecmp(argv[i], "max-nearestpo2") == 0)
		size = CommandLine::MaxNearestPO2;
	else
	{
		char* endPtr;
		unsigned long value = std::strtol(argv[i], &endPtr, 10);
		if (endPtr != argv[i] + std::strlen(argv[i]))
		{
			std::cerr << "error: invalid size " << argv[i] << std::endl;
			return false;
		}

		size = static_cast<int>(value);
	}

	return true;
}

bool readChannel(Image::Channel& channel, char c)
{
	switch (c)
	{
		case 'r':
		case 'R':
			channel = Image::Channel::Red;
			return true;
		case 'g':
		case 'G':
			channel = Image::Channel::Green;
			return true;
		case 'b':
		case 'B':
			channel = Image::Channel::Blue;
			return true;
		case 'a':
		case 'A':
			channel = Image::Channel::Alpha;
			return true;
		case 'x':
		case 'X':
			channel = Image::Channel::None;
			return true;
		default:
			return false;
	}
}

bool readFilter(Image::ResizeFilter& filter, int& i, int argc, const char** argv)
{
	if (i >= argc - 1)
		return false;

	if (strcasecmp(argv[i + 1], "box") == 0)
		filter = Image::ResizeFilter::Box;
	else if (strcasecmp(argv[i + 1], "linear") == 0)
		filter = Image::ResizeFilter::Linear;
	else if (strcasecmp(argv[i + 1], "cubic") == 0)
		filter = Image::ResizeFilter::Cubic;
	else if (strcasecmp(argv[i + 1], "catmull-rom") == 0)
		filter = Image::ResizeFilter::CatmullRom;
	else if (strcasecmp(argv[i + 1], "b-spline") == 0)
		filter = Image::ResizeFilter::BSpline;
	else
		return false;

	++i;
	return true;
}

bool readImageList(std::vector<std::string>& images, const char* fileName)
{
	std::ifstream stream(fileName);
	if (!stream.is_open())
	{
		std::cerr << "error: couldn't open image list file '" << fileName << "'" << std::endl;
		return false;
	}

	std::string curStr;
	while (stream.good())
	{
		curStr.clear();
		readUntilChar(curStr, stream, "\n\r");
		if (!curStr.empty())
			images.push_back(curStr);
	}

	return true;
}

bool readCustomMipList(CommandLine::CustomMipImages& customMipImages, const char* fileName)
{
	std::ifstream stream(fileName);
	if (!stream.is_open())
	{
		std::cerr << "error: couldn't open custom mip file '" << fileName << "'" << std::endl;
		return false;
	}

	const char* whitespace = "\t\n\v\f\r ";
	const char* newline = "\n\r";
	std::string curStr;
	while (stream.good())
	{
		int delim = readNextToken(curStr, stream, whitespace, newline);
		if (curStr.empty())
			continue;

		char* endPtr;
		const char* str = curStr.c_str();
		std::size_t level = std::strtoul(str, &endPtr, 10);
		if (endPtr != str + curStr.size())
		{
			std::cerr << "error: invalid mip level " << curStr << std::endl;
			return false;
		}

		delim = readNextToken(curStr, stream, whitespace, newline, delim);

		std::size_t depth = 0;
		str = curStr.c_str();
		std::size_t value = std::strtoul(str, &endPtr, 10);
		if (endPtr == str + curStr.size())
		{
			depth = value;
			delim = readNextToken(curStr, stream, whitespace, newline, delim);
		}

		auto face = Texture::CubeFace::PosX;
		if (strToCubeFace(face, curStr.c_str()))
			delim = readNextToken(curStr, stream, whitespace, newline, delim);

		auto replace = Texture::MipReplacement::Continue;
		if (strToMipReplace(replace, curStr.c_str()))
			delim = readNextToken(curStr, stream, whitespace, newline, delim);

		// Read the rest of the line, should have already gotten the next token up to any
		// whitespace.
		readNextToken(curStr, stream, newline, newline, delim, true);
		if (curStr.empty())
		{

			std::cerr << "error: no file provided for custom mip level " << level << ", depth " <<
				depth << ", face "<< faceName(face) << " already provided" <<
				std::endl;
			return false;
		}

		if (!customMipImages.emplace(
				Texture::ImageIndex(face, static_cast<unsigned int>(level),
					static_cast<unsigned int>(depth)),
				CommandLine::CustomMipImage(curStr, replace)).second)
		{
			std::cerr << "error: custom mip for level " << level << ", depth " << depth <<
				", face "<< faceName(face) << " already provided" << std::endl;
			return false;
		}
	}

	return true;
}

bool validate(CommandLine& args)
{
	if (args.images.empty())
	{
		std::cerr << "error: an input image must be provided" << std::endl;
		return false;
	}

	switch (args.imageType)
	{
		case CommandLine::ImageType::Image:
			if (args.images.size() != 1)
			{
				std::cerr << "error: only 1 input image may be provided for a standard texture" <<
					std::endl;
				return false;
			}
			break;
		case CommandLine::ImageType::Cube:
			if (args.images.size() != 6)
			{
				std::cerr << "error: 6 images must be provided for a cubemap texture" << std::endl;
				return false;
			}

			if (args.dimension != Texture::Dimension::Dim2D)
			{
				std::cerr << "error: cubemap texture must have a dimension of 2" << std::endl;
				return false;
			}
			args.dimension = Texture::Dimension::Cube;
			break;
		case CommandLine::ImageType::CubeArray:
			if (args.images.size() % 6 != 0)
			{
				std::cerr <<
					"error: a multiple of 6 images must be provided for a cubemap texture" <<
					std::endl;
				return false;
			}

			if (args.dimension != Texture::Dimension::Dim2D)
			{
				std::cerr << "error: cubemap texture must have a dimension of 2" << std::endl;
				return false;
			}
			args.dimension = Texture::Dimension::Cube;
			break;
		default:
			break;
	}

	for (const std::string& image : args.images)
	{
		if (image.empty())
		{
			std::cerr << "error: not all images were provided" << std::endl;
			return false;
		}
	}

	if (args.format == Texture::Format::Unknown)
	{
		std::cerr << "error: texture file format cannot be determined" << std::endl;
		return false;
	}

	if (!args.output)
	{
		std::cerr << "error: output file must be provided" << std::endl;
		return false;
	}

	if (args.fileType == Texture::FileType::Auto)
	{
		args.fileType = Texture::fileType(args.output);
		if (args.fileType == Texture::FileType::Auto)
		{
			std::cerr << "error: cannot deduce file type for '" << args.output << "'" << std::endl;
			return false;
		}
	}

	if (!Texture::isFormatValid(args.format, args.type, args.fileType))
	{
		std::cerr << "error: file format " << fileTypeName(args.fileType) <<
			" doesn't support format " << formatName(args.format) << " with type " <<
			typeName(args.type) << std::endl;
		return false;
	}

	if (args.textureColorSpace == ColorSpace::sRGB &&
		!Texture::hasNativeSRGB(args.format, args.type))
	{
		args.textureColorSpace = ColorSpace::Linear;
	}

	if (args.imageColorSpace == ColorSpace::sRGB && args.log != CommandLine::Log::Quiet)
	{
		switch (args.format)
		{
			case Texture::Format::R4G4:
			case Texture::Format::R4G4B4A4:
			case Texture::Format::B4G4R4A4:
			case Texture::Format::A4R4G4B4:
			case Texture::Format::R5G6B5:
			case Texture::Format::B5G6R5:
			case Texture::Format::R5G5B5A1:
			case Texture::Format::A1R5G5B5:
			case Texture::Format::R8:
			case Texture::Format::R8G8:
			case Texture::Format::ETC1:
				std::cerr <<
					"warning: performing sRGB conversion for a low-precision texture format" << std::endl <<
					"         this may result in a noticeable loss of quality" << std::endl;
				break;

			default:
				break;
		}
	}

	if (args.mipLevels <= 1 && !args.customMipImages.empty())
	{
		std::cerr << "error: cannot specify custom mip images without generating mipmaps" <<
			std::endl;
		return false;
	}

	auto depth = static_cast<unsigned int>(args.images.size());
	std::vector<unsigned int> levelDepthCounts;
	for (const auto& customMip : args.customMipImages)
	{
		if (customMip.first.mipLevel == 0)
		{
			std::cerr << "error: cannot provide custom mip for level 0" << std::endl;
			return false;
		}

		unsigned int thisDepth = depth;
		if (args.dimension == Texture::Dimension::Dim3D)
		{
			thisDepth = std::max(depth >> customMip.first.mipLevel, 1U);
			if (levelDepthCounts.size() <= customMip.first.mipLevel)
				levelDepthCounts.resize(customMip.first.mipLevel + 1, 0);
			++levelDepthCounts[customMip.first.mipLevel];
		}

		if (customMip.first.depth >= thisDepth)
		{
			std::cerr << "error: custom mip depth " << customMip.first.depth <<
				" out of range for level " << customMip.first.mipLevel << std::endl;
			return false;
		}
	}

	// 3D textures must have either no depths or all depths provided.
	for (unsigned int i = 0; i < levelDepthCounts.size(); ++i)
	{
		unsigned int thisDepth = std::max(depth >> i, 1U);
		if (levelDepthCounts[i] > 0 && levelDepthCounts[i] != thisDepth)
		{
			std::cerr << "error: must provide custom mips for all depths in level " << i <<
				std::endl;
			return false;
		}
	}

	return true;
}

} // namespace

bool CommandLine::parse(int argc, const char** argv)
{
	if (argc == 1)
	{
		printHelp(programName(argv[0]));
		return false;
	}

	// Check if help was requested.
	for (int i = 1; i < argc; ++i)
	{
		if (matches(argv[i], "-h", "--help"))
		{
			printHelp(programName(argv[0]));
			return false;
		}
	}

	// Parse command line.
	bool success = true;
	bool imageFile = false;
	bool typeSet = false;
	bool alphaSet = false;
	const char* invalidInputsError = "error: cannot mix different types of image inputs";
	for (int i = 1; i < argc; ++i)
	{
		if (matches(argv[i], "-j", "--jobs"))
		{
			jobs = Texture::allCores;
			if (i + 1 < argc)
			{
				char* endPtr;
				unsigned long jobsl = std::strtol(argv[i + 1], &endPtr, 10);
				if (endPtr == argv[i + 1] + std::strlen(argv[i + 1]))
				{
					jobs = static_cast<unsigned int>(jobsl);
					++i;
				}
			}
		}
		else if (matches(argv[i], "-q", "--quiet"))
			log = Log::Quiet;
		else if (matches(argv[i], "-v", "--verbose"))
			log = Log::Verbose;
		else if (matches(argv[i], "-i", "--input"))
		{
			if (!images.empty())
			{
				std::cerr << invalidInputsError << std::endl;
				success = false;
				break;
			}

			if (i == argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			images.push_back(argv[++i]);
		}
		else if (matches(argv[i], "-a", "--array"))
		{
			if (images.empty())
				imageType = ImageType::Array;
			else if (imageType != ImageType::Array || imageFile)
			{
				std::cerr << invalidInputsError << std::endl;
				success = false;
				break;
			}

			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 or 2 arguments" <<
					std::endl;
				success = false;
				break;
			}

			std::size_t index = images.size();
			if (i < argc - 2)
				readIndex(index, i, argc, argv);

			if (index >= images.size())
				images.resize(index + 1);

			if (!images[index].empty())
			{
				std::cerr << "error: image for index " << index << " already provided" << std::endl;
				success = false;
				break;
			}
			images[index] = argv[++i];
		}
		else if (matches(argv[i], "-c", "--cube"))
		{
			if (images.empty())
			{
				imageType = ImageType::Cube;
				images.resize(6);
			}
			else if (imageType != ImageType::Cube || imageFile)
			{
				std::cerr << invalidInputsError << std::endl;
				success = false;
				break;
			}

			if (i >= argc - 2)
			{
				std::cerr << "error: command " << argv[i] << " requires 2 arguments" << std::endl;
				success = false;
				break;
			}

			Texture::CubeFace face;
			if (!readCubeFace(face, i, argc, argv))
			{
				success = false;
				break;
			}

			assert(i < argc - 1);
			++i;
			if (!images[static_cast<int>(face)].empty())
			{
				std::cerr << "error: image for face " << faceName(face) << " already provided" <<
					std::endl;
				success = false;
				break;
			}
			images[static_cast<int>(face)] = argv[i];
		}
		else if (matches(argv[i], "-C", "--cube-array"))
		{
			if (images.empty())
				imageType = ImageType::CubeArray;
			else if (imageType != ImageType::CubeArray || imageFile)
			{
				std::cerr << invalidInputsError << std::endl;
				success = false;
				break;
			}

			if (i >= argc - 3)
			{
				std::cerr << "error: command " << argv[i] << " requires 3 arguments" << std::endl;
				success = false;
				break;
			}

			std::size_t cubeIndex;
			if (!readIndex(cubeIndex, i, argc, argv))
			{
				std::cerr << "error: invalid index " << argv[i] << std::endl;
				success = false;
				break;
			}

			Texture::CubeFace face;
			if (!readCubeFace(face, i, argc, argv))
			{
				success = false;
				break;
			}

			assert(i < argc - 1);
			++i;
			if (cubeIndex*6 >= images.size())
				images.resize((cubeIndex + 1)*6);

			std::size_t index = cubeIndex*6 + static_cast<std::size_t>(face);
			if (!images[index].empty())
			{
				std::cerr << "error: image for index " << cubeIndex << " and face " <<
					faceName(face) << " already provided" << std::endl;
				success = false;
				break;
			}
			images[index] = argv[i];
		}
		else if (matches(argv[i], "-I", "--input-list"))
		{
			if (!images.empty())
			{
				std::cerr << invalidInputsError << std::endl;
				success = false;
				break;
			}

			if (i >= argc - 2)
			{
				std::cerr << "error: command " << argv[i] << " requires 2 arguments" << std::endl;
				success = false;
				break;
			}

			++i;
			if (strcasecmp(argv[i], "image") == 0)
				imageType = ImageType::Image;
			else if (strcasecmp(argv[i], "array") == 0)
				imageType = ImageType::Array;
			else if (strcasecmp(argv[i], "cube") == 0)
				imageType = ImageType::Cube;
			else if (strcasecmp(argv[i], "cube-array") == 0)
				imageType = ImageType::CubeArray;
			else
			{
				std::cerr << "error: unknown image type " << argv[i] << std::endl;
				success = false;
				break;
			}

			if (!readImageList(images, argv[++i]))
			{
				success = false;
				break;
			}
		}
		else if (matches(argv[i], "-r", "--resize"))
		{
			if (i >= argc - 2)
			{
				std::cerr << "error: command " << argv[i] << " requires 2 or 3 arguments" <<
					std::endl;
				success = false;
				break;
			}

			if (!readSize(width, i, argc, argv) || !readSize(height, i, argc, argv))
			{
				success = false;
				break;
			}

			readFilter(resizeFilter, i, argc, argv);
		}
		else if (matches(argv[i], "-m", "--mipmap"))
		{
			std::size_t levels;
			if (readIndex(levels, i, argc, argv))
				mipLevels = static_cast<unsigned int>(levels);
			else
				mipLevels = Texture::allMipLevels;
			readFilter(mipFilter, i, argc, argv);
		}
		else if (matches(argv[i], "-M", "--custom-mip"))
		{
			if (i >= argc - 2)
			{
				std::cerr << "error: command " << argv[i] << " requires at least 2 arguments" <<
					std::endl;
				success = false;
				break;
			}

			std::size_t level;
			if (!readIndex(level, i, argc, argv))
			{
				std::cerr << "error: invalid mip level " << argv[i] << std::endl;
				success = false;
				break;
			}

			std::size_t depth = 0;
			readIndex(depth, i, argc, argv);

			auto face = Texture::CubeFace::PosX;
			readCubeFace(face, i, argc, argv, true);

			auto replace = Texture::MipReplacement::Continue;
			readMipReplace(replace, i, argc, argv);

			if (i >= argc - 1)
			{
				std::cerr << "error: no file provided for custom mip level " << level <<
					", depth " << depth << ", face "<< faceName(face) << " already provided" <<
					std::endl;
				success = false;
				break;
			}

			++i;
			if (!customMipImages.emplace(
					Texture::ImageIndex(face, static_cast<unsigned int>(level),
						static_cast<unsigned int>(depth)),
					CustomMipImage(argv[i], replace)).second)
			{
				std::cerr << "error: custom mip for level " << level << ", depth " << depth <<
					", face "<< faceName(face) << " already provided" << std::endl;
				success = false;
				break;
			}
		}
		else if (std::strcmp(argv[i], "--custom-mip-list") == 0)
		{
			if (i == argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			++i;
			if (!readCustomMipList(customMipImages, argv[i]))
			{
				success = false;
				break;
			}
		}
		else if (std::strcmp(argv[i], "--flipx") == 0)
			flipX = true;
		else if (std::strcmp(argv[i], "--flipy") == 0)
			flipY = true;
		else if (std::strcmp(argv[i], "--rotate") == 0)
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			char* endPtr;
			long angle = std::strtol(argv[++i], &endPtr, 10);
			if (endPtr != argv[i] + std::strlen(argv[i]) || angle % 90 != 0)
			{
				std::cerr << "error: rotate angle must be a multiple of 90 degrees" << std::endl;
				success = false;
				break;
			}

			switch ((angle/90) % 4)
			{
				case -3:
					rotate = true;
					rotateAngle = Image::RotateAngle::CCW270;
					break;
				case -2:
					rotate = true;
					rotateAngle = Image::RotateAngle::CCW180;
					break;
				case -1:
					rotate = true;
					rotateAngle = Image::RotateAngle::CCW90;
					break;
				case 0:
					rotate = false;
					break;
				case 1:
					rotate = true;
					rotateAngle = Image::RotateAngle::CW90;
					break;
				case 2:
					rotate = true;
					rotateAngle = Image::RotateAngle::CW180;
					break;
				case 3:
					rotate = true;
					rotateAngle = Image::RotateAngle::CW270;
					break;
				default:
					assert(false);
					break;
			}
		}
		else if (matches(argv[i], "-n", "--normalmap"))
		{
			normalMap = true;
			if (i < argc - 1)
			{
				if (strcasecmp(argv[i + 1], "wrap") == 0)
				{
					normalOptions = Image::NormalOptions::WrapX | Image::NormalOptions::WrapY;
					++i;
				}
				else if (strcasecmp(argv[i + 1], "wrapx") == 0)
				{
					normalOptions = Image::NormalOptions::WrapX;
					++i;
				}
				else if (strcasecmp(argv[i + 1], "wrapy") == 0)
				{
					normalOptions = Image::NormalOptions::WrapY;
					++i;
				}
			}

			if (i < argc - 1)
			{
				char* endPtr;
				double h = std::strtod(argv[i + 1], &endPtr);
				if (endPtr == argv[i + 1] + std::strlen(argv[i + 1]))
				{
					normalHeight = h;
					++i;
				}
			}
		}
		else if (matches(argv[i], "-g", "--grayscale"))
			grayscale = true;
		else if (matches(argv[i], "-s", "--swizzle"))
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			swizzle = true;
			const char* swizzleCode = argv[++i];
			bool parsed = true;
			if (std::strlen(swizzleCode) != 4)
				parsed = false;
			if (parsed && !readChannel(redSwzl, argv[i][0]))
				parsed = false;
			if (parsed && !readChannel(greenSwzl, argv[i][1]))
				parsed = false;
			if (parsed && !readChannel(blueSwzl, argv[i][2]))
				parsed = false;
			if (parsed && !readChannel(alphaSwzl, argv[i][3]))
				parsed = false;

			if (!parsed)
			{
				std::cerr << "error: swizzle must contain 4 elements of r, g, b, a, or x" <<
					std::endl;
				success = false;
				break;
			}

			if (redSwzl == Image::Channel::None)
				colorMask.r = false;
			if (greenSwzl == Image::Channel::None)
				colorMask.g = false;
			if (blueSwzl == Image::Channel::None)
				colorMask.b = false;
			if (alphaSwzl == Image::Channel::None)
				colorMask.a = false;

			if (!alphaSet && !colorMask.a)
				alpha = Texture::Alpha::None;
		}
		else if (std::strcmp(argv[i], "--srgb") == 0)
			imageColorSpace = textureColorSpace = ColorSpace::sRGB;
		else if (std::strcmp(argv[i], "--pre-multiply") == 0)
		{
			preMultiply = true;
			if (!alphaSet && colorMask.a)
				alpha = Texture::Alpha::PreMultiplied;
		}
		else if (matches(argv[i], "-d", "--dimension"))
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			++i;
			if (std::strcmp(argv[i], "1") == 0)
				dimension = Texture::Dimension::Dim1D;
			else if (std::strcmp(argv[i], "2") == 0)
				dimension = Texture::Dimension::Dim2D;
			else if (std::strcmp(argv[i], "3") == 0)
				dimension = Texture::Dimension::Dim3D;
			else
			{
				std::cerr << "error: unknown dimension " << argv[i] << std::endl;
				success = false;
				break;
			}
		}
		else if (matches(argv[i], "-f", "--format"))
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			++i;
			for (const auto& formatInfo : formatMap)
			{
				if (strcasecmp(formatInfo.second, argv[i]) == 0)
				{
					format = formatInfo.first;
					break;
				}
			}

			if (format == Texture::Format::Unknown)
			{
				std::cerr << "error: unknown format " << argv[i] << std::endl;
				success = false;
				break;
			}

			// Unique default types
			if (!typeSet)
			{
				if (format == Texture::Format::B10G11R11_UFloat ||
					format == Texture::Format::E5B9G9R9_UFloat ||
					format == Texture::Format::BC6H)
				{
					type = Texture::Type::UFloat;
				}
			}
		}
		else if (matches(argv[i], "-t", "--type"))
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			++i;
			if (strcasecmp(argv[i], "unorm") == 0)
				type = Texture::Type::UNorm;
			else if (strcasecmp(argv[i], "snorm") == 0)
				type = Texture::Type::SNorm;
			else if (strcasecmp(argv[i], "uint") == 0)
				type = Texture::Type::UInt;
			else if (strcasecmp(argv[i], "int") == 0)
				type = Texture::Type::Int;
			else if (strcasecmp(argv[i], "ufloat") == 0)
				type = Texture::Type::UFloat;
			else if (strcasecmp(argv[i], "float") == 0)
				type = Texture::Type::Float;
			else
			{
				std::cerr << "error: unknown type " << argv[i] << std::endl;
				success = false;
				break;
			}
		}
		else if (std::strcmp(argv[i], "--alpha") == 0)
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			++i;
			alphaSet = true;
			if (strcasecmp(argv[i], "none") == 0)
				alpha = Texture::Alpha::None;
			else if (strcasecmp(argv[i], "standard") == 0)
				alpha = Texture::Alpha::Standard;
			else if (strcasecmp(argv[i], "pre-multiplied") == 0)
				alpha = Texture::Alpha::PreMultiplied;
			else if (strcasecmp(argv[i], "encoded") == 0)
				alpha = Texture::Alpha::Encoded;
			else
			{
				std::cerr << "error: unknown alpha type " << argv[i] << std::endl;
				success = false;
				break;
			}
		}
		else if (matches(argv[i], "-Q", "--quality"))
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			++i;
			if (strcasecmp(argv[i], "lowest") == 0)
				quality = Texture::Quality::Lowest;
			else if (strcasecmp(argv[i], "low") == 0)
				quality = Texture::Quality::Low;
			else if (strcasecmp(argv[i], "normal") == 0)
				quality = Texture::Quality::Normal;
			else if (strcasecmp(argv[i], "high") == 0)
				quality = Texture::Quality::High;
			else if (strcasecmp(argv[i], "highest") == 0)
				quality = Texture::Quality::Highest;
			else
			{
				std::cerr << "error: unknown quality " << argv[i] << std::endl;
				success = false;
				break;
			}
		}
		else if (matches(argv[i], "-o", "--output"))
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			if (output)
			{
				std::cerr << "error: output path already provided" << std::endl;
				success = false;
				break;
			}

			output = argv[++i];
		}
		else if (std::strcmp(argv[i], "--file-format") == 0)
		{
			if (i >= argc - 1)
			{
				std::cerr << "error: command " << argv[i] << " requires 1 argument" << std::endl;
				success = false;
				break;
			}

			++i;
			if (strcasecmp(argv[i], "dds") == 0)
				fileType = Texture::FileType::DDS;
			else if (strcasecmp(argv[i], "ktx") == 0)
				fileType = Texture::FileType::KTX;
			else if (strcasecmp(argv[i], "pvr") == 0)
				fileType = Texture::FileType::PVR;
			else
			{
				std::cerr << "error: unknown file format " << argv[i] << std::endl;
				success = false;
				break;
			}
		}
		else if (std::strcmp(argv[i], "--create-dir") == 0)
			createOutputDir = true;
		else
		{
			std::cerr << "error: unknown option: " << argv[i] << std::endl;
			success = false;
		}
	}

	if (success)
		success = validate(*this);

	if (!success)
		std::cerr << "Run " << programName(argv[0]) << " -h for usage." << std::endl;
	return success;
}
