// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/UiObject.h>
#include <AnKi/Ui/UiManager.h>

namespace anki
{

UiAllocator UiObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki
