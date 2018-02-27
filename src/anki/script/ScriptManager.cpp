// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/ScriptManager.h>
#include <anki/script/ScriptEnvironment.h>
#include <anki/util/Logger.h>

namespace anki
{

// Forward
#define ANKI_SCRIPT_CALL_WRAP(x_) void wrapModule##x_(lua_State*)
ANKI_SCRIPT_CALL_WRAP(Logger);
ANKI_SCRIPT_CALL_WRAP(Math);
ANKI_SCRIPT_CALL_WRAP(Renderer);
ANKI_SCRIPT_CALL_WRAP(Scene);
ANKI_SCRIPT_CALL_WRAP(Event);
#undef ANKI_SCRIPT_CALL_WRAP

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

	// Wrap stuff
	lua_State* l = m_lua.getLuaState();

#define ANKI_SCRIPT_CALL_WRAP(x_) wrapModule##x_(l)
	ANKI_SCRIPT_CALL_WRAP(Logger);
	ANKI_SCRIPT_CALL_WRAP(Math);
	ANKI_SCRIPT_CALL_WRAP(Renderer);
	ANKI_SCRIPT_CALL_WRAP(Scene);
	ANKI_SCRIPT_CALL_WRAP(Event);
#undef ANKI_SCRIPT_CALL_WRAP

	return Error::NONE;
}

Error ScriptManager::newScriptEnvironment(ScriptEnvironmentPtr& out)
{
	out.reset(m_alloc.newInstance<ScriptEnvironment>(this));
	return out->init();
}

} // end namespace anki
