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
#include <AnKi/Shaders/Include/ModelTypes.h>

namespace anki {

// Forward
class XmlElement;

/// @addtogroup resource
/// @{

/// The ID of a buildin material variable.
enum class BuiltinMaterialVariableId : U8
{
	NONE = 0,
	TRANSFORM,
	PREVIOUS_TRANSFORM,
	ROTATION,
	GLOBAL_SAMPLER,

	COUNT,
	FIRST = 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BuiltinMaterialVariableId)

/// The ID of builtin mutators.
enum class BuiltinMutatorId : U8
{
	NONE = 0,
	INSTANCED,
	PASS,
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
		m_index = b.m_index;
		m_indexInBinary = b.m_indexInBinary;
		m_indexInBinary2ndElement = b.m_indexInBinary2ndElement;
		m_opaqueBinding = b.m_opaqueBinding;
		m_constant = b.m_constant;
		m_instanced = b.m_instanced;
		m_numericValueIsSet = b.m_numericValueIsSet;
		m_dataType = b.m_dataType;
		m_builtin = b.m_builtin;
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

	Bool isConstant() const
	{
		return m_constant;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

	Bool isBuildin() const
	{
		return m_builtin != BuiltinMaterialVariableId::NONE;
	}

	ShaderVariableDataType getDataType() const
	{
		ANKI_ASSERT(m_dataType != ShaderVariableDataType::NONE);
		return m_dataType;
	}

	/// Get the binding of a texture or a sampler type of material variable.
	U32 getOpaqueBinding() const
	{
		ANKI_ASSERT(m_opaqueBinding != MAX_U32 && (isTexture() || isSampler()));
		return m_opaqueBinding;
	}

protected:
	String m_name;
	U32 m_index = MAX_U32;
	U32 m_indexInBinary = MAX_U32;
	U32 m_indexInBinary2ndElement = MAX_U32; ///< To calculate the stride.
	U32 m_opaqueBinding = MAX_U32; ///< Binding for textures and samplers.
	Bool m_constant = false;
	Bool m_instanced = false;
	Bool m_numericValueIsSet = false; ///< The unamed union bellow is set
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;
	BuiltinMaterialVariableId m_builtin = BuiltinMaterialVariableId::NONE;

	/// Values for non-builtins
	/// @{
	union
	{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
	};

	ImageResourcePtr m_image;
	/// @}

	Bool valueSetByMaterial() const
	{
		return m_image.isCreated() || m_numericValueIsSet;
	}
};

// Specialize the MaterialVariable::getValue
#define ANKI_SPECIALIZE_GET_VALUE(t_, var_, shaderType_) \
	template<> \
	inline const t_& MaterialVariable::getValue<t_>() const \
	{ \
		ANKI_ASSERT(m_dataType == ShaderVariableDataType::shaderType_); \
		ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE); \
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

	/// Return true of the the variable is active.
	Bool isVariableActive(const MaterialVariable& var) const
	{
		return m_activeVars.get(var.m_index);
	}

	const ShaderProgramPtr& getShaderProgram() const
	{
		return m_prog;
	}

	U32 getPerDrawUniformBlockSize() const
	{
		return m_perDrawUboSize;
	}

	U32 getPerInstanceUniformBlockSize(U32 instanceCount) const
	{
		ANKI_ASSERT(instanceCount > 0 && instanceCount <= MAX_INSTANCE_COUNT);
		return m_perInstanceUboSizeSingleInstance * instanceCount;
	}

	ShaderVariableBlockInfo getBlockInfo(const MaterialVariable& var, U32 instanceCount) const
	{
		ANKI_ASSERT(isVariableActive(var));
		ANKI_ASSERT(var.inBlock());
		ANKI_ASSERT(m_blockInfos[var.m_index].m_offset >= 0);
		if(var.isInstanced())
		{
			ANKI_ASSERT(m_blockInfos[var.m_index].m_arraySize == I16(MAX_INSTANCE_COUNT));
			ANKI_ASSERT(instanceCount > 0 && instanceCount <= MAX_INSTANCE_COUNT);
		}
		else
		{
			ANKI_ASSERT(m_blockInfos[var.m_index].m_arraySize == 1);
			ANKI_ASSERT(instanceCount == 1);
		}

		ShaderVariableBlockInfo out = m_blockInfos[var.m_index];
		out.m_arraySize = I16(instanceCount);
		return out;
	}

	template<typename T>
	void writeShaderBlockMemory(const MaterialVariable& var, const T* elements, U32 elementCount, void* buffBegin,
								const void* buffEnd) const
	{
		ANKI_ASSERT(isVariableActive(var));
		ANKI_ASSERT(getShaderVariableTypeFromTypename<T>() == var.getDataType());
		const ShaderVariableBlockInfo blockInfo = getBlockInfo(var, elementCount);
		anki::writeShaderBlockMemory(var.getDataType(), blockInfo, elements, elementCount, buffBegin, buffEnd);
	}

private:
	ShaderProgramPtr m_prog;
	DynamicArray<ShaderVariableBlockInfo> m_blockInfos;
	BitSet<128, U32> m_activeVars = {false};
	U32 m_perDrawUboSize = 0;
	U32 m_perInstanceUboSizeSingleInstance = 0;
};

/// Material resource.
///
/// Material XML file format:
/// @code
/// <material [shadow="0 | 1"] [forwardShading="0 | 1"] shaderProgram="path">
///		[<mutation>
///			<mutator name="str" value="value"/>
///		</mutation>]
///
///		[<inputs>
///			<input shaderVar="name to shaderProg var" value="values"/> (1)
///		</inputs>]
/// </material>
///
/// [<rtMaterial>
///		<rayType shaderProgram="path" type="shadows|gi|reflections|pathTracing">
///			[<mutation>
///				<mutator name="str" value="value"/>
///			</mutation>]
///		</rayType>
///
/// 	[<inputs>
/// 		<input name="name" value="value" />
/// 	</inputs>]
/// </rtMaterial>]
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

	U32 getLodCount() const
	{
		return m_lodCount;
	}

	Bool castsShadow() const
	{
		return m_shadow;
	}

	Bool hasTessellation() const
	{
		return !!(m_prog->getStages() & (ShaderTypeBit::TESSELLATION_CONTROL | ShaderTypeBit::TESSELLATION_EVALUATION));
	}

	Bool isForwardShading() const
	{
		return m_forwardShading;
	}

	Bool isInstanced() const
	{
		return m_builtinMutators[BuiltinMutatorId::INSTANCED] != nullptr;
	}

	ConstWeakArray<MaterialVariable> getVariables() const
	{
		return m_vars;
	}

	U32 getDescriptorSetIndex() const
	{
		ANKI_ASSERT(m_descriptorSetIdx != MAX_U8);
		return m_descriptorSetIdx;
	}

	U32 getBoneTransformsStorageBlockBinding() const
	{
		ANKI_ASSERT(supportsSkinning());
		return m_boneTrfsBinding;
	}

	U32 getPrevFrameBoneTransformsStorageBlockBinding() const
	{
		ANKI_ASSERT(supportsSkinning());
		return m_prevFrameBoneTrfsBinding;
	}

	Bool supportsSkinning() const
	{
		ANKI_ASSERT((m_boneTrfsBinding == MAX_U32 && m_prevFrameBoneTrfsBinding == MAX_U32)
					|| (m_boneTrfsBinding != MAX_U32 && m_prevFrameBoneTrfsBinding != MAX_U32));
		return m_boneTrfsBinding != MAX_U32;
	}

	U32 getPerDrawUniformBlockBinding() const
	{
		ANKI_ASSERT(m_perDrawUboBinding != MAX_U32);
		return m_perDrawUboBinding;
	}

	U32 getPerInstanceUniformBlockBinding() const
	{
		ANKI_ASSERT(isInstanced() && m_perInstanceUboBinding != MAX_U32);
		return m_perInstanceUboBinding;
	}

	U32 getGlobalUniformsUniformBlockBinding() const
	{
		ANKI_ASSERT(m_globalUniformsUboBinding != MAX_U32);
		return m_globalUniformsUboBinding;
	}

	const MaterialVariant& getOrCreateVariant(const RenderingKey& key) const;

	U32 getShaderGroupHandleIndex(RayType type) const
	{
		ANKI_ASSERT(!!(m_rayTypes & RayTypeBit(1 << type)));
		ANKI_ASSERT(m_rtShaderGroupHandleIndices[type] < MAX_U32);
		return m_rtShaderGroupHandleIndices[type];
	}

	RayTypeBit getSupportedRayTracingTypes() const
	{
		return m_rayTypes;
	}

	const MaterialGpuDescriptor& getMaterialGpuDescriptor() const
	{
		return m_materialGpuDescriptor;
	}

	/// Get all texture views that the MaterialGpuDescriptor returned by getMaterialGpuDescriptor(), references. Used
	/// for lifetime management.
	ConstWeakArray<TextureViewPtr> getAllTextureViews() const
	{
		return ConstWeakArray<TextureViewPtr>((m_textureViewCount) ? &m_textureViews[0] : nullptr, m_textureViewCount);
	}

private:
	class SubMutation
	{
	public:
		const ShaderProgramResourceMutator* m_mutator;
		MutatorValue m_value;
	};

	ShaderProgramResourcePtr m_prog;

	Array<const ShaderProgramResourceMutator*, U32(BuiltinMutatorId::COUNT)> m_builtinMutators = {};

	Bool m_shadow = true;
	Bool m_forwardShading = false;
	U8 m_lodCount = 1;
	U8 m_descriptorSetIdx = MAX_U8; ///< The material set.
	U32 m_perDrawUboIdx = MAX_U32; ///< The b_perDraw UBO inside the binary.
	U32 m_perInstanceUboIdx = MAX_U32; ///< The b_perInstance UBO inside the binary.
	U32 m_perDrawUboBinding = MAX_U32;
	U32 m_perInstanceUboBinding = MAX_U32;
	U32 m_boneTrfsBinding = MAX_U32;
	U32 m_prevFrameBoneTrfsBinding = MAX_U32;
	U32 m_globalUniformsUboBinding = MAX_U32;

	/// Matrix of variants.
	mutable Array5d<MaterialVariant, U(Pass::COUNT), MAX_LOD_COUNT, 2, 2, 2> m_variantMatrix;
	mutable RWMutex m_variantMatrixMtx;

	DynamicArray<MaterialVariable> m_vars;

	DynamicArray<SubMutation> m_nonBuiltinsMutation;

	Array<ShaderProgramResourcePtr, U(RayType::COUNT)> m_rtPrograms;
	Array<U32, U(RayType::COUNT)> m_rtShaderGroupHandleIndices = {};

	MaterialGpuDescriptor m_materialGpuDescriptor;

	Array<ImageResourcePtr, U(TextureChannelId::COUNT)> m_images; ///< Keep the resources alive.
	Array<TextureViewPtr, U(TextureChannelId::COUNT)> m_textureViews; ///< Cache the GPU objects.
	U8 m_textureViewCount = 0;

	RayTypeBit m_rayTypes = RayTypeBit::NONE;

	ANKI_USE_RESULT Error createVars();

	static ANKI_USE_RESULT Error parseVariable(CString fullVarName, Bool instanced, U32& idx, CString& name);

	/// Parse whatever is inside the <inputs> tag.
	ANKI_USE_RESULT Error parseInputs(XmlElement inputsEl, Bool async);

	ANKI_USE_RESULT Error parseMutators(XmlElement mutatorsEl);
	ANKI_USE_RESULT Error findBuiltinMutators();

	void initVariant(const ShaderProgramResourceVariant& progVariant, MaterialVariant& variant, Bool instanced) const;

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

	ANKI_USE_RESULT Error parseRtMaterial(XmlElement rootEl);

	ANKI_USE_RESULT Error findGlobalUniformsUbo();
};
/// @}

} // end namespace anki
