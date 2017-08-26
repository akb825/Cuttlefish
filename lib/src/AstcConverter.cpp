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

#include "AstcConverter.h"

#if CUTTLEFISH_HAS_ASTC

#include <stdio.h>
#include <stdlib.h>

#if CUTTLEFISH_GCC || CUTTLEFISH_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include "astc_codec_internals.h"
#if CUTTLEFISH_GCC || CUTTLEFISH_CLANG
#pragma GCC diagnostic pop
#endif

// astc-encoder uses global variables to control its operation.
int print_diagnostics;
int print_tile_errors;
int print_statistics;

int perform_srgb_transform;
int rgb_force_use_of_hdr;
int alpha_force_use_of_hdr;

// Stub out functions.
int store_tga_image(const astc_codec_image*, const char*, int)
{
	return 0;
}

astc_codec_image* load_tga_image(const char*, int , int*)
{
	return nullptr;
}

astc_codec_image* load_image_with_stb(const char*, int, int*)
{
	return nullptr;
}

int astc_codec_unlink(const char*)
{
	return 0;
}

void astc_codec_internal_error(const char *filename, int linenum)
{
	printf("Internal error: File=%s Line=%d\n", filename, linenum);
	exit(1);
}

namespace cuttlefish
{
} // namespace cuttlefish

#endif // CUTTLEFISH_HAS_ASTC
