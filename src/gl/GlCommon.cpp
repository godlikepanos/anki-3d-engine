// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlCommon.h"

namespace anki {

//==============================================================================
ShaderType computeShaderTypeIndex(const GLenum glType)
{
	ShaderType idx = ShaderType::VERTEX;
	switch(glType)
	{
	case GL_VERTEX_SHADER:
		idx = ShaderType::VERTEX;
		break;
	case GL_TESS_CONTROL_SHADER:
		idx = ShaderType::TESSELLATION_CONTROL;
		break;
	case GL_TESS_EVALUATION_SHADER:
		idx = ShaderType::TESSELLATION_EVALUATION;
		break;
	case GL_GEOMETRY_SHADER:
		idx = ShaderType::GEOMETRY;
		break;
	case GL_FRAGMENT_SHADER:
		idx = ShaderType::FRAGMENT;
		break;
	case GL_COMPUTE_SHADER:
		idx = ShaderType::COMPUTE;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return idx;
}

//==============================================================================
GLenum computeGlShaderType(const ShaderType idx, GLbitfield* bit)
{
	static const Array<GLenum, 6> gltype = {{GL_VERTEX_SHADER, 
		GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}};

	static const Array<GLuint, 6> glbit = {{
		GL_VERTEX_SHADER_BIT, GL_TESS_CONTROL_SHADER_BIT, 
		GL_TESS_EVALUATION_SHADER_BIT, GL_GEOMETRY_SHADER_BIT,
		GL_FRAGMENT_SHADER_BIT, GL_COMPUTE_SHADER_BIT}};

	if(bit)
	{
		*bit = glbit[enumToType(idx)];
	}

	return gltype[enumToType(idx)];
}

//==============================================================================
template<typename T>
void writeShaderBlockMemorySanityChecks(
	const ShaderVariableBlockInfo& varBlkInfo,
	const void* elements, 
	U32 elementsCount,
	void* buffBegin,
	const void* buffEnd)
{
	// Check args
	ANKI_ASSERT(elements != nullptr);
	ANKI_ASSERT(elementsCount > 0);
	ANKI_ASSERT(buffBegin != nullptr);
	ANKI_ASSERT(buffEnd != nullptr);
	ANKI_ASSERT(buffBegin < buffEnd);
	
	// Check varBlkInfo
	ANKI_ASSERT(varBlkInfo.m_offset != -1);
	ANKI_ASSERT(varBlkInfo.m_arraySize > 0);

	if(varBlkInfo.m_arraySize > 1)
	{
		ANKI_ASSERT(varBlkInfo.m_arrayStride > 0);
	}

	// Check array size
	ANKI_ASSERT(static_cast<I16>(elementsCount) <= varBlkInfo.m_arraySize);
}

//==============================================================================
template<typename T>
static void writeShaderBlockMemorySimple(
	const ShaderVariableBlockInfo& varBlkInfo,
	const void* elements, 
	U32 elementsCount,
	void* buffBegin,
	const void* buffEnd)
{
	writeShaderBlockMemorySanityChecks<T>(varBlkInfo, elements, elementsCount,
		buffBegin, buffEnd);
	
	U8* buff = static_cast<U8*>(buffBegin) + varBlkInfo.m_offset;
	for(U i = 0; i < elementsCount; i++)
	{
		ANKI_ASSERT(buff + sizeof(T) <= static_cast<const U8*>(buffEnd));

		T* out = reinterpret_cast<T*>(buff);
		const T* in = reinterpret_cast<const T*>(elements) + i;
		*out = *in;

		buff += varBlkInfo.m_arrayStride;
	}
}

//==============================================================================
template<typename T, typename Vec>
static void writeShaderBlockMemoryMatrix(
	const ShaderVariableBlockInfo& varBlkInfo,
	const void* elements, 
	U32 elementsCount,
	void* buffBegin,
	const void* buffEnd)
{
	writeShaderBlockMemorySanityChecks<T>(varBlkInfo, elements, elementsCount,
		buffBegin, buffEnd);
	ANKI_ASSERT(varBlkInfo.m_matrixStride > 0);
	ANKI_ASSERT(varBlkInfo.m_matrixStride >= static_cast<I16>(sizeof(Vec)));

	U8* buff = static_cast<U8*>(buffBegin) + varBlkInfo.m_offset;
	for(U i = 0; i < elementsCount; i++)
	{
		U8* subbuff = buff;
		T matrix = reinterpret_cast<const T*>(elements)[i];
		matrix.transpose();
		for(U j = 0; j < sizeof(T) / sizeof(Vec); j++)
		{
			ANKI_ASSERT(
				(subbuff + sizeof(Vec)) <= static_cast<const U8*>(buffEnd));

			Vec* out = reinterpret_cast<Vec*>(subbuff);
			*out = matrix.getRow(j);
			subbuff += varBlkInfo.m_matrixStride;
		}
		buff += varBlkInfo.m_arrayStride;
	}
}

//==============================================================================
void writeShaderBlockMemory(
	ShaderVariableDataType type,
	const ShaderVariableBlockInfo& varBlkInfo,
	const void* elements, 
	U32 elementsCount,
	void* buffBegin,
	const void* buffEnd)
{
	switch(type)
	{
	case ShaderVariableDataType::FLOAT:
		writeShaderBlockMemorySimple<F32>(varBlkInfo, elements, elementsCount,
			buffBegin, buffEnd);
		break;
	case ShaderVariableDataType::VEC2:
		writeShaderBlockMemorySimple<Vec2>(varBlkInfo, elements, elementsCount,
			buffBegin, buffEnd);
		break;
	case ShaderVariableDataType::VEC3:
		writeShaderBlockMemorySimple<Vec3>(varBlkInfo, elements, elementsCount,
			buffBegin, buffEnd);
		break;
	case ShaderVariableDataType::VEC4:
		writeShaderBlockMemorySimple<Vec4>(varBlkInfo, elements, elementsCount,
			buffBegin, buffEnd);
		break;
	case ShaderVariableDataType::MAT3:
		writeShaderBlockMemoryMatrix<Mat3, Vec3>(varBlkInfo, elements, 
			elementsCount, buffBegin, buffEnd);
		break;
	case ShaderVariableDataType::MAT4:
		writeShaderBlockMemoryMatrix<Mat4, Vec4>(varBlkInfo, elements, 
			elementsCount, buffBegin, buffEnd);
		break;
	default:
		ANKI_ASSERT(0);
	}
}

} // end namespace anki
