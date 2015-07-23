// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/Math.h"
#include "anki/util/Allocator.h"
#include "anki/util/Ptr.h"

namespace anki {

// Forward
class Painter;
class Image;

/// @addtogroup ui
/// @{

// Pointers
using ImagePtr = IntrusivePtr<Image>;

using UiAllocator = GenericMemoryPoolAllocator<U8>;

/// Color.
using Color = Vec4;

/// Rectangle.
class Rect
{
public:
	Vec2 m_min = {0.0};
	Vec2 m_max = {getEpsilon<F32>()};

	Rect() = default;

	Rect(Vec2 min, Vec2 max)
		: m_min(min)
		, m_max(max)
	{
		ANKI_ASSERT(m_min < m_max);
	}

	Rect(const Rect&) = default;

	Rect& operator=(const Rect&) = default;
};
/// @}

} // end namespace anki

