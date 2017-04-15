// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class Material;
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
	VIEW_PROJECTION_MATRIX,
	VIEW_MATRIX,
	NORMAL_MATRIX,
	CAMERA_ROTATION_MATRIX,
	COUNT
};

/// Holds the shader variables. Its a container for shader program variables that share the same name
class MaterialVariable : public NonCopyable
{
	friend class Material;
	friend class MaterialVariant;

public:
	MaterialVariable()
	{
		m_mat4 = Mat4::getZero();
	}

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
#define ANKI_SPECIALIZE_GET_VALUE(t_, var_, shaderType_)                                                               \
	template<>                                                                                                         \
	const t_& MaterialVariable::getValue<t_>() const                                                                   \
	{                                                                                                                  \
		ANKI_ASSERT(m_input);                                                                                          \
		ANKI_ASSERT(m_input->getShaderVariableDataType() == ShaderVariableDataType::shaderType_);                      \
		ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE);                                                     \
		return var_;                                                                                                   \
	}

ANKI_SPECIALIZE_GET_VALUE(F32, m_float, FLOAT)
ANKI_SPECIALIZE_GET_VALUE(Vec2, m_vec2, VEC2)
ANKI_SPECIALIZE_GET_VALUE(Vec3, m_vec3, VEC3)
ANKI_SPECIALIZE_GET_VALUE(Vec4, m_vec4, VEC4)
ANKI_SPECIALIZE_GET_VALUE(Mat3, m_mat3, MAT3)

template<>
const TextureResourcePtr& MaterialVariable::getValue() const
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
	friend class Material;
	friend class MaterialVariable;

public:
	MaterialVariant()
	{
	}

	~MaterialVariant()
	{
	}

	/// Return true of the the variable is active.
	Bool variableActive(const MaterialVariable& var) const
	{
		return m_variant->variableActive(*var.m_input);
	}

private:
	const ShaderProgramResourceVariant* m_variant = nullptr;
};

/// Material resource.
///
/// Material XML file format:
/// @code
/// <material>
/// 	[<levelsOfDetail>N</levelsOfDetail>]
/// 	[<shadow>0 | 1</shadow>]
/// 	[<forwardShading>0 | 1</forwardShading>]
///
///		<shaderProgram>path/to/shader_program.ankiprog</shaderProgram>
///
///		[<inputs>
///			<input>
///				<shaderInput>name</shaderInput>
///				[<value></value>] (1)
///				[<builtin></builtin>] (2)
///			</input>
///		</inputs>]
/// </material>
/// @endcode
/// (1): For non-builtins.
/// (2): For builtins.
class Material : public ResourceObject
{
	friend class MaterialVariable;
	friend class MaterialVariant;

public:
	Material(ResourceManager* manager);

	~Material();

	/// Load a material file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	U getLodCount() const
	{
		return m_lodCount;
	}

	Bool hasShadow() const
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

	const MaterialVariant& getVariant(const RenderingKey& key) const;

	const DynamicArray<MaterialVariable>& getVariables() const
	{
		return m_vars;
	}

private:
	ShaderProgramResourcePtr m_prog;

	Bool8 m_shadow = true;
	Bool8 m_forwardShading = false;
	U8 m_lodCount = 1;

	Array3d<MaterialVariant, U(Pass::COUNT), MAX_LODS, MAX_INSTANCE_GROUPS> m_variantMatrix; ///< Matrix of variants.

	DynamicArray<MaterialVariable> m_vars; ///< Non-const vars.
	DynamicArray<ShaderProgramResourceConstantValue> m_constValues;

	static U getInstanceGroupIdx(U instanceCount);

	/// Parse whatever is inside the <inputs> tag.
	ANKI_USE_RESULT Error parseInputs(XmlElement inputsEl);
};
/// @}

} // end namespace anki
