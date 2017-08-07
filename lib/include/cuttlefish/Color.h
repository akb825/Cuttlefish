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
 * @brief File containing structures for various color fromats.
 *
 * Colors that have 8-bit per channel or less may have different storage orders based on the
 * platform and source image. Use the shift and mask accessors in Image to access the color
 * channels. For example, for a 16 BPP image you can use the shift and mask on a 16-bit integer.
 * For a 24 or 32 BPP image, you can divide the shift by 8 to use as an index or use the shift and
 * mask on a 32-bit integer.
 */

#include <cuttlefish/Config.h>
#include <cstdint>

namespace cuttlefish
{

/**
 * @brief Structure containing a 3 channel color with 16 bits per channel.
 */
struct ColorRGB16
{
	std::uint16_t r; ///< @brief The red channel.
	std::uint16_t g; ///< @brief The green channel.
	std::uint16_t b; ///< @brief The blue channel.
};

/**
 * @brief Structure containing a 4 channel color with 16 bits per channel.
 */
struct ColorRGBA16
{
	std::uint16_t r; ///< @brief The red channel.
	std::uint16_t g; ///< @brief The green channel.
	std::uint16_t b; ///< @brief The blue channel.
	std::uint16_t a; ///< @brief The alpha channel.
};

/**
 * @brief Structure containing a 3 channel floating point color.
 */
struct ColorRGBf
{
	float r; ///< @brief The red channel.
	float g; ///< @brief The green channel.
	float b; ///< @brief The blue channel.
};

/**
 * @brief Structure containing a 4 channel floating point color.
 */
struct ColorRGBAf
{
	float r; ///< @brief The red channel.
	float g; ///< @brief The green channel.
	float b; ///< @brief The blue channel.
	float a; ///< @brief The alpha channel.
};

/**
 * @brief Structure containing a complex number.
 */
struct Complex
{
	double r; ///< @brief The real component.
	double i; ///< @brief The imaginary component.
};

} // namespace cuttlefish
