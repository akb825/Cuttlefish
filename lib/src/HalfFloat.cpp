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

#if CUTTLEFISH_WINDOWS
#include <intrin.h>
#endif

namespace cuttlefish
{

#if CUTTLEFISH_SSE && !CUTTLEFISH_WINDOWS
static void __cpuid(int cpuInfo[4], int function)
{
	cpuInfo[0] = function;
	cpuInfo[2] = 0;
	asm volatile("cpuid\n\t" : "+a"(cpuInfo[0]), "=b"(cpuInfo[1]), "+c"(cpuInfo[2]),
		"=d"(cpuInfo[3]));
}
#endif

bool checkHasHardwareHalfFloat()
{
#if CUTTLEFISH_SSE
	const int f16cBit = 1 << 29;

	int cpuInfo[4];
	__cpuid(cpuInfo, 1);
	int ecx = cpuInfo[2];
	return (ecx & f16cBit) != 0;
#elif CUTTLEFISH_NEON
	return true;
#else
	return false;
#endif
}

const bool hasHardwareHalfFloat = checkHasHardwareHalfFloat();

} // cuttlefish
