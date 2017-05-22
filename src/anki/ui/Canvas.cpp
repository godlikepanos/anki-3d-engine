// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/Canvas.h>
#include <anki/ui/Font.h>

namespace anki
{

Canvas::~Canvas()
{
	nk_free(&m_nkCtx);
}

Error Canvas::init(FontPtr font)
{
	m_font = font;

	nk_allocator alloc = makeNkAllocator(&getAllocator().getMemoryPool());
	if(!nk_init(&m_nkCtx, &alloc, &m_font->getFont().handle))
	{
		ANKI_UI_LOGE("nk_init() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
