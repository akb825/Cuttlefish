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

#pragma once

#include <cuttlefish/Config.h>
#include <cuttlefish/Export.h>

#include <cassert>
#include <cstdint>

#if CUTTLEFISH_X86_32 || CUTTLEFISH_X86_64
#include <immintrin.h>
#define CUTTLEFISH_SSE 1
#define CUTTLEFISH_NEON 0
#elif CUTTLEFISH_ARM_32 || CUTTLEFISH_ARM_64
#include <arm_neon.h>
#define CUTTLEFISH_SSE 0
#define CUTTLEFISH_NEON 1
#else
#define CUTTLEFISH_SSE 0
#define CUTTLEFISH_NEON 0
#endif

#if CUTTLEFISH_SSE && CUTTLEFISH_CLANG
#define CUTTLEFISH_START_HALF_FLOAT() \
	_Pragma("clang attribute push(__attribute__((target(\"sse,sse2,f16c\"))), apply_to = function)")
#define CUTTLEFISH_END_HALF_FLOAT() _Pragma("clang attribute pop")
#elif CUTTLEFISH_SSE && CUTTLEFISH_GCC
#define CUTTLEFISH_START_HALF_FLOAT() \
	_Pragma("GCC push_options") \
	_Pragma("GCC target(\"sse,sse2,f16c\")")
#define CUTTLEFISH_END_HALF_FLOAT() _Pragma("GCC pop_options")
#else
#define CUTTLEFISH_START_HALF_FLOAT()
#define CUTTLEFISH_END_HALF_FLOAT()
#endif

namespace cuttlefish
{

// Export for unit tests.
CUTTLEFISH_EXPORT extern const bool hasHardwareHalfFloat;

CUTTLEFISH_START_HALF_FLOAT()

// NOTE: Always assume input has 4 floats, though output may be different.
inline void packHardwareHalfFloat1(std::uint16_t* result, const float* value)
{
#if CUTTLEFISH_SSE
	__m128 f = _mm_loadu_ps(value);
	__m128i h = _mm_cvtps_ph(f, 0);
	*result = static_cast<std::uint16_t>(_mm_cvtsi128_si32(h));
#elif CUTTLEFISH_NEON
	float32x4_t f = vld1q_f32(value);
	float16x4_t h = vcvt_f16_f32(f);
	vst1_lane_f16(reinterpret_cast<float16_t*>(result), h, 0);
#else
	CUTTLEFISH_UNUSED(result);
	CUTTLEFISH_UNUSED(value);
	assert(false);
#endif
}

inline void packHardwareHalfFloat2(std::uint16_t* result, const float* value)
{
#if CUTTLEFISH_SSE
	__m128 f = _mm_loadu_ps(value);
	__m128i h = _mm_cvtps_ph(f, 0);
	*reinterpret_cast<std::uint32_t*>(result) = _mm_cvtsi128_si32(h);
#elif CUTTLEFISH_NEON
	float32x4_t f = vld1q_f32(value);
	float16x4_t h = vcvt_f16_f32(f);
	vst1_lane_f16(reinterpret_cast<float16_t*>(result), h, 0);
	vst1_lane_f16(reinterpret_cast<float16_t*>(result) + 1, h, 1);
#else
	CUTTLEFISH_UNUSED(result);
	CUTTLEFISH_UNUSED(value);
	assert(false);
#endif
}

inline void packHardwareHalfFloat3(std::uint16_t* result, const float* value)
{
#if CUTTLEFISH_SSE
	__m128 f = _mm_loadu_ps(value);
	__m128i h = _mm_cvtps_ph(f, 0);
	std::uint64_t temp;
	_mm_storeu_si64(&temp, h);
	result[0] = reinterpret_cast<std::uint16_t*>(&temp)[0];
	result[1] = reinterpret_cast<std::uint16_t*>(&temp)[1];
	result[2] = reinterpret_cast<std::uint16_t*>(&temp)[2];
#elif CUTTLEFISH_NEON
	float32x4_t f = vld1q_f32(value);
	float16x4_t h = vcvt_f16_f32(f);
	vst1_lane_f16(reinterpret_cast<float16_t*>(result), h, 0);
	vst1_lane_f16(reinterpret_cast<float16_t*>(result) + 1, h, 1);
	vst1_lane_f16(reinterpret_cast<float16_t*>(result) + 2, h, 2);
#else
	CUTTLEFISH_UNUSED(result);
	CUTTLEFISH_UNUSED(value);
	assert(false);
#endif
}

inline void packHardwareHalfFloat4(std::uint16_t* result, const float* value)
{
#if CUTTLEFISH_SSE
	__m128 f = _mm_loadu_ps(value);
	__m128i h = _mm_cvtps_ph(f, 0);
	_mm_storeu_si64(result, h);
#elif CUTTLEFISH_NEON
	float32x4_t f = vld1q_f32(value);
	float16x4_t h = vcvt_f16_f32(f);
	vst1_f16(reinterpret_cast<float16_t*>(result), h);
#else
	CUTTLEFISH_UNUSED(result);
	CUTTLEFISH_UNUSED(value);
	assert(false);
#endif
}

CUTTLEFISH_END_HALF_FLOAT()

} // namespace cuttlefish
