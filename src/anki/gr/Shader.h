// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/Math.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Using an AnKi typename get the ShaderVariableDataType. Used for debugging.
template<typename T>
ShaderVariableDataType getShaderVariableTypeFromTypename();

#define ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(typename_, type_) \
	template<> \
	inline ShaderVariableDataType getShaderVariableTypeFromTypename<typename_>() \
	{ \
		return ShaderVariableDataType::type_; \
	}

ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(I32, INT)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(IVec2, IVEC2)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(IVec3, IVEC3)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(IVec4, IVEC4)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(U32, UINT)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(UVec2, UVEC2)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(UVec3, UVEC3)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(UVec4, UVEC4)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(F32, FLOAT)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(Vec2, VEC2)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(Vec3, VEC3)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(Vec4, VEC4)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(Mat3, MAT3)
ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(Mat4, MAT4)

#undef ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET

/// Shader block information.
class ShaderVariableBlockInfo
{
public:
	I16 m_offset = -1; ///< Offset inside the block

	I16 m_arraySize = -1; ///< Number of elements.

	/// Stride between the each array element if the variable is array.
	I16 m_arrayStride = -1;

	/// Identifying the stride between columns of a column-major matrix or rows of a row-major matrix.
	I16 m_matrixStride = -1;
};

/// Populate the memory of a variable that is inside a shader block.
void writeShaderBlockMemory(ShaderVariableDataType type,
	const ShaderVariableBlockInfo& varBlkInfo,
	const void* elements,
	U32 elementsCount,
	void* buffBegin,
	const void* buffEnd);

/// Specialization constant value.
class ShaderSpecializationConstValue
{
public:
	union
	{
		F32 m_float;
		I32 m_int;
	};

	ShaderVariableDataType m_dataType;

	ShaderSpecializationConstValue()
		: m_int(0)
		, m_dataType(ShaderVariableDataType::NONE)
	{
	}

	explicit ShaderSpecializationConstValue(F32 f)
		: m_float(f)
		, m_dataType(ShaderVariableDataType::FLOAT)
	{
	}

	explicit ShaderSpecializationConstValue(I32 i)
		: m_int(i)
		, m_dataType(ShaderVariableDataType::INT)
	{
	}

	ShaderSpecializationConstValue(const ShaderSpecializationConstValue&) = default;

	ShaderSpecializationConstValue& operator=(const ShaderSpecializationConstValue&) = default;
};

/// Shader init info.
class ShaderInitInfo : public GrBaseInitInfo
{
public:
	ShaderType m_shaderType = ShaderType::COUNT;
	ConstWeakArray<U8> m_binary = {};
	ConstWeakArray<ShaderSpecializationConstValue> m_constValues;

	ShaderInitInfo()
	{
	}

	ShaderInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	ShaderInitInfo(ShaderType type, ConstWeakArray<U8> bin, CString name = {})
		: GrBaseInitInfo(name)
		, m_shaderType(type)
		, m_binary(bin)
	{
	}
};

/// GPU shader.
class Shader : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::SHADER;

	ShaderType getShaderType() const
	{
		ANKI_ASSERT(m_shaderType != ShaderType::COUNT);
		return m_shaderType;
	}

protected:
	ShaderType m_shaderType = ShaderType::COUNT;

	/// Construct.
	Shader(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~Shader()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT Shader* newInstance(GrManager* manager, const ShaderInitInfo& init);
};
/// @}

} // end namespace anki
