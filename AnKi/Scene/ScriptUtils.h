// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Util/DynamicBitSet.h>

namespace anki {

class ScriptEnvironment;
class ScriptVariables;
class SceneSerializer;

enum class ScriptVariableType : U8
{
	kNumber,
	kBool,
	kString,
	kVec2,
	kVec3,
	kVec4,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ScriptVariableType)

// It mirrors the global variables of a script environment.
class ScriptVariables
{
	friend class ScriptVariable;

public:
	ANKI_NON_COPYABLE(ScriptVariables)

	ScriptVariables() = default;

	~ScriptVariables()
	{
		destroy();
	}

	void destroy();

	// Look at the environment and gather all the global variables.
	void rebuildVarsFromLua(ScriptEnvironment& env);

	// Update the variables in the environment. C++ to LUA.
	void flushDirtyVarsToLua(ScriptEnvironment& env);

	// Update the mirorred vars from the environment. LUA to C++.
	void updateVarsFromLua(ScriptEnvironment& env);

	template<typename TFunc>
	FunctorContinue iterateVariables(TFunc func);

	// It assumes that rebuildVarsFromLua() is already called to populate the vars
	Error serialize(SceneSerializer& serializer, ScriptEnvironment& env);

private:
	union Value
	{
		F64 m_number = 0.0;
		Bool m_bool;
		Char* m_string;
		Vec2 m_vec2;
		Vec3 m_vec3;
		Vec4 m_vec4;
	};

	SceneDynamicArray<SceneString> m_names;
	SceneDynamicArray<U64> m_nameHashes;
	SceneDynamicArray<Value> m_values;
	SceneDynamicArray<ScriptVariableType> m_types;
	SceneDynamicBitSet<> m_dirty;

	Bool findVar(CString name, ScriptVariableType type, U32& idx) const;
};

class ScriptVariable
{
	friend class ScriptVariables;

public:
	ANKI_NON_COPYABLE(ScriptVariable)

	ScriptVariable() = default;

	CString getName() const
	{
		return m_allVars->m_names[m_idx];
	}

	ScriptVariableType getType() const
	{
		return m_allVars->m_types[m_idx];
	}

	void setNumber(F64 value)
	{
		if(ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kNumber) && m_allVars->m_values[m_idx].m_number != value)
		{
			m_allVars->m_values[m_idx].m_number = value;
			m_allVars->m_dirty.setBit(m_idx);
		}
	}

	F64 getNumber() const
	{
		return (ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kNumber)) ? m_allVars->m_values[m_idx].m_number : 0.0;
	}

	void setBool(Bool value)
	{
		if(ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kBool) && m_allVars->m_values[m_idx].m_bool != value)
		{
			m_allVars->m_values[m_idx].m_bool = value;
			m_allVars->m_dirty.setBit(m_idx);
		}
	}

	Bool getBool() const
	{
		return (ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kBool)) ? m_allVars->m_values[m_idx].m_bool : false;
	}

	void setString(CString value)
	{
		const U32 len = value.getLength();
		if(ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kString))
		{
			deleteInstance(SceneMemoryPool::getSingleton(), m_allVars->m_values[m_idx].m_string);
			m_allVars->m_values[m_idx].m_string = newArray<Char>(SceneMemoryPool::getSingleton(), len + 1);
			if(len > 0)
			{
				memcpy(m_allVars->m_values[m_idx].m_string, value.cstr(), len + 1);
			}
			else
			{
				m_allVars->m_values[m_idx].m_string[0] = '\0';
			}

			m_allVars->m_dirty.setBit(m_idx);
		}
	}

	CString getString() const
	{
		return (ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kString)) ? CString(m_allVars->m_values[m_idx].m_string) : CString();
	}

	void setVec2(Vec2 value)
	{
		if(ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kVec2) && m_allVars->m_values[m_idx].m_vec2 != value)
		{
			m_allVars->m_values[m_idx].m_vec2 = value;
			m_allVars->m_dirty.setBit(m_idx);
		}
	}

	Vec2 getVec2() const
	{
		return (ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kVec2)) ? m_allVars->m_values[m_idx].m_vec2 : Vec2(0.0f);
	}

	void setVec3(Vec3 value)
	{
		if(ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kVec3) && m_allVars->m_values[m_idx].m_vec3 != value)
		{
			m_allVars->m_values[m_idx].m_vec3 = value;
			m_allVars->m_dirty.setBit(m_idx);
		}
	}

	Vec3 getVec3() const
	{
		return (ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kVec3)) ? m_allVars->m_values[m_idx].m_vec3 : Vec3(0.0f);
	}

	void setVec4(Vec4 value)
	{
		if(ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kVec4) && m_allVars->m_values[m_idx].m_vec4 != value)
		{
			m_allVars->m_values[m_idx].m_vec4 = value;
			m_allVars->m_dirty.setBit(m_idx);
		}
	}

	Vec4 getVec4() const
	{
		return (ANKI_EXPECT(m_allVars->m_types[m_idx] == ScriptVariableType::kVec4)) ? m_allVars->m_values[m_idx].m_vec4 : Vec4(0.0f);
	}

private:
	ScriptVariables* m_allVars = nullptr;
	U32 m_idx = kMaxU32;
};

template<typename TFunc>
FunctorContinue ScriptVariables::iterateVariables(TFunc func)
{
	FunctorContinue cont = FunctorContinue::kContinue;
	for(U32 i = 0; i < m_names.getSize(); ++i)
	{
		ScriptVariable var;
		var.m_allVars = this;
		var.m_idx = i;
		cont = func(var);
		if(cont == FunctorContinue::kStop)
		{
			break;
		}
	}

	return cont;
}

} // end namespace anki
