// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/ScriptManager.h>
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

Error ScriptManager::init(AllocAlignedCallback allocCb, void* allocCbData, SceneGraph* scene, MainRenderer* renderer)
{
	ANKI_SCRIPT_LOGI("Initializing scripting engine...");

	m_scene = scene;
	m_r = renderer;
	m_alloc = ChainAllocator<U8>(allocCb, allocCbData, 1024, 1.0, 0);

	ANKI_CHECK(m_lua.create(m_alloc, this));

	// Wrap stuff
	lua_State* l = m_lua.getLuaState();

#define ANKI_SCRIPT_CALL_WRAP(x_)                                                                                      \
	extern void wrapModule##x_(lua_State*);                                                                            \
	wrapModule##x_(l);

	ANKI_SCRIPT_CALL_WRAP(Math);
	ANKI_SCRIPT_CALL_WRAP(Renderer);
	ANKI_SCRIPT_CALL_WRAP(Scene);
	ANKI_SCRIPT_CALL_WRAP(Event);

#undef ANKI_SCRIPT_CALL_WRAP

	return ErrorCode::NONE;
}

} // end namespace anki
