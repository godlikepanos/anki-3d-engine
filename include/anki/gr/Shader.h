// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/Math.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Using an AnKi typename get the ShaderVariableDataType. Used for debugging.
template<typename T>
ShaderVariableDataType getShaderVariableTypeFromTypename();

#define ANKI_SPECIALIZE_SHADER_VAR_TYPE_GET(typename_, type_) \
	template<> \
	inline ShaderVariableDataType \
		getShaderVariableTypeFromTypename<typename_>() { \
		return ShaderVariableDataType::type_; \
	}

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

	/// Identifying the stride between columns of a column-major matrix or rows
	/// of a row-major matrix.
	I16 m_matrixStride = -1;
};

/// Populate the memory of a variable that is inside a shader block.
void writeShaderBlockMemory(
	ShaderVariableDataType type,
	const ShaderVariableBlockInfo& varBlkInfo,
	const void* elements,
	U32 elementsCount,
	void* buffBegin,
	const void* buffEnd);

/// GPU shader.
class Shader: public GrObject
{
public:
	/// Construct.
	Shader(GrManager* manager);

	/// Destroy.
	~Shader();

	/// Access the implementation.
	ShaderImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create shader.
	void create(ShaderType shaderType, const void* source, PtrSize sourceSize);

private:
	UniquePtr<ShaderImpl> m_impl;
};
/// @}

} // end namespace anki

