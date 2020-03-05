// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/shader_compiler/Common.h>
#include <anki/gr/utils/Functions.h>
#include <anki/util/BitSet.h>
#include <anki/util/String.h>
#include <anki/util/HashMap.h>
#include <anki/util/WeakArray.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// The means to mutate a shader program.
/// @memberof ShaderProgramResource2
class ShaderProgramResourceMutator2 : public NonCopyable
{
public:
	String m_name;
	DynamicArray<MutatorValue> m_values;
};

/// @memberof ShaderProgramResource2
class ShaderProgramResourcePartialMutation2 : public NonCopyable
{
public:
	const ShaderProgramResourceMutator2* m_mutator = nullptr;
	MutatorValue m_value = 0;
	U64 m_hash = 0;
};

/// Shader program resource variant.
class ShaderProgramResourceInputVariable2
{
public:
	String m_name;
	Bool m_constant = false;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;

	Bool isTexture() const
	{
		return m_dataType >= ShaderVariableDataType::TEXTURE_FIRST
			   && m_dataType <= ShaderVariableDataType::TEXTURE_LAST;
	}

	Bool isSampler() const
	{
		return m_dataType == ShaderVariableDataType::SAMPLER;
	}

	Bool inBlock() const
	{
		return !m_constant && !isTexture() && !isSampler();
	}
};

/// Shader program resource variant.
class ShaderProgramResourceVariant2
{
public:
	// TODO
};

/// The value of a constant.
class ShaderProgramResourceConstantValue2
{
public:
	union
	{
		I32 m_int;
		IVec2 m_ivec2;
		IVec3 m_ivec3;
		IVec4 m_ivec4;

		U32 m_uint;
		UVec2 m_uvec2;
		UVec3 m_uvec3;
		UVec4 m_uvec4;

		F32 m_float;
		Vec2 m_vec2;
		Vec3 m_vec3;
		Vec4 m_vec4;
	};

	U32 m_inputVariableIndex;
	U8 _m_padding[sizeof(Vec4) - sizeof(m_inputVariableIndex)];

	ShaderProgramResourceConstantValue2()
	{
		zeroMemory(*this);
	}
};

static_assert(sizeof(ShaderProgramResourceConstantValue2) == sizeof(Vec4) * 2, "Need it to be packed");

/// Shader program resource. It loads special AnKi programs.
class ShaderProgramResource2 : public ResourceObject
{
public:
	ShaderProgramResource2(ResourceManager* manager);

	~ShaderProgramResource2();

	/// Load the resource.
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	/// Get the array of input variables.
	const DynamicArray<ShaderProgramResourceInputVariable2>& getInputVariables() const
	{
		return m_inputVars;
	}

	/// Try to find an input variable.
	const ShaderProgramResourceInputVariable2* tryFindInputVariable(CString name) const
	{
		for(const ShaderProgramResourceInputVariable2& m : m_inputVars)
		{
			if(m.m_name == name)
			{
				return &m;
			}
		}
		return nullptr;
	}

	/// Get the array of mutators.
	const DynamicArray<ShaderProgramResourceMutator2>& getMutators() const
	{
		return m_mutators;
	}

	/// Try to find a mutator.
	const ShaderProgramResourceMutator2* tryFindMutator(CString name) const
	{
		for(const ShaderProgramResourceMutator2& m : m_mutators)
		{
			if(m.m_name == name)
			{
				return &m;
			}
		}
		return nullptr;
	}

	/// Has tessellation shaders.
	Bool hasTessellation() const
	{
		return !!(m_shaderStages & ShaderTypeBit::TESSELLATION_EVALUATION);
	}

private:
	using Mutator = ShaderProgramResourceMutator2;
	using Input = ShaderProgramResourceInputVariable2;

	DynamicArray<Input> m_inputVars;
	DynamicArray<Mutator> m_mutators;

	mutable HashMap<U64, ShaderProgramResourceVariant2*> m_variants;
	mutable Mutex m_mtx;

	ShaderTypeBit m_shaderStages = ShaderTypeBit::NONE;
};

/// Smart initializer of multiple ShaderProgramResourceConstantValue.
class ShaderProgramResourceConstantValueInitList2
{
public:
	ShaderProgramResourceConstantValueInitList2(ShaderProgramResource2Ptr ptr)
		: m_ptr(ptr)
	{
	}

	~ShaderProgramResourceConstantValueInitList2()
	{
	}

	template<typename T>
	ShaderProgramResourceConstantValueInitList2& add(CString name, const T& t)
	{
		const ShaderProgramResourceInputVariable2* in = m_ptr->tryFindInputVariable(name);
		ANKI_ASSERT(in);
		ANKI_ASSERT(in->m_constant);
		ANKI_ASSERT(in->m_dataType == getShaderVariableTypeFromTypename<T>());
		m_constantValues[m_count].m_inputVariableIndex = U32(in - m_ptr->getInputVariables().getBegin());
		memcpy(&m_constantValues[m_count].m_int, &t, sizeof(T));
		++m_count;
		return *this;
	}

	ConstWeakArray<ShaderProgramResourceConstantValue2> get() const
	{
		// TODO check if all are set
		return ConstWeakArray<ShaderProgramResourceConstantValue2>(&m_constantValues[0], m_count);
	}

	ShaderProgramResourceConstantValue2& operator[](U32 idx)
	{
		return m_constantValues[idx];
	}

private:
	ShaderProgramResource2Ptr m_ptr;
	U32 m_count = 0;
	Array<ShaderProgramResourceConstantValue2, 64> m_constantValues;
};

/// Smart initializer of multiple ShaderProgramResourceMutation2.
class ShaderProgramResourceMutationInitList2
{
public:
	ShaderProgramResourceMutationInitList2(ShaderProgramResource2Ptr ptr)
		: m_ptr(ptr)
	{
	}

	~ShaderProgramResourceMutationInitList2()
	{
	}

	ShaderProgramResourceMutationInitList2& add(CString name, MutatorValue t)
	{
		const ShaderProgramResourceMutator2* m = m_ptr->tryFindMutator(name);
		ANKI_ASSERT(m);
		m_mutation[m_count] = t;
		m_setMutators.set(m - m_ptr->getMutators().getBegin());
		++m_count;
		return *this;
	}

private:
	static constexpr U32 MAX_MUTATORS = 64;

	ShaderProgramResource2Ptr m_ptr;
	U32 m_count = 0;
	Array<MutatorValue, MAX_MUTATORS> m_mutation;
	BitSet<MAX_MUTATORS> m_setMutators = {false};
};
/// @}

} // end namespace anki
