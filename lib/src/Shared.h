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

#include <cstdint>

#define FOURCC(a, b, c, d) ((std::uint32_t)(a) | ((std::uint32_t)(b) << 8) | \
	((std::uint32_t)(c) << 16) | ((std::uint32_t)(d) << 24 ))

namespace cuttlefish
{

inline float clamp(float v, float minVal, float maxVal)
{
	if (v < minVal)
		return minVal;
	else if (v > maxVal)
		return maxVal;
	return v;
}

} // namespace cuttlefish
