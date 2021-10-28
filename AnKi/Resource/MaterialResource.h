// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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

namespace anki
{

// Forward
class XmlElement;

/// @addtogroup resource
/// @{

/// The ID of a buildin material variable.
enum class BuiltinMaterialVariableId : U8
{
	NONE = 0,
	MODEL_MATRIX,
	BONE_TRANSFORMS_ADDRESS,
	PREVIOUS_BONE_TRANSFORMS_ADDRESS,

	COUNT,
	FIRST = 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BuiltinMaterialVariableId)

/// The ID of builtin mutators.
enum class BuiltinMutatorId : U8
{
	NONE = 0,
	SUB_TECHNIQUE,
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
		m_numericValueIsSet = b.m_numericValueIsSet;
		m_dataType = b.m_dataType;
		m_builtin = b.m_builtin;
		m_offsetInStruct = b.m_offsetInStruct;
		m_Mat4 = b.m_Mat4;
		m_image = std::move(b.m_image);
		return *this;
	}

	/// Get the builtin info.
	BuiltinMaterialVariableId getBuiltin() const
	{
		return m_builtin;
	}

	CString getName() const
	{
		return m_name;
	}

	template<typename T>
	const T& getValue() const;

	Bool isTexture() const
	{
		return m_image.isCreated();
	}

	Bool isNumeric() const
	{
		return !isTexture();
	}

	Bool isBuiltin() const
	{
		return m_builtin != BuiltinMaterialVariableId::NONE;
	}

	ShaderVariableDataType getDataType() const
	{
		ANKI_ASSERT(m_dataType != ShaderVariableDataType::NONE);
		return m_dataType;
	}

protected:
	// !!WARNING!! Don't forget to update the move operator

	String m_name;
	Bool m_numericValueIsSet = false; ///< Is numeric and the unamed union bellow is set.
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;
	BuiltinMaterialVariableId m_builtin = BuiltinMaterialVariableId::NONE;
	U32 m_offsetInStruct = MAX_U32; ///< Offset in the AnKiGpuSceneDescriptor.

	/// Values for non-builtins
	/// @{
	union
	{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

		U16 m_bindlessTextureIndex;
	};

	ImageResourcePtr m_image;
	/// @}

	Bool valueSetByMaterial() const
	{
		return m_image.isCreated() || m_numericValueIsSet;
	}
};

// Specialize the MaterialVariable::getValue
#define ANKI_SPECIALIZE_GET_VALUE(type, var, shaderType) \
	template<> \
	inline const type& MaterialVariable::getValue<type>() const \
	{ \
		ANKI_ASSERT(m_dataType == ShaderVariableDataType::shaderType); \
		ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE); \
		return var; \
	}

#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	ANKI_SPECIALIZE_GET_VALUE(type, ANKI_CONCATENATE(m_, type), capital)
#include <AnKi/Gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

#undef ANKI_SPECIALIZE_GET_VALUE

template<>
inline const ImageResourcePtr& MaterialVariable::getValue() const
{
	ANKI_ASSERT(isTexture());
	ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE);
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

private:
	ShaderProgramPtr m_prog;
};

/// Material resource.
///
/// Material XML file format:
/// @code
///	<material [shadows="0|1"]>
///		<technique name="str" shaderProgram="filename">
///			[<mutation>
///				<mutator name="str" value="value"/>
///			</mutation>]
///		</technique>
///
///		[<inputs>
///			<input name="name in GpuSceneMaterialDescription" value="values" [arrayIndex="number"]/> (1)
///		</inputs>]
///
///		[<technique ...>
///			...
///		</technique>]
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
		return m_shadow;
	}

	ConstWeakArray<MaterialVariable> getVariables() const
	{
		return m_vars;
	}

	Bool supportsSkinning() const
	{
		return m_supportSkinning;
	}

	const MaterialVariant& getOrCreateVariant(const RenderingKey& key) const;

private:
	class SubMutation
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

		U32 m_perDrawUboBinding = MAX_U32;
		U32 m_perInstanceUboBinding = MAX_U32;
		U32 m_boneTrfsBinding = MAX_U32;
		U32 m_prevFrameBoneTrfsBinding = MAX_U32;

		U32 m_rtShaderGroupHandleIndex = MAX_U32;

		/// Matrix of variants.
		mutable Array4d<MaterialVariant, U(RenderingSubTechnique::COUNT), MAX_LOD_COUNT, 2, 2> m_variantMatrix;
		mutable RWMutex m_variantMatrixMtx;

		Array<const ShaderProgramResourceMutator*, U32(BuiltinMutatorId::COUNT)> m_builtinMutators = {};

		DynamicArray<SubMutation> m_nonBuiltinsMutation;
	};

	DynamicArray<MaterialVariable> m_vars;
	Array<Technique, U32(RenderingTechnique::COUNT)> m_techniques;
	U32 m_gpuSceneDescriptionStructSize = 0;
	Bool m_shadow = true;
	Bool m_supportSkinning = false;

	/// Parse whatever is inside the <inputs> tag.
	ANKI_USE_RESULT Error parseInputs(XmlElement inputsEl, Bool async);

	ANKI_USE_RESULT Error parseTechnique(XmlElement techniqueEl, Bool async);

	ANKI_USE_RESULT Error parseMutators(XmlElement mutatorsEl, Technique& technique);

	ANKI_USE_RESULT Error createVars(const ShaderProgramBinary& binary);

	ANKI_USE_RESULT Error findBuiltinMutators(Technique& technique);

	const MaterialVariable* tryFindVariableInternal(CString name) const
	{
		for(const MaterialVariable& v : m_vars)
		{
			if(v.m_name == name)
			{
				return &v;
			}
		}

		return nullptr;
	}

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
