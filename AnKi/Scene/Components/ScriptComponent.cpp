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
#include <AnKi/Scene/Components/TriggerComponent.h>

namespace anki {

ScriptComponent::ScriptComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
}

ScriptComponent::~ScriptComponent()
{
	deleteInstance(SceneMemoryPool::getSingleton(), m_environments[0]);
	deleteInstance(SceneMemoryPool::getSingleton(), m_environments[1]);
}

ScriptComponent& ScriptComponent::setScriptResourceFilename(CString fname)
{
	if(fname.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_environments[1]);
		m_environments[1] = nullptr;
		m_resource.reset(nullptr);
		return *this;
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

	return *this;
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

ScriptComponent& ScriptComponent::setScriptText(CString text)
{
	if(text.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_environments[0]);
		m_environments[0] = nullptr;
		m_text.destroy();
		return *this;
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

	return *this;
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
	if(!isValid() || info.m_paused)
	{
		return;
	}

	lua_State* lua = (m_environments[0]) ? &m_environments[0]->getLuaState() : &m_environments[1]->getLuaState();

	// Call update()
	{
		// Push function name
		lua_getglobal(lua, "update");

		if(!lua_isfunction(lua, -1))
		{
			// Not defined (lua_isnil) or defined as a non-function, pop whatever lua_getglobal pushed
			lua_pop(lua, 1);
		}
		else
		{
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
	}

	// Call onTriggerEnter
	TriggerComponent* comp = info.m_node->tryGetFirstComponentOfType<TriggerComponent>();
	if(comp)
	{
		for(SceneNode* node : comp->getSceneNodesEnter())
		{
			// Push function name
			lua_getglobal(lua, "onTriggerEnter");

			if(!lua_isfunction(lua, -1))
			{
				// Not defined (lua_isnil) or defined as a non-function, pop whatever lua_getglobal pushed
				lua_pop(lua, 1);
				break;
			}
			else
			{
				// Push args
				LuaBinder::pushVariableToTheStack(lua, node);

				// Do the call (1 argument, no result)
				if(lua_pcall(lua, 1, 0, 0) != 0)
				{
					ANKI_SCENE_LOGE("Error running ScriptComponent's \"onTriggerEnter\": %s", lua_tostring(lua, -1));
					return;
				}

				updated = true;
			}
		}
	}

	// Call onTriggerExit
	if(comp)
	{
		for(SceneNode* node : comp->getSceneNodesExit())
		{
			// Push function name
			lua_getglobal(lua, "onTriggerExit");

			if(!lua_isfunction(lua, -1))
			{
				// Not defined (lua_isnil) or defined as a non-function, pop whatever lua_getglobal pushed
				lua_pop(lua, 1);
				break;
			}
			else
			{
				// Push args
				LuaBinder::pushVariableToTheStack(lua, node);

				// Do the call (1 argument, no result)
				if(lua_pcall(lua, 1, 0, 0) != 0)
				{
					ANKI_SCENE_LOGE("Error running ScriptComponent's \"onTriggerExit\": %s", lua_tostring(lua, -1));
					return;
				}

				updated = true;
			}
		}
	}
}

Error ScriptComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);
	ANKI_SERIALIZE(m_text, 1);

	// TODO Serialize environments

	return Error::kNone;
}

} // end namespace anki
