// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_COMMON_H
#define ANKI_GR_COMMON_H

#include "anki/gr/Enums.h"
#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Assert.h"
#include "anki/util/Array.h"
#include "anki/Math.h"

#if ANKI_GL == ANKI_GL_DESKTOP
#	if ANKI_OS == ANKI_OS_WINDOWS && !defined(GLEW_STATIC)
#		define GLEW_STATIC
#	endif
#	include <GL/glew.h>
#	if !defined(ANKI_GLEW_H)
#		error "Wrong GLEW included"
#	endif
#elif ANKI_GL == ANKI_GL_ES
#	include <GLES3/gl3.h>
#else
#	error "See file"
#endif

namespace anki {

// Forward
class BufferImpl;
class BufferHandle;
class ShaderImpl;
class ShaderHandle;
class PipelineImpl;
class PipelineHandle;
struct PipelineInitializer;
class FramebufferImpl;
class FramebufferHandle;
class TextureImpl;
class TextureHandle;
class SamplerImpl;
class SamplerHandle;
class OcclusionQueryImpl;
class OcclusionQueryHandle;
class CommandBufferImpl;
class CommandBufferHandle;
class GrManager;
class GrManagerImpl;
struct FramebufferInitializer;
class TextureInitializer;
class SamplerInitializer;
struct GrManagerInitializer;

/// @addtogroup graphics_private
/// @{

/// The type of the allocator of CommandBuffer
template<typename T>
using CommandBufferAllocator = ChainAllocator<T>;

/// The type of the allocator for heap allocations
template<typename T>
using GrAllocator = HeapAllocator<T>;
/// @}

/// @addtogroup graphics
/// @{

// Some constants
const U MAX_VERTEX_ATTRIBUTES = 16;
const U MAX_COLOR_ATTACHMENTS = 4;
const U MAX_MIPMAPS = 16;
const U MAX_TEXTURE_LAYERS = 32;

/// GL generic callback
using SwapBuffersCallback = void(*)(void*);
using MakeCurrentCallback = void(*)(void*, void*);

/// Command buffer initialization hints. They are used to optimize the 
/// allocators of a command buffer
class CommandBufferInitHints
{
	friend class CommandBufferImpl;

private:
	enum
	{
		MAX_CHUNK_SIZE = 4 * 1024 * 1024 // 4MB
	};

	PtrSize m_chunkSize = 1024;
};
/// @}

/// @addtogroup opengl_containers
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
struct ShaderVariableBlockInfo
{
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
/// @}

/// @addtogroup opengl_other
/// @{

/// The draw indirect structure for index drawing, also the parameters of a 
/// regular drawcall
class GlDrawElementsIndirectInfo
{
public:
	GlDrawElementsIndirectInfo()
	{}

	GlDrawElementsIndirectInfo(
		U32 count, 
		U32 instanceCount, 
		U32 firstIndex, 
		U32 baseVertex, 
		U32 baseInstance)
	:	m_count(count), 
		m_instanceCount(instanceCount), 
		m_firstIndex(firstIndex), 
		m_baseVertex(baseVertex),
		m_baseInstance(baseInstance)
	{}

	U32 m_count = MAX_U32;
	U32 m_instanceCount = 1;
	U32 m_firstIndex = 0;
	U32 m_baseVertex = 0;
	U32 m_baseInstance = 0;
};

/// The draw indirect structure for arrays drawing, also the parameters of a 
/// regular drawcall
class GlDrawArraysIndirectInfo
{
public:
	GlDrawArraysIndirectInfo()
	{}

	GlDrawArraysIndirectInfo(
		U32 count, 
		U32 instanceCount, 
		U32 first, 
		U32 baseInstance)
	:	m_count(count), 
		m_instanceCount(instanceCount), 
		m_first(first), 
		m_baseInstance(baseInstance)
	{}

	U32 m_count = MAX_U32;
	U32 m_instanceCount = 1;
	U32 m_first = 0;
	U32 m_baseInstance = 0;	
};

/// A function that returns an index from a shader type GLenum
ShaderType computeShaderTypeIndex(const GLenum glType);
/// @}

} // end namespace anki

#endif
