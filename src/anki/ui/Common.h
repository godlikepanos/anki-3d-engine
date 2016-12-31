// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>
#include <anki/util/Allocator.h>

namespace anki
{

// Forward
class Widget;
class Canvas;
class UiImage;

/// @addtogroup ui
/// @{

using UiAllocator = GenericMemoryPoolAllocator<U8>;

/// Color.
using Color = Vec4;

/// Rectangle
class Rect
{
public:
	UVec2 m_min;
	UVec2 m_max;
};

/// Used in widget classes.
#define ANKI_WIDGET friend class Canvas;
/// @}

} // end namespace anki
