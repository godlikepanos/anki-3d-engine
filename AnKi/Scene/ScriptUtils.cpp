// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ScriptUtils.h>
#include <AnKi/Script/ScriptEnvironment.h>

namespace anki {

void ScriptVariables::destroy()
{
	for(U32 i = 0; i < m_values.getSize(); ++i)
	{
		if(m_types[i] == ScriptVariableType::kString)
		{
			deleteInstance(SceneMemoryPool::getSingleton(), m_values[i].m_string);
		}
	}

	m_names.destroy();
	m_nameHashes.destroy();
	m_values.destroy();
	m_types.destroy();
	m_dirty.destroy();
}

void ScriptVariables::rebuildVarsFromLua(ScriptEnvironment& env)
{
	destroy();

	// Create the array
	class CountCallbacks final : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		U32 m_count = 0;

		Bool onNumber([[maybe_unused]] CString name, [[maybe_unused]] F64& value) override
		{
			++m_count;
			return false;
		}

		Bool onBool([[maybe_unused]] CString name, [[maybe_unused]] Bool& value) override
		{
			++m_count;
			return false;
		}

		Bool onString([[maybe_unused]] CString name, [[maybe_unused]] ScriptString& value) override
		{
			++m_count;
			return false;
		}

		Bool onUserData([[maybe_unused]] CString name, LuaUserData& value) override
		{
			CString typeStr = value.getDataTypeInfo().m_typeName;
			if(typeStr == "Vec2" || typeStr == "Vec3" || typeStr == "Vec4")
			{
				++m_count;
			}
			else
			{
				ANKI_SCENE_LOGE("Script can't work with user data of type: %s", typeStr.cstr());
			}

			return false;
		}
	} countCallbacks;

	LuaBinder::visitGlobals(&env.getLuaState(), countCallbacks);

	m_names.resize(countCallbacks.m_count);
	m_values.resize(countCallbacks.m_count);
	m_types.resize(countCallbacks.m_count);
	m_nameHashes.resize(countCallbacks.m_count);

	// Populate the vars
	class Callbacks final : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		ScriptVariables& m_self;
		U32 m_idx = 0;

		Callbacks(ScriptVariables& self)
			: m_self(self)
		{
		}

		Bool onNumber(CString name, F64& value) override
		{
			m_self.m_types[m_idx] = ScriptVariableType::kNumber;
			m_self.m_names[m_idx] = name;
			m_self.m_nameHashes[m_idx] = name.computeHash();
			m_self.m_values[m_idx].m_number = value;
			++m_idx;
			return false;
		}

		Bool onBool(CString name, Bool& value) override
		{
			m_self.m_types[m_idx] = ScriptVariableType::kBool;
			m_self.m_names[m_idx] = name;
			m_self.m_nameHashes[m_idx] = name.computeHash();
			m_self.m_values[m_idx].m_bool = value;
			++m_idx;
			return false;
		}

		Bool onString(CString name, ScriptString& value) override
		{
			m_self.m_types[m_idx] = ScriptVariableType::kString;
			m_self.m_names[m_idx] = name;
			m_self.m_nameHashes[m_idx] = name.computeHash();

			const U32 len = value.getLength();
			m_self.m_values[m_idx].m_string = newArray<Char>(SceneMemoryPool::getSingleton(), len + 1);
			if(len == 0)
			{
				m_self.m_values[m_idx].m_string[0] = '\0';
			}
			else
			{
				memcpy(m_self.m_values[m_idx].m_string, value.cstr(), len + 1);
			}

			++m_idx;
			return false;
		}

		Bool onUserData(CString name, LuaUserData& value) override
		{
			CString typeStr = value.getDataTypeInfo().m_typeName;
			if(typeStr == "Vec2")
			{
				m_self.m_types[m_idx] = ScriptVariableType::kVec2;
				m_self.m_names[m_idx] = name;
				m_self.m_nameHashes[m_idx] = name.computeHash();
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &m_self.m_values[m_idx].m_vec2.x, size);
				ANKI_ASSERT(size == sizeof(Vec2));
				++m_idx;
			}
			else if(typeStr == "Vec3")
			{
				m_self.m_types[m_idx] = ScriptVariableType::kVec3;
				m_self.m_names[m_idx] = name;
				m_self.m_nameHashes[m_idx] = name.computeHash();
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &m_self.m_values[m_idx].m_vec3.x, size);
				ANKI_ASSERT(size == sizeof(Vec3));
				++m_idx;
			}
			else if(typeStr == "Vec4")
			{
				m_self.m_types[m_idx] = ScriptVariableType::kVec4;
				m_self.m_names[m_idx] = name;
				m_self.m_nameHashes[m_idx] = name.computeHash();
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &m_self.m_values[m_idx].m_vec4.x, size);
				ANKI_ASSERT(size == sizeof(Vec4));
				++m_idx;
			}
			else
			{
				ANKI_SCENE_LOGE("Script can't work with user data of type: %s", typeStr.cstr());
			}

			return false;
		}
	} callbacks(*this);

	LuaBinder::visitGlobals(&env.getLuaState(), callbacks);
}

void ScriptVariables::flushDirtyVarsToLua(ScriptEnvironment& env)
{
	if(m_dirty.getSetBitCount() == 0)
	{
		return;
	}

	class Callbacks final : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		ScriptVariables& m_self;

		Callbacks(ScriptVariables& self)
			: m_self(self)
		{
		}

		Bool findVar(CString name, U32& idx)
		{
			const U64 hash = name.computeHash();

			for(idx = 0; idx < m_self.m_nameHashes.getSize(); ++idx)
			{
				if(m_self.m_nameHashes[idx] == hash)
				{
					break;
				}
			}

			const Bool found = idx < m_self.m_nameHashes.getSize();
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
			U32 idx;
			if(findVar(name, idx) && m_self.m_dirty.getBit(idx))
			{
				value = m_self.m_values[idx].m_number;
				m_self.m_dirty.unsetBit(idx);
				return true;
			}
			else
			{
				return false;
			}
		}

		Bool onBool(CString name, Bool& value) override
		{
			U32 idx;
			if(findVar(name, idx) && m_self.m_dirty.getBit(idx))
			{
				value = m_self.m_values[idx].m_bool;
				m_self.m_dirty.unsetBit(idx);
				return true;
			}
			else
			{
				return false;
			}
		}

		Bool onString(CString name, ScriptString& value) override
		{
			U32 idx;
			if(findVar(name, idx) && m_self.m_dirty.getBit(idx))
			{
				value = m_self.m_values[idx].m_string;
				m_self.m_dirty.unsetBit(idx);
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
				U32 idx;
				if(findVar(name, idx) && m_self.m_dirty.getBit(idx))
				{
					value.getDataTypeInfo().m_deserializeCallback(&m_self.m_values[idx].m_vec4.x, value);
					m_self.m_dirty.unsetBit(idx);
					return true;
				}
			}

			return false;
		}
	} callbacks(*this);

	LuaBinder::visitGlobals(&env.getLuaState(), callbacks);
}

} // end namespace anki
