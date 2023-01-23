/*
 * Copyright 2023 Aaron Barany
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

#include "HalfFloat.h"
#include <gtest/gtest.h>

#if CUTTLEFISH_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <glm/gtc/packing.hpp>

#if CUTTLEFISH_GCC
#pragma GCC diagnostic pop
#endif

namespace cuttlefish
{

TEST(HalfFloatTest, PackHardwareHalfFloat)
{
	if (!hasHardwareHalfFloat)
		return;

	float floatValues[4] = {1.2f, -3.4f, 5.6f, -7.8f};
	std::uint16_t halfFloatValues[4];
	*reinterpret_cast<std::uint64_t*>(halfFloatValues) =
		glm::packHalf4x16(*reinterpret_cast<glm::vec4*>(floatValues));

	std::uint16_t convertedValues[4] = {0, 0, 0, 0};

	packHardwareHalfFloat1(convertedValues, floatValues);
	EXPECT_EQ(halfFloatValues[0], convertedValues[0]);
	EXPECT_EQ(0, convertedValues[1]);
	EXPECT_EQ(0, convertedValues[2]);
	EXPECT_EQ(0, convertedValues[3]);

	packHardwareHalfFloat2(convertedValues, floatValues);
	EXPECT_EQ(halfFloatValues[0], convertedValues[0]);
	EXPECT_EQ(halfFloatValues[1], convertedValues[1]);
	EXPECT_EQ(0, convertedValues[2]);
	EXPECT_EQ(0, convertedValues[3]);

	packHardwareHalfFloat3(convertedValues, floatValues);
	EXPECT_EQ(halfFloatValues[0], convertedValues[0]);
	EXPECT_EQ(halfFloatValues[1], convertedValues[1]);
	EXPECT_EQ(halfFloatValues[2], convertedValues[2]);
	EXPECT_EQ(0, convertedValues[3]);

	packHardwareHalfFloat4(convertedValues, floatValues);
	EXPECT_EQ(halfFloatValues[0], convertedValues[0]);
	EXPECT_EQ(halfFloatValues[1], convertedValues[1]);
	EXPECT_EQ(halfFloatValues[2], convertedValues[2]);
	EXPECT_EQ(halfFloatValues[3], convertedValues[3]);
}

} // cuttlefish
