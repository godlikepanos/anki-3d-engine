// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/ui/Common.h"

namespace anki {

/// @addtogroup ui
/// @{

/// The base of all UI objects.
class UiObject
{
public:
	UiObject(Canvas* canvas)
		: m_canvas(canvas)
	{}

	virtual ~UiObject() = default;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	UiAllocator getAllocator() const;

private:
	Atomic<I32> m_refcount = {0};
	Canvas* m_canvas = nullptr;
};
/// @}

} // end namespace anki

