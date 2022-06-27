/*
 * Copyright 2022 Aaron Barany
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

#include "Shared.h"
#include <istream>

namespace cuttlefish
{

void readStreamData(std::vector<std::uint8_t>& outData, std::istream& stream)
{
	// Try to get the full size of the data by seeking.
	stream.seekg(0, std::ios_base::end);
	std::istream::pos_type pos = stream.tellg();
	if (pos < 0)
	{
		// Fall back to reading bytes if we can't seek. Clear the stream bits in case the seek
		// set a fail bit.
		stream.clear();

		constexpr std::size_t bufferSize = 4096;
		char buffer[bufferSize];
		outData.clear();
		do
		{
			stream.read(buffer, bufferSize);
			auto readSize = static_cast<std::size_t>(stream.gcount());
			if (readSize == 0)
				return;

			outData.insert(outData.end(), buffer, buffer + readSize);
		} while (true);
		return;
	}
	stream.seekg(0, std::ios_base::beg);

	// Resize will zero-initialize the vector. Theoretically calling reserve() and using
	// std::istreambuf_iterator to populate the data would have the fewest data writes, but is
	// more function call overhead and slower in practice.
	auto size = static_cast<std::size_t>(pos);
	outData.resize(size);
	stream.read(reinterpret_cast<char*>(outData.data()), size);
}

} // namespace cuttlefish
