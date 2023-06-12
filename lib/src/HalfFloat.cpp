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
#elif CUTTLEFISH_SSE
#include <cpuid.h>
#endif

namespace cuttlefish
{

#if CUTTLEFISH_WINDOWS
static void __get_cpuid(unsigned int level, unsigned int* eax, unsigned int* ebx, unsigned int* ecx,
    unsigned int* edx)
{
	int cpuInfo[4];
	__cpuid(cpuInfo, level);
	*eax = cpuInfo[0];
	*ebx = cpuInfo[1];
	*ecx = cpuInfo[2];
	*edx = cpuInfo[3];
}
#endif

bool checkHasHardwareHalfFloat()
{
#if CUTTLEFISH_SSE
	const int f16cBit = 1 << 29;

    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	return (ecx & f16cBit) != 0;
#elif CUTTLEFISH_NEON
	return true;
#else
	return false;
#endif
}

const bool hasHardwareHalfFloat = checkHasHardwareHalfFloat();

} // cuttlefish
