// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	NONE = 0,
	LOD,
	BONES,
	VELOCITY,

	COUNT,
	FIRST = 0
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
		m_offsetInLocalUniforms = b.m_offsetInLocalUniforms;
		m_opaqueBinding = b.m_opaqueBinding;
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

	Bool isTexture() const
	{
		return m_dataType >= ShaderVariableDataType::TEXTURE_FIRST
			   && m_dataType <= ShaderVariableDataType::TEXTURE_LAST;
	}

	Bool isUniform() const
	{
		return !isTexture();
	}

	ShaderVariableDataType getDataType() const
	{
		ANKI_ASSERT(m_dataType != ShaderVariableDataType::NONE);
		return m_dataType;
	}

	/// Get the binding of a texture or a sampler type of material variable.
	U32 getTextureBinding() const
	{
		ANKI_ASSERT(m_opaqueBinding != MAX_U32 && isTexture());
		return m_opaqueBinding;
	}

	U32 getOffsetInLocalUniforms() const
	{
		ANKI_ASSERT(m_offsetInLocalUniforms != MAX_U32);
		return m_offsetInLocalUniforms;
	}

protected:
	String m_name;
	U32 m_offsetInLocalUniforms = MAX_U32;
	U32 m_opaqueBinding = MAX_U32; ///< Binding for textures and samplers.
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;

	/// Values
	/// @{
	union
	{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
	};

	ImageResourcePtr m_image;
	/// @}
};

// Specialize the MaterialVariable::getValue
#define ANKI_SPECIALIZE_GET_VALUE(t_, var_, shaderType_) \
	template<> \
	inline const t_& MaterialVariable::getValue<t_>() const \
	{ \
		ANKI_ASSERT(m_dataType == ShaderVariableDataType::shaderType_); \
		return var_; \
	}

#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	ANKI_SPECIALIZE_GET_VALUE(type, ANKI_CONCATENATE(m_, type), capital)
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO

#undef ANKI_SPECIALIZE_GET_VALUE

template<>
inline const ImageResourcePtr& MaterialVariable::getValue() const
{
	ANKI_ASSERT(isTexture());
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
		ANKI_ASSERT(m_rtShaderGroupHandleIndex != MAX_U32);
		return m_rtShaderGroupHandleIndex;
	}

private:
	ShaderProgramPtr m_prog;
	U32 m_rtShaderGroupHandleIndex = MAX_U32;
};

/// Material resource.
///
/// Material XML file format:
/// @code
///	<material [shadows="0|1"]>
///		<shaderProgram filename="filename">
///			[<mutation>
///				<mutator name="str" value="value"/>
///			</mutation>]
///		</shaderProgram>
///
///		[<shaderProgram ...>
///			...
///		</shaderProgram>]
///
///		[<inputs>
///			<input name="name in AnKiMaterialUniforms struct or opaque type" value="value(s)"/> (1)
///		</inputs>]
///	</material>
/// @endcode
///
/// (1): Only for non-builtins.
class MaterialResource : public ResourceObject
{
public:
	MaterialResource(ResourceManager* manager);

	~MaterialResource();

	/// Load a material file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	Bool castsShadow() const
	{
		return !!(m_techniquesMask & (RenderingTechniqueBit::SHADOW | RenderingTechniqueBit::RT_SHADOW));
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

	/// Get all GPU resources of this material. Will be used for GPU refcounting.
	ConstWeakArray<TexturePtr> getAllTextures() const
	{
		return m_textures;
	}

	/// @note It's thread-safe.
	const MaterialVariant& getOrCreateVariant(const RenderingKey& key) const;

	/// Get a buffer with prefilled uniforms.
	ConstWeakArray<U8> getPrefilledLocalUniforms() const
	{
		return ConstWeakArray<U8>(static_cast<const U8*>(m_prefilledLocalUniforms), m_localUniformsSize);
	}

private:
	class PartialMutation
	{
	public:
		const ShaderProgramResourceMutator* m_mutator;
		MutatorValue m_value;
	};

	class Technique
	{
	public:
		ShaderProgramResourcePtr m_prog;
		U8 m_lodCount = 1;

		mutable Array3d<MaterialVariant, MAX_LOD_COUNT, 2, 2> m_variantMatrix; ///< Matrix of variants.
		mutable RWMutex m_variantMatrixMtx;

		DynamicArray<PartialMutation> m_partialMutation; ///< Only with the non-builtins.

		U32 m_presentBuildinMutators = 0;
		U32 m_localUniformsStructIdx = 0; ///< Struct index in the program binary.
	};

	Array<Technique, U(RenderingTechnique::COUNT)> m_techniques;
	RenderingTechniqueBit m_techniquesMask = RenderingTechniqueBit::NONE;

	DynamicArray<MaterialVariable> m_vars;

	Bool m_supportsSkinning = false;

	DynamicArray<TexturePtr> m_textures;

	void* m_prefilledLocalUniforms = nullptr;
	U32 m_localUniformsSize = 0;

	ANKI_USE_RESULT Error parseMutators(XmlElement mutatorsEl, Technique& technique);
	ANKI_USE_RESULT Error parseTechnique(XmlElement techniqueEl, Bool async);
	ANKI_USE_RESULT Error parseInput(XmlElement inputEl, Bool async, BitSet<128>& varsSet);
	ANKI_USE_RESULT Error findBuiltinMutators(Technique& technique);
	ANKI_USE_RESULT Error createVars(Technique& technique);
	void prefillLocalUniforms();

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
