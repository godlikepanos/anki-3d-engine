// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Script/ScriptEnvironment.h>

namespace anki {

enum class ScriptComponentVariableType : U8
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
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ScriptComponentVariableType)

class ScriptComponentVariable
{
	friend class ScriptComponent;

	template<typename, typename, typename>
	friend class DynamicArray;

public:
	ANKI_NON_COPYABLE(ScriptComponentVariable)

	void setNumber(F64 value)
	{
		if(ANKI_EXPECT(m_type == ScriptComponentVariableType::kNumber) && m_number != value)
		{
			m_number = value;
			m_dirty = true;
		}
	}

	F64 getNumber() const
	{
		return (ANKI_EXPECT(m_type == ScriptComponentVariableType::kNumber)) ? m_number : 0.0;
	}

	void setBool(Bool value)
	{
		if(ANKI_EXPECT(m_type == ScriptComponentVariableType::kBool) && m_bool != value)
		{
			m_bool = value;
			m_dirty = true;
		}
	}

	Bool getBool() const
	{
		return (ANKI_EXPECT(m_type == ScriptComponentVariableType::kBool)) ? m_bool : false;
	}

	void setString(CString value)
	{
		if(ANKI_EXPECT(m_type == ScriptComponentVariableType::kString))
		{
			m_string = value;
			m_dirty = true;
		}
	}

	CString getString() const
	{
		return (ANKI_EXPECT(m_type == ScriptComponentVariableType::kString)) ? m_string.toCString() : CString();
	}

	void setVec2(Vec2 value)
	{
		if(ANKI_EXPECT(m_type == ScriptComponentVariableType::kVec2) && m_vec2 != value)
		{
			m_vec2 = value;
			m_dirty = true;
		}
	}

	Vec2 getVec2() const
	{
		return (ANKI_EXPECT(m_type == ScriptComponentVariableType::kVec2)) ? m_vec2 : Vec2(0.0f);
	}

	void setVec3(Vec3 value)
	{
		if(ANKI_EXPECT(m_type == ScriptComponentVariableType::kVec3) && m_vec3 != value)
		{
			m_vec3 = value;
			m_dirty = true;
		}
	}

	Vec3 getVec3() const
	{
		return (ANKI_EXPECT(m_type == ScriptComponentVariableType::kVec3)) ? m_vec3 : Vec3(0.0f);
	}

	void setVec4(Vec4 value)
	{
		if(ANKI_EXPECT(m_type == ScriptComponentVariableType::kVec4) && m_vec4 != value)
		{
			m_vec4 = value;
			m_dirty = true;
		}
	}

	Vec4 getVec4() const
	{
		return (ANKI_EXPECT(m_type == ScriptComponentVariableType::kVec4)) ? m_vec4 : Vec4(0.0f);
	}

	ScriptComponentVariableType getType() const
	{
		return m_type;
	}

	CString getName() const
	{
		return m_name;
	}

private:
	union
	{
		F64 m_number = 0.0;
		Bool m_bool;
		SceneString m_string;
		Vec2 m_vec2;
		Vec3 m_vec3;
		Vec4 m_vec4;
	};

	SceneString m_name;
	ScriptComponentVariableType m_type = ScriptComponentVariableType::kCount;
	Bool m_dirty = false;

	ScriptComponentVariable()
	{
		callConstructor(m_string);
	}

	ScriptComponentVariable(ScriptComponentVariable&& b)
	{
		callConstructor(m_string);
		*this = std::move(b);
	}

	~ScriptComponentVariable()
	{
		if(m_type == ScriptComponentVariableType::kString)
		{
			m_string.destroy();
		}
	}

	ScriptComponentVariable& operator=(ScriptComponentVariable&& b);
};

// Component of scripts. It can point to a resource with the script code or have the script code embedded to it.
class ScriptComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ScriptComponent)

public:
	ScriptComponent(const SceneComponentInitInfo& init);

	~ScriptComponent();

	ScriptComponent& setScriptResourceFilename(CString fname);

	CString getScriptResourceFilename() const;

	ScriptComponent& setScriptText(CString text);

	CString getScriptText() const;

	Bool hasScriptText() const
	{
		return !m_text.isEmpty();
	}

	Bool hasScriptResource() const
	{
		return m_resource.isCreated();
	}

	Bool isValid() const
	{
		return m_environments[0] || m_environments[1];
	}

	template<typename TFunc>
	FunctorContinue iterateVariables(TFunc func)
	{
		FunctorContinue cont = FunctorContinue::kContinue;
		for(ScriptComponentVariable& var : m_vars)
		{
			cont = func(var);
			if(cont == FunctorContinue::kStop)
			{
				break;
			}
		}
		return cont;
	}

private:
	ScriptResourcePtr m_resource;
	SceneString m_text;
	Array<ScriptEnvironment*, 2> m_environments = {}; // One env if it contains its source and another if it's a resource

	SceneDynamicArray<ScriptComponentVariable> m_vars;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;

	void rebuildVarsFromLua(ScriptEnvironment& env);
	void flushDirtyVarsToLua(ScriptEnvironment& env);
};

} // end namespace anki
