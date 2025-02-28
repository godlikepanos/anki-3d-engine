// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/ScriptEvent.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Script/ScriptEnvironment.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

ScriptEvent::ScriptEvent(Second startTime, Second duration, CString script)
	: Event(startTime, duration)
{
	// Do the rest
	String extension;
	getFilepathExtension(script, extension);

	if(!extension.isEmpty() && extension == "lua")
	{
		// It's a file
		if(!ANKI_EXPECT(!ResourceManager::getSingleton().loadResource(script, m_scriptRsrc)))
		{
			markForDeletion();
			return;
		}

		// Exec the script
		if(!ANKI_EXPECT(!m_env.evalString(m_scriptRsrc->getSource())))
		{
			markForDeletion();
			return;
		}
	}
	else
	{
		// It's a string
		m_script = script;

		// Exec the script
		if(!ANKI_EXPECT(!m_env.evalString(m_script.toCString())))
		{
			markForDeletion();
			return;
		}
	}
}

ScriptEvent::~ScriptEvent()
{
}

void ScriptEvent::update(Second prevUpdateTime, Second crntTime)
{
	lua_State* lua = &m_env.getLuaState();

	// Push function name
	lua_getglobal(lua, "update");

	// Push args
	LuaBinder::pushVariableToTheStack(lua, static_cast<Event*>(this));
	lua_pushnumber(lua, prevUpdateTime);
	lua_pushnumber(lua, crntTime);

	// Do the call (3 arguments, no result)
	if(lua_pcall(lua, 3, 0, 0) != 0)
	{
		ANKI_SCENE_LOGE("Error running ScriptEvent's \"update\": %s", lua_tostring(lua, -1));
		return;
	}
}

void ScriptEvent::onKilled(Second prevUpdateTime, Second crntTime)
{
	lua_State* lua = &m_env.getLuaState();

	// Push function name
	lua_getglobal(lua, "onKilled");

	// Push args
	LuaBinder::pushVariableToTheStack(lua, static_cast<Event*>(this));
	lua_pushnumber(lua, prevUpdateTime);
	lua_pushnumber(lua, crntTime);

	// Do the call (3 arguments, no result)
	if(lua_pcall(lua, 3, 0, 0) != 0)
	{
		ANKI_SCENE_LOGE("Error running ScriptEvent's \"onKilled\": %s", lua_tostring(lua, -1));
		return;
	}
}

} // end namespace anki
