// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/resource/ShaderProgramResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/RenderingKey.h>
#include <anki/Math.h>
#include <anki/util/Visitor.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

// Forward
class XmlElement;
class MaterialResource;
template<typename T>
class MaterialVariableTemplate;
class MaterialVariable;
class MaterialVariant;

/// @addtogroup resource
/// @{

/// The ID of a buildin material variable
enum class BuiltinMaterialVariableId : U8
{
	NONE = 0,
	MODEL_VIEW_PROJECTION_MATRIX,
	MODEL_VIEW_MATRIX,
	MODEL_MATRIX,
	VIEW_PROJECTION_MATRIX,
	VIEW_MATRIX,
	NORMAL_MATRIX,
	ROTATION_MATRIX,
	CAMERA_ROTATION_MATRIX,
	CAMERA_POSITION,
	PREVIOUS_MODEL_VIEW_PROJECTION_MATRIX,
	COUNT
};

/// Holds the shader variables. Its a container for shader program variables that share the same name
class MaterialVariable : public NonCopyable
{
	friend class MaterialResource;
	friend class MaterialVariant;

public:
	MaterialVariable();

	~MaterialVariable();

	/// Get the builtin info.
	BuiltinMaterialVariableId getBuiltin() const
	{
		return m_builtin;
	}

	const ShaderProgramResourceInputVariable& getShaderProgramResourceInputVariable() const
	{
		ANKI_ASSERT(m_input);
		return *m_input;
	}

	template<typename T>
	const T& getValue() const;

protected:
	const ShaderProgramResourceInputVariable* m_input = nullptr;

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
		Mat3 m_mat3;
		Mat4 m_mat4;
	};

	TextureResourcePtr m_tex;

	BuiltinMaterialVariableId m_builtin = BuiltinMaterialVariableId::NONE;
};

// Specialize the MaterialVariable::getValue
#define ANKI_SPECIALIZE_GET_VALUE(t_, var_, shaderType_) \
	template<> \
	inline const t_& MaterialVariable::getValue<t_>() const \
	{ \
		ANKI_ASSERT(m_input); \
		ANKI_ASSERT(m_input->getShaderVariableDataType() == ShaderVariableDataType::shaderType_); \
		ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE); \
		return var_; \
	}

ANKI_SPECIALIZE_GET_VALUE(I32, m_int, INT)
ANKI_SPECIALIZE_GET_VALUE(IVec2, m_ivec2, IVEC2)
ANKI_SPECIALIZE_GET_VALUE(IVec3, m_ivec3, IVEC3)
ANKI_SPECIALIZE_GET_VALUE(IVec4, m_ivec4, IVEC4)
ANKI_SPECIALIZE_GET_VALUE(U32, m_uint, UINT)
ANKI_SPECIALIZE_GET_VALUE(UVec2, m_uvec2, UVEC2)
ANKI_SPECIALIZE_GET_VALUE(UVec3, m_uvec3, UVEC3)
ANKI_SPECIALIZE_GET_VALUE(UVec4, m_uvec4, UVEC4)
ANKI_SPECIALIZE_GET_VALUE(F32, m_float, FLOAT)
ANKI_SPECIALIZE_GET_VALUE(Vec2, m_vec2, VEC2)
ANKI_SPECIALIZE_GET_VALUE(Vec3, m_vec3, VEC3)
ANKI_SPECIALIZE_GET_VALUE(Vec4, m_vec4, VEC4)
ANKI_SPECIALIZE_GET_VALUE(Mat3, m_mat3, MAT3)
ANKI_SPECIALIZE_GET_VALUE(Mat4, m_mat4, MAT4)

template<>
inline const TextureResourcePtr& MaterialVariable::getValue() const
{
	ANKI_ASSERT(m_input);
	ANKI_ASSERT(m_input->getShaderVariableDataType() >= ShaderVariableDataType::SAMPLERS_FIRST
				&& m_input->getShaderVariableDataType() <= ShaderVariableDataType::SAMPLERS_LAST);
	ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE);
	return m_tex;
}

#undef ANKI_SPECIALIZE_GET_VALUE

/// Material variant.
class MaterialVariant : public NonCopyable
{
	friend class MaterialResource;
	friend class MaterialVariable;

public:
	MaterialVariant()
	{
	}

	~MaterialVariant()
	{
	}

	const ShaderProgramResourceVariant& getShaderProgramResourceVariant() const
	{
		ANKI_ASSERT(m_variant);
		return *m_variant;
	}

	/// Return true of the the variable is active.
	Bool variableActive(const MaterialVariable& var) const
	{
		ANKI_ASSERT(var.m_input);
		return m_variant->variableActive(*var.m_input);
	}

	const ShaderProgramPtr& getShaderProgram() const
	{
		return m_variant->getProgram();
	}

	U getUniformBlockSize() const
	{
		return m_variant->getUniformBlockSize();
	}

private:
	const ShaderProgramResourceVariant* m_variant = nullptr;
};

/// Material resource.
///
/// Material XML file format:
/// @code
/// <material
///		[shadow="0 | 1"]
///		[forwardShading="0 | 1"]
///		<shaderProgram="path"/>
///
///		[<mutators>
///			<mutator name="str" value="value"/>
///		</mutators>]
///
///		[<inputs>
///			<input
///				shaderInput="name to shaderProg input"
///				[value="values"] (1)
///				[builtin="name"]/> (2)
///		</inputs>]
/// </material>
/// @endcode
/// (1): For non-builtins.
/// (2): For builtins.
class MaterialResource : public ResourceObject
{
	friend class MaterialVariable;
	friend class MaterialVariant;

public:
	MaterialResource(ResourceManager* manager);

	~MaterialResource();

	/// Load a material file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	U getLodCount() const
	{
		return m_lodCount;
	}

	Bool castsShadow() const
	{
		return m_shadow;
	}

	Bool hasTessellation() const
	{
		return m_prog->hasTessellation();
	}

	Bool isForwardShading() const
	{
		return m_forwardShading;
	}

	Bool isInstanced() const
	{
		return m_prog->isInstanced();
	}

	const MaterialVariant& getOrCreateVariant(const RenderingKey& key) const;

	const DynamicArray<MaterialVariable>& getVariables() const
	{
		return m_vars;
	}

	ShaderProgramResourcePtr getShaderProgramResource() const
	{
		return m_prog;
	}

	U getDescriptorSetIndex() const
	{
		return m_descriptorSetIdx;
	}

private:
	ShaderProgramResourcePtr m_prog;

	Bool8 m_shadow = true;
	Bool8 m_forwardShading = false;
	U8 m_lodCount = 1;
	U8 m_descriptorSetIdx = 0; ///< Cache the value from the m_prog;

	const ShaderProgramResourceMutator* m_lodMutator = nullptr;
	const ShaderProgramResourceMutator* m_passMutator = nullptr;
	const ShaderProgramResourceMutator* m_instanceMutator = nullptr;
	const ShaderProgramResourceMutator* m_bonesMutator = nullptr;
	const ShaderProgramResourceMutator* m_velocityMutator = nullptr;

	DynamicArray<ShaderProgramResourceMutation> m_mutations;

	/// Matrix of variants.
	mutable Array5d<MaterialVariant, U(Pass::COUNT), MAX_LOD_COUNT, MAX_INSTANCE_GROUPS, 2, 2> m_variantMatrix;
	mutable SpinLock m_variantMatrixMtx;

	DynamicArray<MaterialVariable> m_vars; ///< Non-const vars.
	DynamicArray<ShaderProgramResourceConstantValue> m_constValues;

	static U getInstanceGroupIdx(U instanceCount);

	/// Parse whatever is inside the <inputs> tag.
	ANKI_USE_RESULT Error parseInputs(XmlElement inputsEl, Bool async);

	ANKI_USE_RESULT Error parseMutators(XmlElement mutatorsEl);
};
/// @}

} // end namespace anki
