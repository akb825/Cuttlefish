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

#include "S3tcCompress.h"

#if CUTTLEFISH_HAS_S3TC

#if CUTTLEFISH_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#endif

#include "nvimage/ImageIO.h"
#include "nvtt/squish/colourset.h"
#include "nvtt/squish/weightedclusterfit.h"
#include "nvtt/OptimalCompressDXT.h"
#include "nvtt/QuickCompressDXT.h"

#if CUTTLEFISH_CLANG
#pragma GCC diagnostic pop
#endif

// Stub out functions.
namespace nv
{

Image* ImageIO::load(const char*)
{
	return nullptr;
}

} // namespace nv

namespace cuttlefish
{
} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_S3TC
