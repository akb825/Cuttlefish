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

#include <cuttlefish/Config.h>
#include "Converter.h"
#include <mutex>

#if CUTTLEFISH_HAS_ASTC

namespace cuttlefish
{

class AstcConverter : public Converter
{
public:
	static const unsigned int blockSize = 16;

	AstcConverter(const Texture& texture, const Image& image, unsigned int blockX,
		unsigned int blockY, Texture::Quality quality);
	~AstcConverter();

	unsigned int jobsX() const override {return m_jobsX;}
	unsigned int jobsY() const override {return m_jobsY;}
	void process(unsigned int x, unsigned int y) override;

private:
	unsigned int m_blockX;
	unsigned int m_blockY;
	unsigned int m_jobsX;
	unsigned int m_jobsY;
	Texture::ColorMask m_colorMask;
	bool m_hdr;

	bool m_alphaWeight;
	int m_partitionsToTest;
	float m_oplimit;
	float m_mincorrel;
	float m_averageErrorLimit;
	float m_blockModeCutoff;
	int m_maxIters;

	static std::mutex m_mutex;
};

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ASTC
