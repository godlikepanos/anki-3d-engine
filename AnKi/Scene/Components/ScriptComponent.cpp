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

ScriptComponent::ScriptComponent(SceneNode* node, U32 uuid)
	: SceneComponent(node, kClassType, uuid)
{
	ANKI_ASSERT(node);
}

ScriptComponent::~ScriptComponent()
{
	deleteInstance(SceneMemoryPool::getSingleton(), m_environments[0]);
	deleteInstance(SceneMemoryPool::getSingleton(), m_environments[1]);
}

void ScriptComponent::setScriptResourceFilename(CString fname)
{
	if(fname.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_environments[1]);
		m_environments[1] = nullptr;
		m_resource.reset(nullptr);
		return;
	}

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
		err = newEnv->evalString(m_resource->getSource());
	}

	// Error
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load the script");
		deleteInstance(SceneMemoryPool::getSingleton(), newEnv);
	}
	else
	{
		m_resource = std::move(rsrc);
		deleteInstance(SceneMemoryPool::getSingleton(), m_environments[1]);
		m_environments[1] = newEnv;
	}
}

CString ScriptComponent::getScriptResourceFilename() const
{
	if(ANKI_EXPECT(m_resource.isCreated()))
	{
		return m_resource->getFilename();
	}
	else
	{
		return "*Error*";
	}
}

void ScriptComponent::setScriptText(CString text)
{
	if(text.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_environments[0]);
		m_environments[0] = nullptr;
		m_text.destroy();
		return;
	}

	// Create the env
	ScriptEnvironment* newEnv = newInstance<ScriptEnvironment>(SceneMemoryPool::getSingleton());

	// Exec the script
	const Error err = newEnv->evalString(text);

	// Error
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load the script");
		deleteInstance(SceneMemoryPool::getSingleton(), newEnv);
	}
	else
	{
		m_text = text;
		deleteInstance(SceneMemoryPool::getSingleton(), m_environments[0]);
		m_environments[0] = newEnv;
	}
}

CString ScriptComponent::getScriptText() const
{
	if(ANKI_EXPECT(m_text))
	{
		return m_text;
	}
	else
	{
		return "*Error*";
	}
}

void ScriptComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = false;
	if((!m_environments[0] && !m_environments[1]) || info.m_paused)
	{
		return;
	}

	lua_State* lua = (m_environments[0]) ? &m_environments[0]->getLuaState() : &m_environments[1]->getLuaState();

	// Push function name
	lua_getglobal(lua, "update");

	// Push args
	LuaBinder::pushVariableToTheStack(lua, &info);

	// Do the call (1 argument, no result)
	if(lua_pcall(lua, 1, 0, 0) != 0)
	{
		ANKI_SCENE_LOGE("Error running ScriptComponent's \"update\": %s", lua_tostring(lua, -1));
		return;
	}

	updated = true;
}

Error ScriptComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);
	ANKI_SERIALIZE(m_text, 1);

	// TODO Serialize environments

	return Error::kNone;
}

} // end namespace anki
