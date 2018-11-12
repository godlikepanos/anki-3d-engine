// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/ScriptManager.h>
#include <anki/script/ScriptEnvironment.h>
#include <anki/util/Logger.h>

namespace anki
{

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

	m_alloc = ScriptAllocator(allocCb, allocCbData);

	ANKI_CHECK(m_lua.create(m_alloc, this));

	return Error::NONE;
}

Error ScriptManager::newScriptEnvironment(ScriptEnvironmentPtr& out)
{
	out.reset(m_alloc.newInstance<ScriptEnvironment>(this));
	return out->init();
}

} // end namespace anki
