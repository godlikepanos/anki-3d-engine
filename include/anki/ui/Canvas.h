// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/ui/Common.h"
#include "anki/util/Allocator.h"

namespace anki {

/// @addtogroup ui
/// @{

/// UI canvas.
class Canvas
{
public:
	Canvas(UiAllocator alloc, Painter* painter, ImageManager* imageMngr)
		: m_alloc(alloc)
		, m_painter(painter)
		, m_img(imageMngr)
	{}

	UiAllocator& getAllocator()
	{
		return m_alloc;
	}

	const UiAllocator& getAllocator() const
	{
		return m_alloc;
	}

private:
	UiAllocator m_alloc;
	WeakPtr<Painter> m_painter;
	WeakPtr<ImageManager> m_img;
};
/// @}

