// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/Common.h>
#include <anki/util/Ptr.h>

namespace anki {

/// @addtogroup ui
/// @{

/// The base of all UI objects.
class UiObject
{
public:
	UiObject(Canvas* canvas)
		: m_canvas(canvas)
	{
		ANKI_ASSERT(canvas);
	}

	virtual ~UiObject() = default;

	UiAllocator getAllocator() const;

	Canvas& getCanvas()
	{
		return *m_canvas;
	}

	const Canvas& getCanvas() const
	{
		return *m_canvas;
	}

private:
	WeakPtr<Canvas> m_canvas;
};
/// @}

} // end namespace anki

