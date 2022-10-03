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
	kNone = 0,
	kTechnique,
	kLod,
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

	Bool isBoundableTexture() const
	{
		return m_dataType >= ShaderVariableDataType::kTextureFirst
			   && m_dataType <= ShaderVariableDataType::kTextureLast;
	}

	Bool isBindlessTexture() const
	{
		return m_dataType == ShaderVariableDataType::kU32 && m_image.get();
	}

	Bool isUniform() const
	{
		return !isBoundableTexture();
	}

	ShaderVariableDataType getDataType() const
	{
		ANKI_ASSERT(m_dataType != ShaderVariableDataType::kNone);
		return m_dataType;
	}

	/// Get the binding of a texture or a sampler type of material variable.
	U32 getTextureBinding() const
	{
		ANKI_ASSERT(m_opaqueBinding != kMaxU32 && isBoundableTexture());
		return m_opaqueBinding;
	}

	U32 getOffsetInLocalUniforms() const
	{
		ANKI_ASSERT(m_offsetInLocalUniforms != kMaxU32);
		return m_offsetInLocalUniforms;
	}

protected:
	String m_name;
	U32 m_offsetInLocalUniforms = kMaxU32;
	U32 m_opaqueBinding = kMaxU32; ///< Binding for textures and samplers.
	ShaderVariableDataType m_dataType = ShaderVariableDataType::kNone;

	/// Values
	/// @{
	union
	{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
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

#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	ANKI_SPECIALIZE_GET_VALUE(type, ANKI_CONCATENATE(m_, type))
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
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
/// 	<shaderPrograms>
///			<shaderProgram name="name of the shader">
///				[<mutation>
///					<mutator name="str" value="value"/>
///				</mutation>]
///			</shaderProgram>
///
///			[<shaderProgram ...>
///				...
///			</shaderProgram>]
/// 	</shaderPrograms>
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
	Error load(const ResourceFilename& filename, Bool async);

	Bool castsShadow() const
	{
		return !!(m_techniquesMask & (RenderingTechniqueBit::kShadow | RenderingTechniqueBit::kRtShadow));
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

	class Program;

	DynamicArray<Program> m_programs;

	Array<U8, U(RenderingTechnique::kCount)> m_techniqueToProgram;
	RenderingTechniqueBit m_techniquesMask = RenderingTechniqueBit::kNone;

	DynamicArray<MaterialVariable> m_vars;

	Bool m_supportsSkinning = false;

	DynamicArray<TexturePtr> m_textures;

	void* m_prefilledLocalUniforms = nullptr;
	U32 m_localUniformsSize = 0;

	Error parseMutators(XmlElement mutatorsEl, Program& prog);
	Error parseShaderProgram(XmlElement techniqueEl, Bool async);
	Error parseInput(XmlElement inputEl, Bool async, BitSet<128>& varsSet);
	Error findBuiltinMutators(Program& prog);
	Error createVars(Program& prog);
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
