/*
 * Copyright 2017-2021 Aaron Barany
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

#if CUTTLEFISH_HAS_ASTC

#include "AstcConverter.h"
#include <cuttlefish/Color.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <list>
#include <mutex>
#include <thread>

#include "astcenc.h"

// Need to stub out these functions, since they don't work on all compilers. Return true for all,
// since it will only be called if support was compiled in.
int cpu_supports_sse41()
{
	return 1;
}

int cpu_supports_popcnt()
{
	return 1;
}

int cpu_supports_avx2()
{
	return 1;
}

namespace cuttlefish
{

namespace
{

const unsigned int blockSize = 16;
const unsigned int maxBlockDim = 12;

class AstcContextManager
{
public:
	static const unsigned int cacheSize;

	astcenc_context* createContext(const astcenc_config& config)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
		{
			if (std::memcmp(&config, &it->config, sizeof(astcenc_config)) != 0)
				continue;

			astcenc_context* context = it->context;
			it->context = nullptr;
			m_cache.erase(it);
			return context;
		}

		astcenc_context* context = nullptr;
		astcenc_context_alloc(&config, 1, &context);
		assert(context);
		return context;
	}

	void destroyContext(astcenc_context* context, const astcenc_config& config)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		while (m_cache.size() >= cacheSize)
			m_cache.pop_back();

		m_cache.emplace_front(context, config);
	}

private:
	struct ContextInfo
	{
		ContextInfo(astcenc_context* _context, const astcenc_config& _config)
			: context(_context), config(_config)
		{
		}

		ContextInfo(const ContextInfo&) = delete;
		ContextInfo& operator=(const ContextInfo&) = delete;

		~ContextInfo()
		{
			astcenc_context_free(context);
		}

		astcenc_context* context;
		astcenc_config config;
	};

	std::list<ContextInfo> m_cache;
	std::mutex m_mutex;
};

const unsigned int AstcContextManager::cacheSize = 3*std::thread::hardware_concurrency();

AstcContextManager g_contextManager;

} // namespace

struct AstcConverter::AstcData
{
	astcenc_swizzle swizzle;
	astcenc_config config;
};

class AstcConverter::AstcThreadData : public Converter::ThreadData
{
public:
	AstcThreadData(unsigned int blockX, unsigned int blockY, const astcenc_config& _config)
		: config(&_config), context(g_contextManager.createContext(_config))
	{
		dummyImage.dim_x = blockX;
		dummyImage.dim_y = blockY;
		dummyImage.dim_z = 1;
		dummyImage.data_type = ASTCENC_TYPE_F32;
		dummyImage.data = nullptr;
	}

	~AstcThreadData()
	{
		g_contextManager.destroyContext(context, *config);
	}

	astcenc_image dummyImage;
	const astcenc_config* config;
	astcenc_context* context;
};

AstcConverter::AstcConverter(const Texture& texture, const Image& image, unsigned int blockX,
	unsigned int blockY, Texture::Quality quality)
	: Converter(image), m_blockX(blockX), m_blockY(blockY),
	m_jobsX((image.width() + blockX - 1)/blockX), m_jobsY((image.height() + blockY - 1)/blockY),
	m_astcData(new AstcData)
{
	m_astcData->swizzle.r = texture.colorMask().r ? ASTCENC_SWZ_R : ASTCENC_SWZ_0;
	m_astcData->swizzle.g = texture.colorMask().g ? ASTCENC_SWZ_G : ASTCENC_SWZ_0;
	m_astcData->swizzle.b = texture.colorMask().b ? ASTCENC_SWZ_B : ASTCENC_SWZ_0;
	if (texture.colorMask().a)
	{
		m_astcData->swizzle.a = texture.alphaType() == Texture::Alpha::None ?
			ASTCENC_SWZ_1 : ASTCENC_SWZ_A;
	}
	else
		m_astcData->swizzle.a = ASTCENC_SWZ_0;

	astcenc_profile profile;
	if (texture.type() == Texture::Type::UFloat)
	{
		if (texture.alphaType() == Texture::Alpha::None ||
			texture.alphaType() == Texture::Alpha::PreMultiplied)
		{
			profile = ASTCENC_PRF_HDR_RGB_LDR_A;
		}
		else
			profile = ASTCENC_PRF_HDR;
	}
	else
		profile = ASTCENC_PRF_LDR;

	unsigned int flags = 0;
	if (texture.alphaType() == Texture::Alpha::Standard ||
		texture.alphaType() == Texture::Alpha::PreMultiplied)
	{
		flags |= ASTCENC_FLG_USE_ALPHA_WEIGHT;
	}
	if (image.colorSpace() == ColorSpace::sRGB)
		flags |= ASTCENC_FLG_USE_PERCEPTUAL;

	float preset;
	switch (quality)
	{
		case Texture::Quality::Lowest:
			preset = ASTCENC_PRE_FASTEST;
			break;
		case Texture::Quality::Low:
			preset = ASTCENC_PRE_FAST;
			break;
		case Texture::Quality::Normal:
			preset = ASTCENC_PRE_MEDIUM;
			break;
		case Texture::Quality::High:
			preset = ASTCENC_PRE_THOROUGH;
			break;
		case Texture::Quality::Highest:
			preset = ASTCENC_PRE_EXHAUSTIVE;
			break;
		default:
			assert(false);
			return;
	}

	astcenc_config_init(profile, blockX, blockY, 1, preset, flags, &m_astcData->config);

	assert(texture.type() == Texture::Type::UNorm || texture.type() == Texture::Type::UFloat);
	data().resize(m_jobsX*m_jobsY*blockSize);
}

AstcConverter::~AstcConverter()
{
	delete m_astcData;
}

void AstcConverter::process(unsigned int x, unsigned int y, ThreadData* threadData)
{
	ColorRGBAf imageData[maxBlockDim*maxBlockDim];
	void* imageRows[maxBlockDim];
	for (unsigned int j = 0, index = 0; j < m_blockY; ++j)
	{
		imageRows[j] = imageData + index;
		auto scanline = reinterpret_cast<const ColorRGBAf*>(image().scanline(
			std::min(y*m_blockY + j, image().height() - 1)));
		for (unsigned int i = 0; i < m_blockX; ++i, ++index)
		{
			unsigned int scanlineIdx = std::min(x*m_blockX + i, image().width() - 1);
			imageData[index] = scanline[scanlineIdx];
		}
	}

	auto block = data().data() + (y*m_jobsX + x)*blockSize;
	auto astcThreadData = static_cast<AstcThreadData*>(threadData);
	astcThreadData->dummyImage.data = imageRows;
	astcenc_compress_image(astcThreadData->context, &astcThreadData->dummyImage,
		m_astcData->swizzle, block, blockSize, 0);
	astcenc_compress_reset(astcThreadData->context);
}

std::unique_ptr<Converter::ThreadData> AstcConverter::createThreadData()
{
	return std::unique_ptr<ThreadData>(new AstcThreadData(m_blockX, m_blockY, m_astcData->config));
}

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ASTC
