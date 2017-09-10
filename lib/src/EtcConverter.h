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

#if CUTTLEFISH_HAS_ETC

#if CUTTLEFISH_MSC
#pragma warning(push)
#pragma warning(disable: 4005)
#endif

#include <EtcImage.h>

#if CUTTLEFISH_MSC
#pragma warning(pop)
#endif

namespace Etc { class Image; }

namespace cuttlefish
{

class EtcConverter : public Converter
{
public:
	static const unsigned int blockDim = 4;

	EtcConverter(const Texture& texture, const Image& image, Texture::Quality quality);

	unsigned int jobsX() const override {return m_jobsX;}
	unsigned int jobsY() const override {return m_jobsY;}
	void process(unsigned int x, unsigned int y) override;

private:
	unsigned int m_blockSize;
	unsigned int m_jobsX;
	unsigned int m_jobsY;
	Etc::Image::Format m_format;
	Etc::ErrorMetric m_metric;
	float m_effort;
};

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ETC
