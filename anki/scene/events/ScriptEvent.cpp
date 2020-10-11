// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/events/ScriptEvent.h>
#include <anki/util/Filesystem.h>
#include <anki/resource/ScriptResource.h>
#include <anki/script/ScriptEnvironment.h>
#include <anki/script/ScriptManager.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

ScriptEvent::ScriptEvent(EventManager* manager)
	: Event(manager)
{
}

ScriptEvent::~ScriptEvent()
{
	m_script.destroy(getAllocator());
}

Error ScriptEvent::init(Second startTime, Second duration, CString script)
{
	Event::init(startTime, duration);

	// Create the env
	ANKI_CHECK(m_env.init(&getSceneGraph().getScriptManager()));

	// Do the rest
	StringAuto extension(getAllocator());
	getFilepathExtension(script, extension);

	if(!extension.isEmpty() && extension == "lua")
	{
		// It's a file
		ANKI_CHECK(getSceneGraph().getResourceManager().loadResource(script, m_scriptRsrc));

		// Exec the script
		ANKI_CHECK(m_env.evalString(m_scriptRsrc->getSource()));
	}
	else
	{
		// It's a string
		m_script.create(getAllocator(), script);

		// Exec the script
		ANKI_CHECK(m_env.evalString(m_script.toCString()));
	}

	return Error::NONE;
}

Error ScriptEvent::update(Second prevUpdateTime, Second crntTime)
{
	lua_State* lua = &m_env.getLuaState();

	// Push function name
	lua_getglobal(lua, "update");

	// Push args
	LuaBinder::pushVariableToTheStack(lua, static_cast<Event*>(this));
	lua_pushnumber(lua, prevUpdateTime);
	lua_pushnumber(lua, crntTime);

	// Do the call (3 arguments, 1 result)
	if(lua_pcall(lua, 3, 1, 0) != 0)
	{
		ANKI_SCENE_LOGE("Error running ScriptEvent's \"update\": %s", lua_tostring(lua, -1));
		return Error::USER_DATA;
	}

	if(!lua_isnumber(lua, -1))
	{
		ANKI_SCENE_LOGE("ScriptEvent's \"update\" should return a number");
		lua_pop(lua, 1);
		return Error::USER_DATA;
	}

	// Get the result
	lua_Number result = lua_tonumber(lua, -1);
	lua_pop(lua, 1);

	if(result < 0)
	{
		ANKI_SCENE_LOGE("ScriptEvent's \"update\" return an error code");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error ScriptEvent::onKilled(Second prevUpdateTime, Second crntTime)
{
	lua_State* lua = &m_env.getLuaState();

	// Push function name
	lua_getglobal(lua, "onKilled");

	// Push args
	LuaBinder::pushVariableToTheStack(lua, static_cast<Event*>(this));
	lua_pushnumber(lua, prevUpdateTime);
	lua_pushnumber(lua, crntTime);

	// Do the call (3 arguments, 1 result)
	if(lua_pcall(lua, 3, 1, 0) != 0)
	{
		ANKI_SCENE_LOGE("Error running ScriptEvent's \"onKilled\": %s", lua_tostring(lua, -1));
		return Error::USER_DATA;
	}

	if(!lua_isnumber(lua, -1))
	{
		ANKI_SCENE_LOGE("ScriptEvent's \"onKilled\" should return a number");
		lua_pop(lua, 1);
		return Error::USER_DATA;
	}

	// Get the result
	lua_Number result = lua_tonumber(lua, -1);
	lua_pop(lua, 1);

	if(result < 0)
	{
		ANKI_SCENE_LOGE("ScriptEvent's \"onKilled\" return an error code");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

} // end namespace anki