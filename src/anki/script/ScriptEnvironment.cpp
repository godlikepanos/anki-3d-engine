// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/ScriptEnvironment.h>
#include <anki/script/ScriptManager.h>

namespace anki
{

Error ScriptEnvironment::init(ScriptManager* manager)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(manager);
	m_manager = manager;
	return m_thread.init(m_manager->getAllocator(), &m_manager->getOtherSystems());
}

} // end namespace anki