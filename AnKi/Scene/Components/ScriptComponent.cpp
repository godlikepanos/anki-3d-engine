// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

ANKI_SCENE_COMPONENT_STATICS(ScriptComponent)

ScriptComponent::ScriptComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
	ANKI_ASSERT(node);
}

ScriptComponent::~ScriptComponent()
{
	deleteInstance(m_node->getMemoryPool(), m_env);
}

Error ScriptComponent::loadScriptResource(CString fname)
{
	// Load
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(fname, m_script));

	// Create the env
	if(m_env)
	{
		deleteInstance(m_node->getMemoryPool(), m_env);
	}
	m_env = newInstance<ScriptEnvironment>(m_node->getMemoryPool());
	ANKI_CHECK(m_env->init(&m_node->getSceneGraph().getScriptManager()));

	// Exec the script
	ANKI_CHECK(m_env->evalString(m_script->getSource()));

	return Error::kNone;
}

Error ScriptComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	ANKI_ASSERT(info.m_node == m_node);
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
