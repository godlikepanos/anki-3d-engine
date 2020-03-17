// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/shader_compiler/ShaderProgramCompiler.h>
#include <anki/gr/utils/Functions.h>
#include <anki/util/BitSet.h>
#include <anki/util/String.h>
#include <anki/util/HashMap.h>
#include <anki/util/WeakArray.h>
#include <anki/Math.h>

namespace anki
{

// Forward
class ShaderProgramResourceVariantInitInfo2;

/// @addtogroup resource
/// @{

/// The means to mutate a shader program.
/// @memberof ShaderProgramResource2
class ShaderProgramResourceMutator2 : public NonCopyable
{
public:
	CString m_name;
	ConstWeakArray<MutatorValue> m_values;
};

/// Shader program resource variant.
class ShaderProgramResourceInputVariable2
{
public:
	String m_name;
	U32 m_index = MAX_U32;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;
	Bool m_constant = false;
	Bool m_instanced = false;

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
	friend class ShaderProgramResource2;

public:
	ShaderProgramResourceVariant2();

	~ShaderProgramResourceVariant2();

	const ShaderProgramPtr& getProgram() const
	{
		return m_prog;
	}

	/// Return true of the the variable is active.
	Bool variableActive(const ShaderProgramResourceInputVariable2& var) const
	{
		return m_activeInputVars.get(var.m_index);
	}

private:
	ShaderProgramPtr m_prog;

	BitSet<128, U64> m_activeInputVars = {false};
};

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

	/// Get or create a graphics shader program variant.
	/// @note It's thread-safe.
	void getOrCreateVariant(
		const ShaderProgramResourceVariantInitInfo2& info, const ShaderProgramResourceVariant2*& variant) const;

private:
	using Mutator = ShaderProgramResourceMutator2;
	using Input = ShaderProgramResourceInputVariable2;

	ShaderProgramBinaryWrapper m_binary;

	DynamicArray<Input> m_inputVars;
	DynamicArray<Mutator> m_mutators;

	mutable HashMap<U64, ShaderProgramResourceVariant2*> m_variants;
	mutable RWMutex m_mtx;

	ShaderTypeBit m_shaderStages = ShaderTypeBit::NONE;

	void initVariant(const ShaderProgramResourceVariantInitInfo2& info, ShaderProgramResourceVariant2& variant) const;

	static ANKI_USE_RESULT Error parseVariable(CString fullVarName, Bool& instanced, U32& idx, CString& name);

	static ANKI_USE_RESULT Error parseConst(CString constName, U32& componentIdx, U32& componentCount, CString& name);
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

/// Smart initializer of multiple ShaderProgramResourceConstantValue.
class ShaderProgramResourceVariantInitInfo2
{
	friend class ShaderProgramResource2;

public:
	ShaderProgramResourceVariantInitInfo2(const ShaderProgramResource2Ptr& ptr)
		: m_ptr(ptr)
	{
	}

	~ShaderProgramResourceVariantInitInfo2()
	{
	}

	template<typename T>
	ShaderProgramResourceVariantInitInfo2& addConstant(CString name, const T& value)
	{
		const ShaderProgramResourceInputVariable2* in = m_ptr->tryFindInputVariable(name);
		ANKI_ASSERT(in);
		ANKI_ASSERT(in->m_constant);
		ANKI_ASSERT(in->m_dataType == getShaderVariableTypeFromTypename<T>());
		m_constantValues[m_constantValueCount].m_inputVariableIndex = U32(in - m_ptr->getInputVariables().getBegin());
		memcpy(&m_constantValues[m_constantValueCount].m_int, &value, sizeof(T));
		++m_constantValueCount;
		return *this;
	}

	ShaderProgramResourceVariantInitInfo2& addMutation(CString name, MutatorValue t)
	{
		const ShaderProgramResourceMutator2* m = m_ptr->tryFindMutator(name);
		const PtrSize idx = m - m_ptr->getMutators().getBegin();
		ANKI_ASSERT(m);
		m_mutation[idx] = t;
		m_setMutators.set(idx);
		++m_mutationCount;
		return *this;
	}

private:
	static constexpr U32 MAX_CONSTANTS = 32;
	static constexpr U32 MAX_MUTATORS = 32;

	ShaderProgramResource2Ptr m_ptr;

	U32 m_constantValueCount = 0;
	Array<ShaderProgramResourceConstantValue2, MAX_CONSTANTS> m_constantValues;

	U32 m_mutationCount = 0;
	Array<MutatorValue, MAX_MUTATORS> m_mutation; ///< The order of storing the values is important. It will be hashed.
	BitSet<MAX_MUTATORS> m_setMutators = {false};
};
/// @}

} // end namespace anki
