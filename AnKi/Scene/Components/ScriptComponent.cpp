// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ScriptComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Script/ScriptEnvironment.h>
#include <AnKi/Scene/Components/TriggerComponent.h>

namespace anki {

ScriptComponent::ScriptComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
}

ScriptComponent::~ScriptComponent()
{
	deleteInstance(SceneMemoryPool::getSingleton(), m_env);
}

ScriptComponent& ScriptComponent::setScriptResourceFilename(CString fname)
{
	// Load resource
	ScriptResourcePtr rsrc;
	Error err = Error::kNone;
	if(fname)
	{
		err = ResourceManager::getSingleton().loadResource(fname, rsrc);
	}

	// Init the env
	ScriptEnvironment* env = nullptr;
	if(fname && !err)
	{
		env = newInstance<ScriptEnvironment>(SceneMemoryPool::getSingleton());
		err = env->evalString(rsrc->getSource());
	}

	if(err)
	{
		deleteInstance(SceneMemoryPool::getSingleton(), env);
		ANKI_SCENE_LOGE("Failed to load the script resource");
	}
	else if(!fname)
	{
		m_text.destroy();
		m_resource.reset(nullptr);
		deleteInstance(SceneMemoryPool::getSingleton(), m_env);
		m_env = nullptr;
		m_vars.destroy();
	}
	else
	{
		m_text.destroy();
		m_resource = std::move(rsrc);
		deleteInstance(SceneMemoryPool::getSingleton(), m_env);
		m_env = env;
		m_vars.rebuildVarsFromLua(*m_env);
	}

#if ANKI_WITH_EDITOR
	m_playOnEditor = false;
#endif

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
	// Init the env
	ScriptEnvironment* env = nullptr;
	Error err = Error::kNone;
	if(text)
	{
		env = newInstance<ScriptEnvironment>(SceneMemoryPool::getSingleton());
		err = env->evalString(text);
	}

	if(err)
	{
		deleteInstance(SceneMemoryPool::getSingleton(), env);
		ANKI_SCENE_LOGE("Failed to init the script");
	}
	else if(!text)
	{
		m_text.destroy();
		m_resource.reset(nullptr);
		deleteInstance(SceneMemoryPool::getSingleton(), m_env);
		m_env = nullptr;
		m_vars.destroy();
	}
	else
	{
		m_resource.reset(nullptr);
		m_text = text;
		deleteInstance(SceneMemoryPool::getSingleton(), m_env);
		m_env = env;
		m_vars.rebuildVarsFromLua(*m_env);
	}

#if ANKI_WITH_EDITOR
	m_playOnEditor = false;
#endif

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
	if(!isValid() || info.m_paused
#if ANKI_WITH_EDITOR
	   || !m_playOnEditor
#endif
	)
	{
		return;
	}

#if ANKI_WITH_EDITOR
	if(info.m_checkForResourceUpdates && !!m_resource && m_resource->isObsolete()) [[unlikely]]
	{
		ANKI_SCENE_LOGV("Script resource is obsolete. Will reload it");
		BaseString<MemoryPoolPtrWrapper<StackMemoryPool>> fname(info.m_framePool);
		fname = m_resource->getFilename();
		setScriptResourceFilename(fname);
	}
#endif

	lua_State* lua = &m_env->getLuaState();

	// Flush C++ to LUA env
	m_vars.flushDirtyVarsToLua(*m_env);

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

	// Update C++ from LUA env if LUA run
	if(updated)
	{
		m_vars.updateVarsFromLua(*m_env);
	}
}

Error ScriptComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);
	ANKI_SERIALIZE(m_text, 1);

	if(serializer.isInReadMode())
	{
		ANKI_ASSERT(!(m_resource && m_text) && "The 2 script sources are mutually exclusive");

		// A component without any script source is valid, only complain if a source is set but the env failed to init
		const Bool hasSource = !!m_resource || !!m_text;

		if(m_resource)
		{
			const SceneString fname = m_resource->getFilename(); // Use a temp to avoid aliasing vars
			setScriptResourceFilename(fname);
		}
		else if(m_text)
		{
			const SceneString text = m_text; // Use a temp to avoid aliasing vars
			setScriptText(text);
		}

		if(hasSource && !m_env)
		{
			ANKI_SCENE_LOGE("Failed to initialize the script environment of the ScriptComponent");
			return Error::kUserData;
		}
	}

	if(m_env)
	{
		ANKI_CHECK(m_vars.serialize(serializer, *m_env));
	}

	return Error::kNone;
}

} // end namespace anki
