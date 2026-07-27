// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ScriptUtils.h>
#include <AnKi/Scene/SceneSerializer.h>
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

Bool ScriptVariables::findVar(CString name, ScriptVariableType expectedType, U32& idx) const
{
	const U64 hash = name.computeHash();

	for(idx = 0; idx < m_nameHashes.getSize(); ++idx)
	{
		if(m_nameHashes[idx] == hash)
		{
			break;
		}
	}

	Bool found = idx < m_nameHashes.getSize();
	if(!found)
	{
		ANKI_SCENE_LOGE("Unable to find LUA var: %s. Was that var created inside some function? Did you forget to use \"local\" on some temp vars?",
						name.cstr());
	}

	if(found && expectedType != m_types[idx])
	{
		ANKI_SCENE_LOGE("Variable type mismatch: %s", name.cstr());
		found = false;
	}

	return found;
}

void ScriptVariables::rebuildVarsFromLua(ScriptEnvironment& env)
{
	destroy();

	// Create the array. Also gather and sort the var names. Want them sorted now to avoid sorting the multiple arrays later which will be a pain
	class CountCallbacks final : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		U32 m_count = 0;
		SceneDynamicArray<SceneString> m_names;

		Bool onNumber(CString name, [[maybe_unused]] F64& value) override
		{
			++m_count;
			m_names.emplaceBack(name);
			return false;
		}

		Bool onBool(CString name, [[maybe_unused]] Bool& value) override
		{
			++m_count;
			m_names.emplaceBack(name);
			return false;
		}

		Bool onString(CString name, [[maybe_unused]] ScriptString& value) override
		{
			++m_count;
			m_names.emplaceBack(name);
			return false;
		}

		Bool onUserData(CString name, LuaUserData& value) override
		{
			CString typeStr = value.getDataTypeInfo().m_typeName;
			if(typeStr == "Vec2" || typeStr == "Vec3" || typeStr == "Vec4")
			{
				++m_count;
				m_names.emplaceBack(name);
			}
			else
			{
				ANKI_SCENE_LOGE("Script can't work with user data of type. Will ignore: %s", name.cstr());
			}

			return false;
		}
	} countCallbacks;

	LuaBinder::visitGlobals(&env.getLuaState(), countCallbacks);

	std::sort(countCallbacks.m_names.getBegin(), countCallbacks.m_names.getEnd());

	m_names.resize(countCallbacks.m_count);
	m_values.resize(countCallbacks.m_count);
	m_types.resize(countCallbacks.m_count);
	m_nameHashes.resize(countCallbacks.m_count);

	// Populate the vars
	class Callbacks final : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		ScriptVariables& m_self;
		SceneDynamicArray<SceneString>& m_names;

		Callbacks(ScriptVariables& self, SceneDynamicArray<SceneString>& names)
			: m_self(self)
			, m_names(names)
		{
		}

		U32 nameToIndex(CString name) const
		{
			for(U32 i = 0; i < m_names.getSize(); ++i)
			{
				if(m_names[i] == name)
				{
					return i;
				}
			}

			ANKI_ASSERT(0);
			return kMaxU32;
		}

		Bool onNumber(CString name, F64& value) override
		{
			const U32 idx = nameToIndex(name);
			m_self.m_types[idx] = ScriptVariableType::kNumber;
			m_self.m_names[idx] = name;
			m_self.m_nameHashes[idx] = name.computeHash();
			m_self.m_values[idx].m_number = value;
			return false;
		}

		Bool onBool(CString name, Bool& value) override
		{
			const U32 idx = nameToIndex(name);
			m_self.m_types[idx] = ScriptVariableType::kBool;
			m_self.m_names[idx] = name;
			m_self.m_nameHashes[idx] = name.computeHash();
			m_self.m_values[idx].m_bool = value;
			return false;
		}

		Bool onString(CString name, ScriptString& value) override
		{
			const U32 idx = nameToIndex(name);
			m_self.m_types[idx] = ScriptVariableType::kString;
			m_self.m_names[idx] = name;
			m_self.m_nameHashes[idx] = name.computeHash();

			const U32 len = value.getLength();
			m_self.m_values[idx].m_string = newArray<Char>(SceneMemoryPool::getSingleton(), len + 1);
			if(len == 0)
			{
				m_self.m_values[idx].m_string[0] = '\0';
			}
			else
			{
				memcpy(m_self.m_values[idx].m_string, value.cstr(), len + 1);
			}

			return false;
		}

		Bool onUserData(CString name, LuaUserData& value) override
		{
			CString typeStr = value.getDataTypeInfo().m_typeName;
			if(typeStr == "Vec2")
			{
				const U32 idx = nameToIndex(name);
				m_self.m_types[idx] = ScriptVariableType::kVec2;
				m_self.m_names[idx] = name;
				m_self.m_nameHashes[idx] = name.computeHash();
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &m_self.m_values[idx].m_vec2.x, size);
				ANKI_ASSERT(size == sizeof(Vec2));
			}
			else if(typeStr == "Vec3")
			{
				const U32 idx = nameToIndex(name);
				m_self.m_types[idx] = ScriptVariableType::kVec3;
				m_self.m_names[idx] = name;
				m_self.m_nameHashes[idx] = name.computeHash();
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &m_self.m_values[idx].m_vec3.x, size);
				ANKI_ASSERT(size == sizeof(Vec3));
			}
			else if(typeStr == "Vec4")
			{
				const U32 idx = nameToIndex(name);
				m_self.m_types[idx] = ScriptVariableType::kVec4;
				m_self.m_names[idx] = name;
				m_self.m_nameHashes[idx] = name.computeHash();
				PtrSize size;
				value.getDataTypeInfo().m_serializeCallback(value, &m_self.m_values[idx].m_vec4.x, size);
				ANKI_ASSERT(size == sizeof(Vec4));
			}
			else
			{
				// Ignore, error was shown above
			}

			return false;
		}
	} callbacks(*this, countCallbacks.m_names);

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

		Bool onNumber(CString name, F64& value) override
		{
			U32 idx;
			if(m_self.findVar(name, ScriptVariableType::kNumber, idx) && m_self.m_dirty.getBit(idx))
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
			if(m_self.findVar(name, ScriptVariableType::kBool, idx) && m_self.m_dirty.getBit(idx))
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
			if(m_self.findVar(name, ScriptVariableType::kString, idx) && m_self.m_dirty.getBit(idx))
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

			ScriptVariableType type = ScriptVariableType::kCount;
			if(typeStr == "Vec2")
			{
				type = ScriptVariableType::kVec2;
			}
			else if(typeStr == "Vec3")
			{
				type = ScriptVariableType::kVec3;
			}
			else if(typeStr == "Vec4")
			{
				type = ScriptVariableType::kVec4;
			}

			if(type != ScriptVariableType::kCount)
			{
				U32 idx;
				if(m_self.findVar(name, type, idx) && m_self.m_dirty.getBit(idx))
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

void ScriptVariables::updateVarsFromLua(ScriptEnvironment& env)
{
	class Callbacks final : public LuaBinderVisitGlobalsCallbacks
	{
	public:
		ScriptVariables& m_self;

		Callbacks(ScriptVariables& self)
			: m_self(self)
		{
		}

		Bool onNumber(CString name, F64& value) override
		{
			U32 idx;
			if(m_self.findVar(name, ScriptVariableType::kNumber, idx))
			{
				m_self.m_values[idx].m_number = value;
				m_self.m_dirty.unsetBit(idx);
			}

			return false;
		}

		Bool onBool(CString name, Bool& value) override
		{
			U32 idx;
			if(m_self.findVar(name, ScriptVariableType::kBool, idx))
			{
				m_self.m_values[idx].m_bool = value;
				m_self.m_dirty.unsetBit(idx);
			}

			return false;
		}

		Bool onString(CString name, ScriptString& value) override
		{
			U32 idx;
			if(m_self.findVar(name, ScriptVariableType::kString, idx))
			{
				ScriptVariable var;
				var.m_allVars = &m_self;
				var.m_idx = idx;

				var.setString(value);

				m_self.m_dirty.unsetBit(idx);
			}

			return false;
		}

		Bool onUserData(CString name, LuaUserData& value) override
		{
			CString typeStr = value.getDataTypeInfo().m_typeName;

			ScriptVariableType type = ScriptVariableType::kCount;
			if(typeStr == "Vec2")
			{
				type = ScriptVariableType::kVec2;
			}
			else if(typeStr == "Vec3")
			{
				type = ScriptVariableType::kVec3;
			}
			else if(typeStr == "Vec4")
			{
				type = ScriptVariableType::kVec4;
			}

			if(type != ScriptVariableType::kCount)
			{
				U32 idx;
				if(m_self.findVar(name, type, idx))
				{
					PtrSize size;
					value.getDataTypeInfo().m_serializeCallback(value, &m_self.m_values[idx].m_vec4.x, size);
					m_self.m_dirty.unsetBit(idx);
				}
			}

			return false;
		}
	} callbacks(*this);

	LuaBinder::visitGlobals(&env.getLuaState(), callbacks);
}

Error ScriptVariables::serialize(SceneSerializer& serializer, ScriptEnvironment& env)
{
	ANKI_ASSERT(m_names.getSize() == m_values.getSize() && m_types.getSize() == m_values.getSize() && m_nameHashes.getSize() == m_values.getSize());

	// Update vars before writes to have their latest values
	if(serializer.isInWriteMode())
	{
		updateVarsFromLua(env);
	}

	// Count only for checks
	U32 count = m_values.getSize();
	ANKI_SERIALIZE(count, 1);
	if(count != m_values.getSize())
	{
		ANKI_SCENE_LOGE("Variable count doesn't match the serialized one");
		return Error::kUserData;
	}

	for(U32 i = 0; i < count; ++i)
	{
		// Name serialized only for checks
		SceneString varName = m_names[i];
		ANKI_SERIALIZE(varName, 1);
		if(varName != m_names[i])
		{
			ANKI_SCENE_LOGE("Script var name doesn't match the serialized one: %s", varName.cstr());
			return Error::kUserData;
		}

		// Type serialized only for checks
		ScriptVariableType type = m_types[i];
		ANKI_SERIALIZE(type, 1);
		if(type != m_types[i])
		{
			ANKI_SCENE_LOGE("Script var type doesn't match the serialized one: %s", varName.cstr());
			return Error::kUserData;
		}

		ScriptVariable var;
		var.m_allVars = this;
		var.m_idx = i;
		switch(type)
		{
		case ScriptVariableType::kNumber:
		{
			F64 value = var.getNumber();
			ANKI_SERIALIZE(value, 1);
			var.setNumber(value);
			break;
		}
		case ScriptVariableType::kBool:
		{
			Bool value = var.getBool();
			ANKI_SERIALIZE(value, 1);
			var.setBool(value);
			break;
		}
		case ScriptVariableType::kString:
		{
			SceneString value = var.getString();
			ANKI_SERIALIZE(value, 1);
			var.setString(value);
			break;
		}
		case ScriptVariableType::kVec2:
		{
			Vec2 value = var.getVec2();
			ANKI_SERIALIZE(value, 1);
			var.setVec2(value);
			break;
		}
		case ScriptVariableType::kVec3:
		{
			Vec3 value = var.getVec3();
			ANKI_SERIALIZE(value, 1);
			var.setVec3(value);
			break;
		}
		case ScriptVariableType::kVec4:
		{
			Vec4 value = var.getVec4();
			ANKI_SERIALIZE(value, 1);
			var.setVec4(value);
			break;
		}
		default:
			ANKI_ASSERT(0);
		}
	}

	// Flush vars after reads to have an up-to-date environment
	if(serializer.isInReadMode())
	{
		flushDirtyVarsToLua(env);
	}

	return Error::kNone;
}

} // end namespace anki
