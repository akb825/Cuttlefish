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

#include "Converter.h"
#include <cuttlefish/Texture.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <thread>
#include <utility>

namespace cuttlefish
{

static std::unique_ptr<Converter> createConverter(const Texture& texture, const Image& image)
{
	(void)texture;
	(void)image;
	return nullptr;
}

bool Converter::convert(const Texture& texture, MipImageList& images, MipTextureList& textureData,
	unsigned int threadCount)
{
	std::atomic<unsigned int> curJob(0);
	std::vector<std::pair<unsigned int, unsigned int>> jobs;
	std::vector<std::thread> threads;
	if (threadCount > 1)
		threads.reserve(threadCount);

	textureData.resize(textureData.size());
	for (unsigned int mip = 0; mip < images.size(); ++mip)
	{
		textureData[mip].resize(images[mip].size());
		for (unsigned int d = 0; d < images[mip].size(); ++d)
		{
			textureData[mip][d].resize(images[mip][d].size());
			for (unsigned int f = 0; f < images[mip][d].size(); ++f)
			{
				auto converter = createConverter(texture, images[mip][d][f]);
				if (!converter)
				{
					// If the converter can't be created, should only do so for the first one.
					assert(mip == 0 && d == 0 && f == 0);
					textureData.clear();
					return false;
				}

				unsigned int jobsX = converter->jobsX();
				unsigned int jobsY = converter->jobsY();
				jobs.resize(jobsX*jobsY);
				assert(!jobs.empty());
				for (unsigned int y = 0; y < jobsY; ++y)
				{
					for (unsigned int x = 0; x < jobsX; ++x)
						jobs[y*jobsX + x] = std::make_pair(x, y);
				}

				unsigned int curThreads = std::min(jobsX*jobsY, threadCount);
				if (curThreads == 1)
				{
					for (const std::pair<unsigned int, unsigned int>& job : jobs)
						converter->process(job.first, job.second);
				}
				else
				{
					for (unsigned int i = 0; i < curThreads; ++i)
					{
						threads.emplace_back([&curJob, &jobs, &converter]()
							{
								do
								{
									unsigned int thisJob = curJob++;
									if (thisJob >= jobs.size())
										return;

									converter->process(jobs[thisJob].first, jobs[thisJob].second);
								} while (true);
							});
					}

					for (std::thread& thread : threads)
						thread.join();
					threads.clear();
				}

				images[mip][d][f].reset();
				textureData[mip][d][f] = std::move(converter->data());
			}
		}
	}

	return true;
}

} // namespace cuttlefish
