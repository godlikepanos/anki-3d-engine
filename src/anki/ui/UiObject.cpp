// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/UiObject.h>
#include <anki/ui/UiManager.h>

namespace anki
{

UiAllocator UiObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki
