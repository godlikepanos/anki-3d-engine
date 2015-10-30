// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/UiObject.h>
#include <anki/ui/Canvas.h>

namespace anki {

//==============================================================================
UiAllocator UiObject::getAllocator() const
{
	return m_canvas->getAllocator();
}

} // end namespace anki
