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

#if CUTTLEFISH_HAS_S3TC

namespace cuttlefish
{

struct ColorRGBAf;

class S3tcConverter : public Converter
{
public:
	static const unsigned int blockDim = 4;

	explicit S3tcConverter(const Texture& texture, const Image& image, unsigned int blockSize,
		Texture::Quality quality);

	unsigned int jobsX() const override {return m_jobsX;}
	unsigned int jobsY() const override {return m_jobsY;}
	void process(unsigned int x, unsigned int y) override;

	Texture::Quality quality() const {return m_quality;}
	bool weightAlpha() const {return m_weightAlpha;}
	virtual void compressBlock(void* block, ColorRGBAf* blockColors) = 0;

private:
	unsigned int m_blockSize;
	unsigned int m_jobsX;
	unsigned int m_jobsY;
	Texture::Quality m_quality;
	bool m_weightAlpha;
};

class Bc1Converter : public S3tcConverter
{
public:
	Bc1Converter(const Texture& texture, const Image& image, Texture::Quality quality);
	void compressBlock(void* block, ColorRGBAf* blockColors) override;
};

class Bc1AConverter : public S3tcConverter
{
public:
	Bc1AConverter(const Texture& texture, const Image& image, Texture::Quality quality);
	void compressBlock(void* block, ColorRGBAf* blockColors) override;
};

class Bc2Converter : public S3tcConverter
{
public:
	Bc2Converter(const Texture& texture, const Image& image, Texture::Quality quality);
	void compressBlock(void* block, ColorRGBAf* blockColors) override;
};

class Bc3Converter : public S3tcConverter
{
public:
	Bc3Converter(const Texture& texture, const Image& image, Texture::Quality quality);
	void compressBlock(void* block, ColorRGBAf* blockColors) override;
};

class Bc4Converter : public S3tcConverter
{
public:
	Bc4Converter(const Texture& texture, const Image& image, Texture::Quality quality,
		bool keepSign);
	void compressBlock(void* block, ColorRGBAf* blockColors) override;

private:
	bool m_signed;
};

class Bc5Converter : public S3tcConverter
{
public:
	Bc5Converter(const Texture& texture, const Image& image, Texture::Quality quality,
		bool keepSign);
	void compressBlock(void* block, ColorRGBAf* blockColors) override;

private:
	bool m_signed;
};

class Bc6HConverter : public S3tcConverter
{
public:
	Bc6HConverter(const Texture& texture, const Image& image, Texture::Quality quality,
		bool keepSign);
	~Bc6HConverter();
	void compressBlock(void* block, ColorRGBAf* blockColors) override;

private:
	bool m_signed;
	static std::mutex m_mutex;
};

class Bc7Converter : public S3tcConverter
{
public:
	Bc7Converter(const Texture& texture, const Image& image, Texture::Quality quality);
	void compressBlock(void* block, ColorRGBAf* blockColors) override;
};

} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_S3TC
