// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Common.h>

namespace anki
{

const Array<Format, GBUFFER_COLOR_ATTACHMENT_COUNT> MS_COLOR_ATTACHMENT_PIXEL_FORMATS = {
	{Format::R8G8B8A8_UNORM, Format::R8G8B8A8_UNORM, Format::A2B10G10R10_UNORM_PACK32, Format::R16G16_SNORM}};

} // end namespace anki