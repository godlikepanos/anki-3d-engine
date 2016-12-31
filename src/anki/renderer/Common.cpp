// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Common.h>

namespace anki
{

const Array<PixelFormat, MS_COLOR_ATTACHMENT_COUNT> MS_COLOR_ATTACHMENT_PIXEL_FORMATS = {
	{PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM),
		PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM),
		PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM)}};

} // end namespace anki