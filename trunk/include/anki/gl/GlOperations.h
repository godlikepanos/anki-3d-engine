// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_OPERATIONS_H
#define ANKI_GL_GL_OPERATIONS_H

#include "anki/gl/GlCommon.h"
#include "anki/gl/GlBufferHandle.h"
#include "anki/gl/GlClientBufferHandle.h"

namespace anki {

/// @addtogroup opengl_other
/// @{

/// A GL drawcall for elements
class GlDrawcallElements
{
	friend class GlDrawElementsJob;

public:
	/// Indirect structure
	class Indirect
	{
	public:
		Indirect()
		{}

		Indirect(U32 count, U32 instanceCount, U32 firstIndex, 
			U32 baseVertex, U32 baseInstance)
			:	m_count(count), m_instanceCount(instanceCount), 
				m_firstIndex(firstIndex), m_baseVertex(baseVertex),
				m_baseInstance(baseInstance)
		{}

		U32 m_count;
		U32 m_instanceCount = 1;
		U32 m_firstIndex = 0;
		U32 m_baseVertex = 0;
		U32 m_baseInstance = 0;
	};

	/// Default
	GlDrawcallElements()
		: m_drawCount(0)
	{}

	/// Initialize a glDrawElementsInstancedBaseVertexBaseInstance call
	GlDrawcallElements(GLenum mode, U8 indexSize, U32 count, 
		U32 instanceCount = 1, U32 firstIndex = 0, U32 baseVertex = 0,
		U32 baseInstance = 0)
		:	m_primitiveType(mode),
			m_indexSize(indexSize),
			m_drawCount(1)
	{
		m_noIndirect.m_count = count;
		m_noIndirect.m_instanceCount = instanceCount;
		m_noIndirect.m_firstIndex = firstIndex;
		m_noIndirect.m_baseVertex = baseVertex;
		m_noIndirect.m_baseInstance = baseInstance;
	}

	/// Initialize a glMultiDrawElementsIndirect call with server buffer
	GlDrawcallElements(GLenum mode, U8 indexSize, 
		const GlBufferHandle& indirectBuff, U32 drawCount, 
		PtrSize offset, PtrSize stride)
		:	m_primitiveType(mode),
			m_indexSize(indexSize),
			m_drawCount(drawCount),
			m_indirectBuff(indirectBuff)
	{
		m_indirect.m_offset = offset;
		m_indirect.m_stride = stride;
	}

	/// Initialize a glMultiDrawElementsIndirect call with client buffer
	GlDrawcallElements(GLenum mode, U8 indexSize, 
		const GlClientBufferHandle& indirectBuff, U32 drawCount, 
		PtrSize offset, PtrSize stride)
		:	m_primitiveType(mode),
			m_indexSize(indexSize),
			m_drawCount(drawCount),
			m_indirectClientBuff(indirectBuff)
	{
		m_indirect.m_offset = offset;
		m_indirect.m_stride = stride;
	}

	Bool isCreated() const
	{
		return m_drawCount > 0;
	}

	/// Execute the drawcall
	void draw(GlJobChainHandle& jobs);

private:
	class IndirectView
	{
	public:
		/// Indirect buffer offset
		U32 m_offset;
	
		/// Indirect buffer stride
		U32 m_stride;
	};

	/// The GL primitive type (eg GL_POINTS). Need to set it
	GLenum m_primitiveType;
	
	/// Type of the indices
	U8 m_indexSize;

	/// Used in glMultiDrawElementsXXX
	U8 m_drawCount;

	/// Indirect buffer
	GlBufferHandle m_indirectBuff;

	/// Indirect client buffer
	GlClientBufferHandle m_indirectClientBuff;

	union
	{
		Indirect m_noIndirect;
		IndirectView m_indirect;
	};

	/// Execute the job. Called by the server
	void exec();
};

/// A GL drawcall for arrays
class GlDrawcallArrays
{
	friend class GlDrawArraysJob;

public:
	/// Indirect structure
	class Indirect
	{
	public:
		Indirect()
		{}

		Indirect(U32 count, U32 instanceCount, U32 first, U32 baseInstance)
			:	m_count(count), 
				m_instanceCount(instanceCount), 
				m_first(first), 
				m_baseInstance(baseInstance)
		{}

		U32 m_count;
		U32 m_instanceCount = 1;
		U32 m_first = 0;
		U32 m_baseInstance = 0;	
	};

	/// Default
	GlDrawcallArrays()
		: m_drawCount(0)
	{}

	/// Initialize a glDrawArraysInstancedBaseInstance call
	GlDrawcallArrays(GLenum mode, U32 count, U32 instanceCount = 1, 
		U32 first = 0, U32 baseInstance = 0)
		:	m_primitiveType(mode),
			m_noIndirect{count, instanceCount, first, baseInstance},
			m_drawCount(1)
	{}
	
	/// Initialize a glMultiDrawArraysIndirect call with server buffer
	GlDrawcallArrays(GLenum mode, const GlBufferHandle& indirectBuff,
		U32 drawCount, PtrSize offset, PtrSize stride)
		:	m_primitiveType(mode),
			m_drawCount(drawCount),
			m_indirectBuff(indirectBuff),
			m_offset(offset),
			m_stride(stride)
	{}

	/// Initialize a glMultiDrawArraysIndirect call with client buffer
	GlDrawcallArrays(GLenum mode, const GlClientBufferHandle& indirectBuff,
		U32 drawCount, PtrSize offset, PtrSize stride)
		:	m_primitiveType(mode),
			m_drawCount(drawCount),
			m_indirectClientBuff(indirectBuff),
			m_offset(offset),
			m_stride(stride)
	{}

	Bool isCreated() const
	{
		return m_drawCount > 0;
	}

	/// Execute the drawcall
	void draw(GlJobChainHandle& jobs);

private:
	/// The GL primitive type (eg GL_POINTS). Need to set it
	GLenum m_primitiveType;

	/// No indirect data
	Indirect m_noIndirect;

	/// Used in glMultiDrawArraysXXX
	U8 m_drawCount;

	/// Indirect buffer
	GlBufferHandle m_indirectBuff;

	/// Indirect client buffer
	GlClientBufferHandle m_indirectClientBuff;

	/// Indirect buffer offset
	U32 m_offset;
	
	/// Indirect buffer stride
	U32 m_stride;

	/// Execute the job. Called by the server
	void exec();
};

/// @}

} // end namespace anki

#endif
