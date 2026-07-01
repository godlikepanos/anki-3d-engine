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

ScriptComponentVariable& ScriptComponentVariable::operator=(ScriptComponentVariable&& b)
{
	m_name = std::move(b.m_name);

	if(m_type == ScriptComponentVariableType::kString)
	{
		m_string.destroy();
	}
	else
	{
		callConstructor(m_string);
	}

	switch(b.m_type)
	{
	case ScriptComponentVariableType::kString:
		m_string = std::move(b.m_string);
		break;
	case ScriptComponentVariableType::kNumber:
		m_number = b.m_number;
		break;
	case ScriptComponentVariableType::kBool:
		m_bool = b.m_bool;
		break;
	case ScriptComponentVariableType::kVec2:
		m_vec2 = b.m_vec2;
		break;
	case ScriptComponentVariableType::kVec3:
		m_vec3 = b.m_vec3;
		break;
	case ScriptComponentVariableType::kVec4:
		m_vec4 = b.m_vec4;
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	m_type = b.m_type;
	b.m_type = ScriptComponentVariableType::kCount;

	m_dirty = b.m_dirty;
	b.m_dirty = false;
	return *this;
}

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

		rebuildVarsFromLua(*m_environments[1]);
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

		rebuildVarsFromLua(*m_environments[0]);
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

	// Update vars
	Bool varsNeedUpdate = false;
	for(ScriptComponentVariable& var : m_vars)
	{
		if(var.m_dirty)
		{
			varsNeedUpdate = true;
			break;
		}
	}

	if(varsNeedUpdate)
	{
		flushDirtyVarsToLua((m_environments[0]) ? *m_environments[0] : *m_environments[1]);
	}
}

Error ScriptComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);
	ANKI_SERIALIZE(m_text, 1);

	// TODO Serialize environments

	return Error::kNone;
}

void ScriptComponent::rebuildVarsFromLua(ScriptEnvironment& env)
{
	class Callbacks : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		ScriptComponent* m_self;

		Bool onNumber(CString name, F64& value) override
		{
			ScriptComponentVariable& var = *m_self->m_vars.emplaceBack();
			var.m_type = ScriptComponentVariableType::kNumber;
			var.m_name = name;
			var.m_number = value;
			return false;
		}

		Bool onBool(CString name, Bool& value) override
		{
			ScriptComponentVariable& var = *m_self->m_vars.emplaceBack();
			var.m_type = ScriptComponentVariableType::kBool;
			var.m_name = name;
			var.m_bool = value;
			return false;
		}

		Bool onString(CString name, ScriptString& value) override
		{
			ScriptComponentVariable& var = *m_self->m_vars.emplaceBack();
			var.m_type = ScriptComponentVariableType::kString;
			var.m_name = name;
			var.m_string = value;
			return false;
		}

		Bool onUserData(CString name, LuaUserData& value) override
		{
			CString typeStr = value.getDataTypeInfo().m_typeName;
			if(typeStr == "Vec2")
			{
				ScriptComponentVariable& var = *m_self->m_vars.emplaceBack();
				var.m_type = ScriptComponentVariableType::kVec2;
				var.m_name = name;
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &var.m_vec2.x, size);
				ANKI_ASSERT(size == sizeof(Vec2));
			}
			else if(typeStr == "Vec3")
			{
				ScriptComponentVariable& var = *m_self->m_vars.emplaceBack();
				var.m_type = ScriptComponentVariableType::kVec3;
				var.m_name = name;
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &var.m_vec3.x, size);
				ANKI_ASSERT(size == sizeof(Vec3));
			}
			else if(typeStr == "Vec4")
			{
				ScriptComponentVariable& var = *m_self->m_vars.emplaceBack();
				var.m_type = ScriptComponentVariableType::kVec4;
				var.m_name = name;
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &var.m_vec4.x, size);
				ANKI_ASSERT(size == sizeof(Vec4));
			}
			else
			{
				ANKI_SCENE_LOGE("Script component can't work with user data of type: %s", typeStr.cstr());
			}

			return false;
		}
	} callbacks;
	callbacks.m_self = this;

	m_vars.destroy();
	LuaBinder::visitGlobals(&env.getLuaState(), callbacks);
}

void ScriptComponent::flushDirtyVarsToLua(ScriptEnvironment& env)
{
	class Callbacks : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		ScriptComponent* m_self;

		ScriptComponentVariable* findVar(CString name)
		{
			ScriptComponentVariable* found = nullptr;
			for(ScriptComponentVariable& var : m_self->m_vars)
			{
				if(var.m_name == name)
				{
					found = &var;
					break;
				}
			}

			if(!found)
			{
				ANKI_SCENE_LOGE(
					"Unable to find LUA var %s. Was that var created inside some function? Did you forget to use \"local\" on some temp vars?",
					name.cstr());
			}

			return found;
		}

		Bool onNumber(CString name, F64& value) override
		{
			ScriptComponentVariable* var = findVar(name);
			if(var && var->m_dirty)
			{
				value = var->m_number;
				var->m_dirty = false;
				return true;
			}
			else
			{
				return false;
			}
		}

		Bool onBool(CString name, Bool& value) override
		{
			ScriptComponentVariable* var = findVar(name);
			if(var && var->m_dirty)
			{
				value = var->m_bool;
				var->m_dirty = false;
				return true;
			}
			else
			{
				return false;
			}
		}

		Bool onString(CString name, ScriptString& value) override
		{
			ScriptComponentVariable* var = findVar(name);
			if(var && var->m_dirty)
			{
				value = var->m_string;
				var->m_dirty = false;
				return true;
			}
			else
			{
				return false;
			}
		}

		Bool onUserData(CString name, LuaUserData& value) override
		{
			CString typeStr = value.getDataTypeInfo().m_typeName;
			if(typeStr == "Vec2" || typeStr == "Vec3" || typeStr == "Vec4")
			{
				ScriptComponentVariable* var = findVar(name);
				if(var && var->m_dirty)
				{
					value.getDataTypeInfo().m_deserializeCallback(&var->m_vec4.x, value);
					var->m_dirty = false;
					return true;
				}
			}

			return false;
		}
	} callbacks;
	callbacks.m_self = this;

	LuaBinder::visitGlobals(&env.getLuaState(), callbacks);
}

} // end namespace anki
