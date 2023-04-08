// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ScriptComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Script/ScriptEnvironment.h>

namespace anki {

ScriptComponent::ScriptComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
	ANKI_ASSERT(node);
}

ScriptComponent::~ScriptComponent()
{
	deleteInstance(SceneMemoryPool::getSingleton(), m_env);
}

void ScriptComponent::loadScriptResource(CString fname)
{
	// Load
	ScriptResourcePtr rsrc;
	Error err = ResourceManager::getSingleton().loadResource(fname, rsrc);

	// Create the env
	ScriptEnvironment* newEnv = nullptr;
	if(!err)
	{
		newEnv = newInstance<ScriptEnvironment>(SceneMemoryPool::getSingleton());
	}

	// Exec the script
	if(!err)
	{
		err = newEnv->evalString(m_script->getSource());
	}

	// Error
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load the script");
		deleteInstance(SceneMemoryPool::getSingleton(), newEnv);
	}
	else
	{
		m_script = std::move(rsrc);
		deleteInstance(SceneMemoryPool::getSingleton(), m_env);
		m_env = newEnv;
	}
}

Error ScriptComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = false;
	if(m_env == nullptr)
	{
		return Error::kNone;
	}

	lua_State* lua = &m_env->getLuaState();

	// Push function name
	lua_getglobal(lua, "update");

	// Push args
	LuaBinder::pushVariableToTheStack(lua, info.m_node);
	lua_pushnumber(lua, info.m_previousTime);
	lua_pushnumber(lua, info.m_currentTime);

	// Do the call (3 arguments, 1 result)
	if(lua_pcall(lua, 3, 1, 0) != 0)
	{
		ANKI_SCENE_LOGE("Error running ScriptComponent's \"update\": %s", lua_tostring(lua, -1));
		return Error::kUserData;
	}

	if(!lua_isnumber(lua, -1))
	{
		ANKI_SCENE_LOGE("ScriptComponent's \"update\" should return a number");
		lua_pop(lua, 1);
		return Error::kUserData;
	}

	// Get the result
	lua_Number result = lua_tonumber(lua, -1);
	lua_pop(lua, 1);

	if(result < 0)
	{
		ANKI_SCENE_LOGE("ScriptComponent's \"update\" return an error code");
		return Error::kUserData;
	}

	updated = (result != 0);

	return Error::kNone;
}

} // end namespace anki
