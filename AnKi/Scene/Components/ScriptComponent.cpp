// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	: SceneComponent(node, kClassType)
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

void ScriptComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = false;
	if(m_env == nullptr)
	{
		return;
	}

	lua_State* lua = &m_env->getLuaState();

	// Push function name
	lua_getglobal(lua, "update");

	// Push args
	LuaBinder::pushVariableToTheStack(lua, info.m_node);
	lua_pushnumber(lua, info.m_previousTime);
	lua_pushnumber(lua, info.m_currentTime);

	// Do the call (3 arguments, no result)
	if(lua_pcall(lua, 3, 0, 0) != 0)
	{
		ANKI_SCENE_LOGE("Error running ScriptComponent's \"update\": %s", lua_tostring(lua, -1));
		return;
	}

	updated = true;
}

} // end namespace anki
