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
class BufferPtr;
class ShaderImpl;
class ShaderPtr;
class PipelineImpl;
class PipelinePtr;
class PipelineInitializer;
class FramebufferImpl;
class FramebufferPtr;
class FramebufferInitializer;
class TextureImpl;
class TexturePtr;
class SamplerImpl;
class SamplerPtr;
class OcclusionQueryImpl;
class OcclusionQueryPtr;
class CommandBufferImpl;
class CommandBufferPtr;
class ResourceGroupImpl;
class ResourceGroupPtr;
class GrManager;
class GrManagerImpl;
class TextureInitializer;
class SamplerInitializer;
class GrManagerInitializer;

/// @addtogroup graphics
/// @{

/// The type of the allocator of CommandBuffer
template<typename T>
using CommandBufferAllocator = ChainAllocator<T>;

/// The type of the allocator for heap allocations
template<typename T>
using GrAllocator = HeapAllocator<T>;

// Some constants
const U MAX_VERTEX_ATTRIBUTES = 8;
const U MAX_COLOR_ATTACHMENTS = 4;
const U MAX_MIPMAPS = 16;
const U MAX_TEXTURE_LAYERS = 32;
const U MAX_TEXTURE_BINDINGS = 8;
const U MAX_UNIFORM_BUFFER_BINDINGS = 4;
const U MAX_STORAGE_BUFFER_BINDINGS = 4;

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
/// @}

} // end namespace anki

#endif
