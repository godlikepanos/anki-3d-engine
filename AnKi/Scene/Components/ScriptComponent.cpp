// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ScriptComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/Components/AnimationComponent.h>

namespace anki {

ScriptComponent::ScriptComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
#if ANKI_WITH_EDITOR
	m_node = init.m_node;
#endif
}

ScriptComponent::~ScriptComponent()
{
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
	ScriptEnvironmentPtr env;
	if(fname && !err)
	{
		env = ScriptManager::getSingleton().newScriptEnvironment();
		err = env->evalString(rsrc->getSource());
	}

	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load the script resource");
	}
	else if(!fname)
	{
		m_text.destroy();
		m_resource.reset(nullptr);
		m_env.reset(nullptr);
		m_vars.destroy();
		m_definedFuncsMask = ScriptComponentFunctionMask::kNone;
	}
	else
	{
		m_text.destroy();
		m_resource = std::move(rsrc);
		m_env.reset(nullptr);
		m_env = std::move(env);
		m_vars.rebuildVarsFromLua(*m_env);
		rebuildDefinedFunctionsMask();
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
	ScriptEnvironmentPtr env;
	Error err = Error::kNone;
	if(text)
	{
		env = ScriptManager::getSingleton().newScriptEnvironment();
		err = env->evalString(text);
	}

	if(err)
	{
		ANKI_SCENE_LOGE("Failed to init the script");
	}
	else if(!text)
	{
		m_text.destroy();
		m_resource.reset(nullptr);
		m_env.reset(nullptr);
		m_vars.destroy();
		m_definedFuncsMask = ScriptComponentFunctionMask::kNone;
	}
	else
	{
		m_resource.reset(nullptr);
		m_text = text;
		m_env.reset(nullptr);
		m_env = std::move(env);
		m_vars.rebuildVarsFromLua(*m_env);
		rebuildDefinedFunctionsMask();
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
#if ANKI_WITH_EDITOR
	if(!isValid() || (info.m_paused && !m_playOnEditor))
#else
	if(!isValid() || info.m_paused)
#endif
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
	if(!!(m_definedFuncsMask & ScriptComponentFunctionMask::kUpdate))
	{
		// Push function name
		lua_getglobal(lua, "update");

		// Push args
		LuaBinder::pushVariableToTheStack(lua, &info);

		// Do the call (1 argument, no result)
		if(lua_pcall(lua, 1, 0, 0) != 0)
		{
			ANKI_SCENE_LOGE("Error running ScriptComponent's \"update\": {}", lua_tostring(lua, -1));
			m_definedFuncsMask &= ~ScriptComponentFunctionMask::kUpdate;
		}
		else
		{
			updated = true;
		}
	}

	auto callLuaFunction = [&](auto pair, ScriptComponentFunctionMask mask) {
		if(!(m_definedFuncsMask & mask))
		{
			// A previous call failed
			return true;
		}

		const CString funcName = (mask == ScriptComponentFunctionMask::kOnAnimationStarted) ? "onAnimationStarted" : "onAnimationEnded";

		// Push function name
		lua_getglobal(lua, funcName.cstr());

		// Push args
		LuaBinder::pushVariableToTheStack(lua, info.m_node);
		lua_pushnumber(lua, pair.first);
		lua_pushnumber(lua, pair.second);

		// Do the call (3 arguments, no result)
		if(lua_pcall(lua, 3, 0, 0) != 0)
		{
			ANKI_SCENE_LOGE("Error running ScriptComponent's \"{}\": {}", funcName, lua_tostring(lua, -1));
			m_definedFuncsMask &= ~mask;
			return true;
		}
		else
		{
			updated = true;
			return false;
		}
	};

	// Call onAnimationStarted
	if(!!(m_definedFuncsMask & ScriptComponentFunctionMask::kOnAnimationStarted))
	{
		SkinComponent* skinc = info.m_node->tryGetFirstComponentOfType<SkinComponent>();
		if(skinc)
		{
			for(auto pair : skinc->getAnimationsStarted())
			{
				if(callLuaFunction(pair, ScriptComponentFunctionMask::kOnAnimationStarted))
				{
					break;
				}
			}
		}

		info.m_node->iterateComponentsOfType<AnimationComponent>([&](const AnimationComponent& animc) {
			for(auto pair : animc.getAnimationsStarted())
			{
				if(callLuaFunction(pair, ScriptComponentFunctionMask::kOnAnimationStarted))
				{
					return FunctorContinue::kStop;
				}
			}

			return FunctorContinue::kContinue;
		});
	}

	// Call onAnimationEnded
	if(!!(m_definedFuncsMask & ScriptComponentFunctionMask::kOnAnimationEnded))
	{
		SkinComponent* skinc = info.m_node->tryGetFirstComponentOfType<SkinComponent>();
		if(skinc)
		{
			for(auto pair : skinc->getAnimationsEnded())
			{
				if(callLuaFunction(pair, ScriptComponentFunctionMask::kOnAnimationEnded))
				{
					break;
				}
			}
		}

		info.m_node->iterateComponentsOfType<AnimationComponent>([&](const AnimationComponent& animc) {
			for(auto pair : animc.getAnimationsEnded())
			{
				if(callLuaFunction(pair, ScriptComponentFunctionMask::kOnAnimationEnded))
				{
					return FunctorContinue::kStop;
				}
			}

			return FunctorContinue::kContinue;
		});
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

void ScriptComponent::rebuildDefinedFunctionsMask()
{
	auto funcExists = [this](CString funcName) {
		lua_getglobal(&m_env->getLuaState(), funcName.cstr());
		const Bool exists = lua_isfunction(&m_env->getLuaState(), -1);
		lua_pop(&m_env->getLuaState(), 1);
		return exists;
	};

	m_definedFuncsMask = ScriptComponentFunctionMask::kNone;
	if(funcExists("update"))
	{
		m_definedFuncsMask |= ScriptComponentFunctionMask::kUpdate;
	}

	if(funcExists("onAnimationStarted"))
	{
		m_definedFuncsMask |= ScriptComponentFunctionMask::kOnAnimationStarted;
	}

	if(funcExists("onAnimationEnded"))
	{
		m_definedFuncsMask |= ScriptComponentFunctionMask::kOnAnimationEnded;
	}

	if(funcExists("onTriggerEnter"))
	{
		m_definedFuncsMask |= ScriptComponentFunctionMask::kOnTriggerEnter;
	}

	if(funcExists("onTriggerExit"))
	{
		m_definedFuncsMask |= ScriptComponentFunctionMask::kOnTriggerExit;
	}
}

void ScriptComponent::callOnTrigger(Bool onEnter, SceneNode& selfNode, SceneNode& otherNode)
{
	const ScriptComponentFunctionMask mask = (onEnter) ? ScriptComponentFunctionMask::kOnTriggerEnter : ScriptComponentFunctionMask::kOnTriggerExit;
	if(isValid() && !!(m_definedFuncsMask & mask))
	{
		m_vars.flushDirtyVarsToLua(*m_env);

		lua_State* lua = &m_env->getLuaState();
		CString funcName = (onEnter) ? "onTriggerEnter" : "onTriggerExit";

		// Push function name
		lua_getglobal(lua, funcName.cstr());

		// Push args
		LuaBinder::pushVariableToTheStack(lua, &selfNode);
		LuaBinder::pushVariableToTheStack(lua, &otherNode);

		// Do the call (2 argument, no result)
		if(lua_pcall(lua, 2, 0, 0) != 0)
		{
			ANKI_SCENE_LOGE("Error running ScriptComponent's \"{}\": {}", funcName, lua_tostring(lua, -1));
			m_definedFuncsMask &= ~mask;
		}
		else
		{
			m_vars.updateVarsFromLua(*m_env);
		}
	}
}

} // end namespace anki
