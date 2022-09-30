// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Script/ScriptEnvironment.h>
#include <AnKi/Script/ScriptManager.h>

namespace anki {

Error ScriptEnvironment::init(ScriptManager* manager)
{
	ANKI_ASSERT(!isInitialized());
	ANKI_ASSERT(manager);
	m_manager = manager;
	return m_thread.init(&m_manager->getMemoryPool(), &m_manager->getOtherSystems());
}

} // end namespace anki
