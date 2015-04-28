// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_COMMON_H
#define ANKI_RENDERER_COMMON_H

#include "anki/Gr.h"

namespace anki {

// Forward
class Fs;
class Tm;
class Bloom;

// Render target formats
const U MS_COLOR_ATTACHMENTS_COUNT = 2;

const Array<PixelFormat, MS_COLOR_ATTACHMENTS_COUNT> 
	MS_COLOR_ATTACHMENTS_PIXEL_FORMAT = {{
	{ComponentFormat::R8G8B8A8, TransformFormat::UNORM, false},
	{ComponentFormat::R8G8B8A8, TransformFormat::UNORM, false}}};

const PixelFormat MS_DEPTH_STENCIL_ATTACHMENT_FORMAT = {
	ComponentFormat::D24, TransformFormat::FLOAT, false};

const PixelFormat FS_COLOR_ATTACHMENT_FORMAT = {
	ComponentFormat::R8G8B8, TransformFormat::UNORM, false};

} // end namespace anki

#endif
