// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Resource/RenderingKey.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Math.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>

namespace anki {

// Forward
class XmlElement;

/// @addtogroup resource
/// @{

/// The ID of builtin mutators.
enum class BuiltinMutatorId : U8
{
	kNone = 0,
	kBones,
	kVelocity,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BuiltinMutatorId)

/// Holds the shader variables. It's a container for shader program variables that share the same name.
class MaterialVariable
{
	friend class MaterialVariant;
	friend class MaterialResource;

public:
	MaterialVariable();

	MaterialVariable(const MaterialVariable&) = delete; // Non-copyable

	MaterialVariable(MaterialVariable&& b)
	{
		*this = std::move(b);
	}

	~MaterialVariable();

	MaterialVariable& operator=(const MaterialVariable&) = delete; // Non-copyable

	MaterialVariable& operator=(MaterialVariable&& b)
	{
		m_name = std::move(b.m_name);
		m_offsetInLocalConstants = b.m_offsetInLocalConstants;
		m_dataType = b.m_dataType;
		m_Mat4 = b.m_Mat4;
		m_image = std::move(b.m_image);
		return *this;
	}

	CString getName() const
	{
		return m_name;
	}

	template<typename T>
	const T& getValue() const;

	ShaderVariableDataType getDataType() const
	{
		ANKI_ASSERT(m_dataType != ShaderVariableDataType::kNone);
		return m_dataType;
	}

	U32 getOffsetInLocalConstants() const
	{
		ANKI_ASSERT(m_offsetInLocalConstants != kMaxU32);
		return m_offsetInLocalConstants;
	}

	ImageResource* tryGetImageResource() const
	{
		return (m_image) ? m_image.get() : nullptr;
	}

protected:
	ResourceString m_name;
	U32 m_offsetInLocalConstants = kMaxU32;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::kNone;

	/// Values
	/// @{
	union
	{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataType.def.h>
#undef ANKI_SVDT_MACRO
	};

	ImageResourcePtr m_image;
	/// @}
};

// Specialize the MaterialVariable::getValue
#define ANKI_SPECIALIZE_GET_VALUE(type, member) \
	template<> \
	inline const type& MaterialVariable::getValue<type>() const \
	{ \
		ANKI_ASSERT(m_dataType == ShaderVariableDataType::k##type); \
		return member; \
	}

#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) ANKI_SPECIALIZE_GET_VALUE(type, ANKI_CONCATENATE(m_, type))
#include <AnKi/Gr/ShaderVariableDataType.def.h>
#undef ANKI_SVDT_MACRO

#undef ANKI_SPECIALIZE_GET_VALUE

template<>
inline const ImageResourcePtr& MaterialVariable::getValue() const
{
	ANKI_ASSERT(m_image.get());
	return m_image;
}

/// Material variant.
class MaterialVariant
{
	friend class MaterialResource;

public:
	MaterialVariant() = default;

	MaterialVariant(const MaterialVariant&) = delete; // Non-copyable

	MaterialVariant& operator=(const MaterialVariant&) = delete; // Non-copyable

	const ShaderProgramPtr& getShaderProgram() const
	{
		return m_prog;
	}

	U32 getRtShaderGroupHandleIndex() const
	{
		ANKI_ASSERT(m_rtShaderGroupHandleIndex != kMaxU32);
		return m_rtShaderGroupHandleIndex;
	}

private:
	ShaderProgramPtr m_prog;
	U32 m_rtShaderGroupHandleIndex = kMaxU32;

	MaterialVariant(MaterialVariant&& b)
	{
		*this = std::move(b);
	}

	MaterialVariant& operator=(MaterialVariant&& b)
	{
		m_prog = std::move(b.m_prog);
		m_rtShaderGroupHandleIndex = b.m_rtShaderGroupHandleIndex;
		return *this;
	}
};

/// Material resource.
///
/// Material XML file format:
/// @code
///	<material [shadows="0|1"]>
/// 	<shaderProgram name="name of the shader" />
///			[<mutation>
///				<mutator name="str" value="value"/>
///			</mutation>]
///		</shaderProgram>
///
///		[<inputs>
///			<input name="name in AnKiLocalConstants struct" value="value(s)"/>
///		</inputs>]
///	</material>
/// @endcode
class MaterialResource : public ResourceObject
{
public:
	MaterialResource(CString fname, U32 uuid)
		: ResourceObject(fname, uuid)
	{
	}

	~MaterialResource();

	/// Load a material file
	Error load(const ResourceFilename& filename, Bool async);

	Bool castsShadow() const
	{
		return !!(m_techniquesMask & (RenderingTechniqueBit::kDepth | RenderingTechniqueBit::kRtShadow));
	}

	ConstWeakArray<MaterialVariable> getVariables() const
	{
		return m_vars;
	}

	Bool supportsSkinning() const
	{
		return m_supportsSkinning;
	}

	RenderingTechniqueBit getRenderingTechniques() const
	{
		ANKI_ASSERT(!!m_techniquesMask);
		return m_techniquesMask;
	}

	/// @note It's thread-safe.
	const MaterialVariant& getOrCreateVariant(const RenderingKey& key) const;

	/// Get a buffer with prefilled uniforms.
	ConstWeakArray<U8> getPrefilledLocalConstants() const
	{
		return ConstWeakArray<U8>(static_cast<const U8*>(m_prefilledLocalConstants), m_localConstantsSize);
	}

private:
	class PartialMutation
	{
	public:
		const ShaderBinaryMutator* m_mutator;
		MutatorValue m_value;
	};

	enum class ShaderTechniqueBit : U8
	{
		kNone = 0,
		kLegacy = 1 << 0,
		kMeshSaders = 1 << 1,
		kSwMeshletRendering = 1 << 2
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS_FRIEND(ShaderTechniqueBit)

	ShaderProgramResourcePtr m_prog;

	mutable Array4d<MaterialVariant, U(RenderingTechnique::kCount), 2, 2, 2> m_variantMatrix; ///< [technique][skinned][vel][meshletRendering]
	mutable RWMutex m_variantMatrixMtx;

	ResourceDynamicArray<PartialMutation> m_partialMutation; ///< Only with the non-builtins.

	ResourceDynamicArray<MaterialVariable> m_vars;

	void* m_prefilledLocalConstants = nullptr;
	U32 m_localConstantsSize = 0;

	U32 m_presentBuildinMutatorMask = 0;

	Bool m_supportsSkinning = false;
	RenderingTechniqueBit m_techniquesMask = RenderingTechniqueBit::kNone;
	ShaderTechniqueBit m_shaderTechniques = ShaderTechniqueBit::kNone;

	Error parseMutators(XmlElement mutatorsEl);
	Error parseShaderProgram(XmlElement techniqueEl, Bool async);
	Error parseInput(XmlElement inputEl, Bool async, BitSet<128>& varsSet);
	Error findBuiltinMutators();
	Error createVars();
	void prefillLocalConstants();

	const MaterialVariable* tryFindVariableInternal(CString name) const;

	const MaterialVariable* tryFindVariable(CString name) const
	{
		return tryFindVariableInternal(name);
	}

	MaterialVariable* tryFindVariable(CString name)
	{
		return const_cast<MaterialVariable*>(tryFindVariableInternal(name));
	}
};
/// @}

} // end namespace anki
