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

#if CUTTLEFISH_HAS_PVRTC

namespace pvrtexture { class CPVRTexture; }

namespace cuttlefish
{

class PvrtcConverter : public Converter
{
public:
	static const unsigned int blockDim = 4;

	PvrtcConverter(const Texture& texture, const Image& image, Texture::Quality quality);

	unsigned int jobsX() const override {return 1;}
	unsigned int jobsY() const override {return 1;}
	void process(unsigned int x, unsigned int y, ThreadData* threadData) override;

private:
	Texture::Format m_format;
	Texture::Quality m_quality;
	bool m_premultipliedAlpha;
};

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_PVRTC
