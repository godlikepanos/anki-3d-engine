// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/script/ScriptManager.h"
#include "anki/util/Logger.h"

#define ANKI_SCRIPT_CALL_WRAP(x_) \
	extern void wrapModule##x_(lua_State*); \
	wrapModule##x_(l);

namespace anki {

//==============================================================================
ScriptManager::ScriptManager()
{}

//==============================================================================
ScriptManager::~ScriptManager()
{
	ANKI_LOGI("Destroying scripting engine...");
}

//==============================================================================
Error ScriptManager::create(
	AllocAlignedCallback allocCb, 
	void* allocCbData, SceneGraph* scene)
{
	ANKI_LOGI("Initializing scripting engine...");

	m_scene = scene;
	m_alloc = ChainAllocator<U8>(allocCb, allocCbData, 1024, 1024 * 1024);

	Error err = m_lua.create(m_alloc, this);
	if(err) return err;

	// Wrap stuff
	lua_State* l = m_lua.getLuaState();

	ANKI_SCRIPT_CALL_WRAP(Math);
	ANKI_SCRIPT_CALL_WRAP(Renderer);
	ANKI_SCRIPT_CALL_WRAP(Scene);

	ANKI_LOGI("Scripting engine initialized");
	return err;
}

} // end namespace anki
