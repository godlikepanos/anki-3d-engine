// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/resource/RenderingKey.h>
#include <anki/resource/ShaderProgramResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/Math.h>
#include <anki/util/Enum.h>
#include <anki/shaders/include/ModelTypes.h>

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
	MODEL_VIEW_PROJECTION_MATRIX,
	PREVIOUS_MODEL_VIEW_PROJECTION_MATRIX,
	MODEL_MATRIX,
	VIEW_MATRIX,
	PROJECTION_MATRIX,
	MODEL_VIEW_MATRIX,
	VIEW_PROJECTION_MATRIX,
	NORMAL_MATRIX,
	ROTATION_MATRIX,
	CAMERA_ROTATION_MATRIX,
	CAMERA_POSITION,
	GLOBAL_SAMPLER,

	COUNT,
	FIRST = 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BuiltinMaterialVariableId)

/// The ID of builtin mutators.
enum class BuiltinMutatorId : U8
{
	NONE = 0,
	INSTANCE_COUNT,
	PASS,
	LOD,
	BONES,
	VELOCITY,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BuiltinMutatorId)

/// Holds the shader variables. It's a container for shader program variables that share the same name.
class MaterialVariable : public NonCopyable
{
	friend class MaterialVariant;
	friend class MaterialResource;

public:
	MaterialVariable();

	MaterialVariable(MaterialVariable&& b)
	{
		*this = std::move(b);
	}

	~MaterialVariable();

	MaterialVariable& operator=(MaterialVariable&& b)
	{
		m_name = std::move(b.m_name);
		m_index = b.m_index;
		m_indexInBinary = b.m_indexInBinary;
		m_indexInBinary2ndElement = b.m_indexInBinary2ndElement;
		m_constant = b.m_constant;
		m_instanced = b.m_instanced;
		m_numericValueIsSet = b.m_numericValueIsSet;
		m_dataType = b.m_dataType;
		m_builtin = b.m_builtin;
		m_Mat4 = b.m_Mat4;
		m_tex = std::move(b.m_tex);
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

protected:
	String m_name;
	U32 m_index = MAX_U32;
	U32 m_indexInBinary = MAX_U32;
	U32 m_indexInBinary2ndElement = MAX_U32;
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
#include <anki/gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO
	};

	TextureResourcePtr m_tex;
	/// @}

	Bool valueSetByMaterial() const
	{
		return m_tex.isCreated() || m_numericValueIsSet;
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
#include <anki/gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

#undef ANKI_SPECIALIZE_GET_VALUE

template<>
inline const TextureResourcePtr& MaterialVariable::getValue() const
{
	ANKI_ASSERT(isTexture());
	ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE);
	return m_tex;
}

/// Material variant.
class MaterialVariant : public NonCopyable
{
	friend class MaterialResource;

public:
	/// Return true of the the variable is active.
	Bool isVariableActive(const MaterialVariable& var) const
	{
		return m_activeVars.get(var.m_index);
	}

	const ShaderProgramPtr& getShaderProgram() const
	{
		return m_prog;
	}

	U32 getUniformBlockSize() const
	{
		return m_uniBlockSize;
	}

	U32 getOpaqueBinding(const MaterialVariable& var) const
	{
		ANKI_ASSERT(m_opaqueBindings[var.m_index] >= 0);
		return U32(m_opaqueBindings[var.m_index]);
	}

	const ShaderVariableBlockInfo& getBlockInfo(const MaterialVariable& var) const
	{
		ANKI_ASSERT(isVariableActive(var));
		ANKI_ASSERT(var.inBlock());
		ANKI_ASSERT(m_blockInfos[var.m_index].m_offset >= 0);
		return m_blockInfos[var.m_index];
	}

	template<typename T>
	void writeShaderBlockMemory(const MaterialVariable& var, const T* elements, U32 elementsCount, void* buffBegin,
								const void* buffEnd) const
	{
		ANKI_ASSERT(isVariableActive(var));
		ANKI_ASSERT(getShaderVariableTypeFromTypename<T>() == var.getDataType());
		const ShaderVariableBlockInfo& blockInfo = getBlockInfo(var);
		anki::writeShaderBlockMemory(var.getDataType(), blockInfo, elements, elementsCount, buffBegin, buffEnd);
	}

private:
	ShaderProgramPtr m_prog;
	DynamicArray<ShaderVariableBlockInfo> m_blockInfos;
	DynamicArray<I16> m_opaqueBindings;
	BitSet<128, U32> m_activeVars = {false};
	U32 m_uniBlockSize = 0;
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
		return m_builtinMutators[BuiltinMutatorId::INSTANCE_COUNT] != nullptr;
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

	U32 getBoneTransformsBinding() const
	{
		return m_boneTrfsBinding;
	}

	U32 getPrevFrameBoneTransformsBinding() const
	{
		return m_prevFrameBoneTrfsBinding;
	}

	U32 getUniformsBinding() const
	{
		ANKI_ASSERT(m_uboBinding != MAX_U32);
		return m_uboBinding;
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
	U32 m_uboIdx = MAX_U32; ///< The b_ankiMaterial UBO inside the binary.
	U32 m_uboBinding = MAX_U32;
	U32 m_boneTrfsBinding = MAX_U32;
	U32 m_prevFrameBoneTrfsBinding = MAX_U32;

	/// Matrix of variants.
	mutable Array5d<MaterialVariant, U(Pass::COUNT), MAX_LOD_COUNT, MAX_INSTANCE_GROUPS, 2, 2> m_variantMatrix;
	mutable RWMutex m_variantMatrixMtx;

	DynamicArray<MaterialVariable> m_vars;

	DynamicArray<SubMutation> m_nonBuiltinsMutation;

	Array<ShaderProgramResourcePtr, U(RayType::COUNT)> m_rtPrograms;
	Array<U32, U(RayType::COUNT)> m_rtShaderGroupHandleIndices = {};

	MaterialGpuDescriptor m_materialGpuDescriptor;

	Array<TextureResourcePtr, TEXTURE_CHANNEL_COUNT> m_textureResources; ///< Keep the resources alive.
	Array<TextureViewPtr, TEXTURE_CHANNEL_COUNT> m_textureViews; ///< Cache the GPU objects.
	U8 m_textureViewCount = 0;

	RayTypeBit m_rayTypes = RayTypeBit::NONE;

	ANKI_USE_RESULT Error createVars();

	static ANKI_USE_RESULT Error parseVariable(CString fullVarName, Bool& instanced, U32& idx, CString& name);

	/// Parse whatever is inside the <inputs> tag.
	ANKI_USE_RESULT Error parseInputs(XmlElement inputsEl, Bool async);

	ANKI_USE_RESULT Error parseMutators(XmlElement mutatorsEl);
	ANKI_USE_RESULT Error findBuiltinMutators();

	static U32 getInstanceGroupIdx(U32 instanceCount);

	void initVariant(const ShaderProgramResourceVariant& progVariant, MaterialVariant& variant,
					 U32 instanceCount) const;

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

	Error parseRtMaterial(XmlElement rootEl);
};
/// @}

} // end namespace anki
