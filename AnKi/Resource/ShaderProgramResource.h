// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/ShaderCompiler/ShaderProgramCompiler.h>
#include <AnKi/Gr/Utils/Functions.h>
#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Math.h>

namespace anki {

// Forward
class ShaderProgramResourceVariantInitInfo;

/// @addtogroup resource
/// @{

/// The means to mutate a shader program.
/// @memberof ShaderProgramResource
class ShaderProgramResourceMutator
{
public:
	CString m_name;
	ConstWeakArray<MutatorValue> m_values;

	ShaderProgramResourceMutator() = default;

	ShaderProgramResourceMutator(const ShaderProgramResourceMutator&) = delete; // Non-copyable

	ShaderProgramResourceMutator& operator=(const ShaderProgramResourceMutator&) = delete; // Non-copyable

	Bool valueExists(MutatorValue v) const
	{
		for(MutatorValue x : m_values)
		{
			if(v == x)
			{
				return true;
			}
		}
		return false;
	}
};

/// Shader program resource variant.
class ShaderProgramResourceConstant
{
public:
	String m_name;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::kNone;
	U32 m_index = kMaxU32;
};

/// Shader program resource variant.
class ShaderProgramResourceVariant
{
	friend class ShaderProgramResource;

public:
	ShaderProgramResourceVariant();

	~ShaderProgramResourceVariant();

	/// @note On ray tracing program resources it points to the actual ray tracing program that contains everything.
	const ShaderProgramPtr& getProgram() const
	{
		return m_prog;
	}

	/// Return true if the the variable is active in this variant.
	Bool isConstantActive(const ShaderProgramResourceConstant& var) const
	{
		return m_activeConsts.get(var.m_index);
	}

	const ShaderProgramBinaryVariant& getBinaryVariant() const
	{
		ANKI_ASSERT(m_binaryVariant);
		return *m_binaryVariant;
	}

	const Array<U32, 3>& getWorkgroupSizes() const
	{
		ANKI_ASSERT(m_workgroupSizes[0] != kMaxU32 && m_workgroupSizes[0] != 0);
		ANKI_ASSERT(m_workgroupSizes[1] != kMaxU32 && m_workgroupSizes[1] != 0);
		ANKI_ASSERT(m_workgroupSizes[2] != kMaxU32 && m_workgroupSizes[2] != 0);
		return m_workgroupSizes;
	}

	/// Only for hit ray tracing programs.
	U32 getShaderGroupHandleIndex() const
	{
		ANKI_ASSERT(m_shaderGroupHandleIndex < kMaxU32);
		return m_shaderGroupHandleIndex;
	}

private:
	ShaderProgramPtr m_prog;
	const ShaderProgramBinaryVariant* m_binaryVariant = nullptr;
	BitSet<128, U64> m_activeConsts = {false};
	Array<U32, 3> m_workgroupSizes;
	U32 m_shaderGroupHandleIndex = kMaxU32; ///< Cache the index of the handle here.
};

/// The value of a constant.
class ShaderProgramResourceConstantValue
{
public:
	union
	{
		U32 m_uint;
		UVec2 m_uvec2;
		UVec3 m_uvec3;
		UVec4 m_uvec4;

		I32 m_int;
		IVec2 m_ivec2;
		IVec3 m_ivec3;
		IVec4 m_ivec4;

		F32 m_float;
		Vec2 m_vec2;
		Vec3 m_vec3;
		Vec4 m_vec4;
	};

	U32 m_constantIndex;
	U8 _m_padding[sizeof(Vec4) - sizeof(m_constantIndex)];

	ShaderProgramResourceConstantValue()
	{
		zeroMemory(*this);
	}
};
static_assert(sizeof(ShaderProgramResourceConstantValue) == sizeof(Vec4) * 2, "Need it to be packed");

/// Smart initializer of multiple ShaderProgramResourceConstantValue.
class ShaderProgramResourceVariantInitInfo
{
	friend class ShaderProgramResource;

public:
	ShaderProgramResourceVariantInitInfo()
	{
	}

	ShaderProgramResourceVariantInitInfo(const ShaderProgramResourcePtr& ptr)
		: m_ptr(ptr)
	{
	}

	~ShaderProgramResourceVariantInitInfo()
	{
	}

	template<typename T>
	ShaderProgramResourceVariantInitInfo& addConstant(CString name, const T& value);

	ShaderProgramResourceVariantInitInfo& addMutation(CString name, MutatorValue t);

private:
	static constexpr U32 kMaxConstants = 32;
	static constexpr U32 kMaxMutators = 32;

	ShaderProgramResourcePtr m_ptr;

	Array<ShaderProgramResourceConstantValue, kMaxConstants> m_constantValues;
	BitSet<kMaxConstants> m_setConstants = {false};

	Array<MutatorValue, kMaxMutators> m_mutation; ///< The order of storing the values is important. It will be hashed.
	BitSet<kMaxMutators> m_setMutators = {false};
};

/// Shader program resource. It loads special AnKi programs.
class ShaderProgramResource : public ResourceObject
{
public:
	ShaderProgramResource(ResourceManager* manager);

	~ShaderProgramResource();

	/// Load the resource.
	Error load(const ResourceFilename& filename, Bool async);

	/// Get the array of constants.
	ConstWeakArray<ShaderProgramResourceConstant> getConstants() const
	{
		return m_consts;
	}

	/// Try to find a constant.
	const ShaderProgramResourceConstant* tryFindConstant(CString name) const
	{
		for(const ShaderProgramResourceConstant& m : m_consts)
		{
			if(m.m_name == name)
			{
				return &m;
			}
		}
		return nullptr;
	}

	/// Get the array of mutators.
	ConstWeakArray<ShaderProgramResourceMutator> getMutators() const
	{
		return m_mutators;
	}

	/// Try to find a mutator.
	const ShaderProgramResourceMutator* tryFindMutator(CString name) const
	{
		for(const ShaderProgramResourceMutator& m : m_mutators)
		{
			if(m.m_name == name)
			{
				return &m;
			}
		}
		return nullptr;
	}

	ShaderTypeBit getStages() const
	{
		return m_shaderStages;
	}

	const ShaderProgramBinary& getBinary() const
	{
		return m_binary.getBinary();
	}

	/// Get or create a graphics shader program variant. If returned variant is nullptr then it means that the mutation
	/// is skipped and thus incorrect.
	/// @note It's thread-safe.
	void getOrCreateVariant(const ShaderProgramResourceVariantInitInfo& info,
							const ShaderProgramResourceVariant*& variant) const;

	/// @copydoc getOrCreateVariant
	void getOrCreateVariant(const ShaderProgramResourceVariant*& variant) const
	{
		getOrCreateVariant(ShaderProgramResourceVariantInitInfo(), variant);
	}

private:
	using Mutator = ShaderProgramResourceMutator;
	using Const = ShaderProgramResourceConstant;

	ShaderProgramBinaryWrapper m_binary;

	DynamicArray<Const> m_consts;
	DynamicArray<Mutator> m_mutators;

	class ConstMapping
	{
	public:
		U32 m_component = 0;
		U32 m_constsIdx = 0; ///< Index in m_consts
	};

	DynamicArray<ConstMapping> m_constBinaryMapping;

	mutable HashMap<U64, ShaderProgramResourceVariant*> m_variants;
	mutable RWMutex m_mtx;

	ShaderTypeBit m_shaderStages = ShaderTypeBit::kNone;

	ShaderProgramResourceVariant* createNewVariant(const ShaderProgramResourceVariantInitInfo& info) const;

	static Error parseConst(CString constName, U32& componentIdx, U32& componentCount, CString& name);
};

template<typename T>
inline ShaderProgramResourceVariantInitInfo& ShaderProgramResourceVariantInitInfo::addConstant(CString name,
																							   const T& value)
{
	const ShaderProgramResourceConstant* in = m_ptr->tryFindConstant(name);
	if(in != nullptr)
	{
		ANKI_ASSERT(in->m_dataType == getShaderVariableTypeFromTypename<T>());
		const U32 constIdx = U32(in - m_ptr->getConstants().getBegin());
		m_constantValues[constIdx].m_constantIndex = constIdx;
		memcpy(&m_constantValues[constIdx].m_int, &value, sizeof(T));
		m_setConstants.set(constIdx);
	}
	else
	{
		ANKI_RESOURCE_LOGW("Constant not found: %s", name.cstr());
	}
	return *this;
}

inline ShaderProgramResourceVariantInitInfo& ShaderProgramResourceVariantInitInfo::addMutation(CString name,
																							   MutatorValue t)
{
	const ShaderProgramResourceMutator* m = m_ptr->tryFindMutator(name);
	ANKI_ASSERT(m);
	ANKI_ASSERT(m->valueExists(t));
	const PtrSize mutatorIdx = m - m_ptr->getMutators().getBegin();
	m_mutation[mutatorIdx] = t;
	m_setMutators.set(mutatorIdx);
	return *this;
}
/// @}

} // end namespace anki
