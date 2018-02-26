// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/ScriptEnvironment.h>
#include <anki/script/ScriptManager.h>

namespace anki
{

ScriptEnvironment::~ScriptEnvironment()
{
	m_manager->destroyLuaThread(m_thread);
}

Error ScriptEnvironment::init()
{
	m_thread = m_manager->newLuaThread();
	return Error::NONE;
}

} // end namespace anki