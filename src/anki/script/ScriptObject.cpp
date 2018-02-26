// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/ScriptObject.h>
#include <anki/script/ScriptManager.h>

namespace anki
{

ScriptAllocator ScriptObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki