// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Config.h>
#include <ThirdParty/ZLib/zlib.h>

#define TINYEXR_USE_MINIZ 0
#define TINYEXR_IMPLEMENTATION 1

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#	pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <ThirdParty/TinyExr/tinyexr.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif
