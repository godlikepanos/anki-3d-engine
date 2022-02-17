// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/Common.h>

#define STBI_ASSERT(x) ANKI_ASSERT(x)
#define STBI_NO_FAILURE_STRINGS 1 // No need
#include <Stb/stb_image.h>
#include <Stb/stb_image_write.h>
#include <Stb/stb_image_resize.h>
