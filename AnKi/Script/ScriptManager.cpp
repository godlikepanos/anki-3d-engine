// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Script/ScriptEnvironment.h>
#include <AnKi/Util/Logger.h>

namespace anki {

ScriptManager::ScriptManager()
{
}

ScriptManager::~ScriptManager()
{
	ANKI_SCRIPT_LOGI("Destroying scripting engine...");
}

Error ScriptManager::init(AllocAlignedCallback allocCb, void* allocCbData)
{
	ANKI_SCRIPT_LOGI("Initializing scripting engine...");

	m_pool.init(allocCb, allocCbData);

	ANKI_CHECK(m_lua.init(&m_pool, &m_otherSystems));

	return Error::kNone;
}

} // end namespace anki
