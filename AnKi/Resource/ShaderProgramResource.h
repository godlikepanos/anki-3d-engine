// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/ShaderCompiler/ShaderCompiler.h>
#include <AnKi/Gr/BackendCommon/Functions.h>
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

/// Shader program resource variant.
class ShaderProgramResourceVariant
{
	friend class ShaderProgramResource;

public:
	ShaderProgramResourceVariant();

	~ShaderProgramResourceVariant();

	/// @note On ray tracing program resources it points to the actual ray tracing program that contains everything.
	ShaderProgram& getProgram() const
	{
		return *m_prog;
	}

	/// Only for hit ray tracing programs.
	U32 getShaderGroupHandleIndex() const
	{
		ANKI_ASSERT(m_shaderGroupHandleIndex < kMaxU32);
		return m_shaderGroupHandleIndex;
	}

private:
	ShaderProgramPtr m_prog;
	U32 m_shaderGroupHandleIndex = kMaxU32; ///< Cache the index of the handle here.
};

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

	ShaderProgramResourceVariantInitInfo& addMutation(CString name, MutatorValue t);

	/// Request a non default technique and specific shaders.
	void requestTechniqueAndTypes(ShaderTypeBit types, CString technique = "Unnamed")
	{
		ANKI_ASSERT(types != ShaderTypeBit::kNone);
		ANKI_ASSERT(!(m_shaderTypes & types) && "Shader types already requested. Possibly programmer's error");
		m_shaderTypes |= types;

		const U32 len = technique.getLength();
		ANKI_ASSERT(len > 0 && len <= kMaxTechniqueNameLength);
		for(ShaderType type : EnumBitsIterable<ShaderType, ShaderTypeBit>(types))
		{
			memcpy(m_techniqueNames[type].getBegin(), technique.cstr(), len + 1);
		}
	}

private:
	static constexpr U32 kMaxMutators = 32;
	static constexpr U32 kMaxTechniqueNameLength = 32;

	ShaderProgramResourcePtr m_ptr;

	Array<MutatorValue, kMaxMutators> m_mutation; ///< The order of storing the values is important. It will be hashed.
	BitSet<kMaxMutators> m_setMutators = {false};

	Array<Array<Char, kMaxTechniqueNameLength + 1>, U32(ShaderType::kCount)> m_techniqueNames = {};
	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;
};

/// Shader program resource. It loads special AnKi programs.
class ShaderProgramResource : public ResourceObject
{
public:
	ShaderProgramResource();

	~ShaderProgramResource();

	/// Load the resource.
	Error load(const ResourceFilename& filename, Bool async);

	/// Try to find a mutator.
	const ShaderBinaryMutator* tryFindMutator(CString name) const
	{
		for(const ShaderBinaryMutator& m : m_binary->m_mutators)
		{
			if(m.m_name.getBegin() == name)
			{
				return &m;
			}
		}
		return nullptr;
	}

	const ShaderBinary& getBinary() const
	{
		return *m_binary;
	}

	/// Get or create a graphics shader program variant. If returned variant is nullptr then it means that the mutation is skipped and thus incorrect.
	/// @note It's thread-safe.
	void getOrCreateVariant(const ShaderProgramResourceVariantInitInfo& info, const ShaderProgramResourceVariant*& variant) const;

private:
	ShaderBinary* m_binary = nullptr;

	mutable ResourceHashMap<U64, ShaderProgramResourceVariant*> m_variants;
	mutable RWMutex m_mtx;

	ShaderProgramResourceVariant* createNewVariant(const ShaderProgramResourceVariantInitInfo& info) const;

	U32 findTechnique(CString name) const;
};

inline ShaderProgramResourceVariantInitInfo& ShaderProgramResourceVariantInitInfo::addMutation(CString name, MutatorValue t)
{
	const ShaderBinaryMutator* m = m_ptr->tryFindMutator(name);
	ANKI_ASSERT(m);
	[[maybe_unused]] Bool valueExits = false;
	for(auto v : m->m_values)
	{
		if(v == t)
		{
			valueExits = true;
			break;
		}
	}
	ANKI_ASSERT(valueExits);
	const PtrSize mutatorIdx = m - m_ptr->getBinary().m_mutators.getBegin();
	m_mutation[mutatorIdx] = t;
	m_setMutators.set(mutatorIdx);
	return *this;
}
/// @}

} // end namespace anki
