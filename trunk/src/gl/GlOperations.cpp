// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlOperations.h"
#include "anki/gl/GlBuffer.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
// GlDrawcallElements                                                          =
//==============================================================================

//==============================================================================
class GlDrawElementsCommand: public GlCommand
{
public:
	GlDrawcallElements m_dc;

	GlDrawElementsCommand(const GlDrawcallElements& dc)
		: m_dc(dc)
	{}

	void operator()(GlCommandBuffer*)
	{
		m_dc.exec();
	}
};

//==============================================================================
void GlDrawcallElements::draw(GlCommandBufferHandle& commands)
{
	commands._pushBackNewCommand<GlDrawElementsCommand>(*this);
}

//==============================================================================
void GlDrawcallElements::exec()
{
	ANKI_ASSERT(m_indexSize != 0);
	ANKI_ASSERT(m_drawCount > 0);

	GLenum indicesType = 0;
	switch(m_indexSize)
	{
	case 1:
		indicesType = GL_UNSIGNED_BYTE;
		break;
	case 2:
		indicesType = GL_UNSIGNED_SHORT;
		break;
	case 4:
		indicesType = GL_UNSIGNED_INT;
		break;
	default:
		ANKI_ASSERT(0);
		break;
	};

	// Check if draw indirect
	if(m_drawCount == 1 && !m_indirectBuff.isCreated()
		&& !m_indirectClientBuff.isCreated())
	{
		glDrawElementsInstancedBaseVertexBaseInstance(
			m_primitiveType,
			m_noIndirect.m_count,
			indicesType,
			(const void*)(PtrSize)(m_noIndirect.m_firstIndex * m_indexSize),
			m_noIndirect.m_instanceCount,
			m_noIndirect.m_baseVertex,
			m_noIndirect.m_baseInstance);
	}
	else if(m_indirectBuff.isCreated())
	{
		GlBuffer& buff = m_indirectBuff._get();
		buff.setTarget(GL_DRAW_INDIRECT_BUFFER);
		buff.bind();

		glMultiDrawElementsIndirect(
			m_primitiveType,
			indicesType,
			(const void*)(PtrSize)m_indirect.m_offset,
			m_drawCount,
			m_indirect.m_stride);
	}
	else
	{
		ANKI_ASSERT(m_indirectClientBuff.isCreated());

		GlBuffer::bindDefault(GL_DRAW_INDIRECT_BUFFER);

		PtrSize ptr = 
			(PtrSize)m_indirectClientBuff.getBaseAddress() 
			+ m_indirect.m_offset;

		glMultiDrawElementsIndirect(
			m_primitiveType,
			indicesType,
			(const void*)ptr,
			m_drawCount,
			m_indirect.m_stride);
	}

	ANKI_COUNTER_INC(GL_DRAWCALLS_COUNT, (U64)1);
}

//==============================================================================
// GlDrawcallArrays                                                            =
//==============================================================================

//==============================================================================
class GlDrawArraysCommand: public GlCommand
{
public:
	GlDrawcallArrays m_dc;

	GlDrawArraysCommand(const GlDrawcallArrays& dc)
		: m_dc(dc)
	{}

	void operator()(GlCommandBuffer*)
	{
		m_dc.exec();
	}
};

//==============================================================================
void GlDrawcallArrays::draw(GlCommandBufferHandle& commands)
{
	commands._pushBackNewCommand<GlDrawArraysCommand>(*this);
}

//==============================================================================
void GlDrawcallArrays::exec()
{
	ANKI_ASSERT(m_drawCount > 0);

	if(m_drawCount == 1 && !m_indirectBuff.isCreated()
		&& !m_indirectClientBuff.isCreated())
	{
		glDrawArraysInstancedBaseInstance(
			m_primitiveType,
			m_noIndirect.m_first,
			m_noIndirect.m_count,
			m_noIndirect.m_instanceCount,
			m_noIndirect.m_baseInstance);
	}
	else if(m_indirectBuff.isCreated())
	{
		GlBuffer& buff = m_indirectBuff._get();
		buff.setTarget(GL_DRAW_INDIRECT_BUFFER);
		buff.bind();

		glMultiDrawArraysIndirect(
			m_primitiveType,
			(const void*)(PtrSize)m_offset,
			m_drawCount,
			m_stride);
	}
	else
	{
		ANKI_ASSERT(m_indirectClientBuff.isCreated());

		GlBuffer::bindDefault(GL_DRAW_INDIRECT_BUFFER);

		PtrSize ptr = 
			(PtrSize)m_indirectClientBuff.getBaseAddress() + m_offset;

		glMultiDrawArraysIndirect(
			m_primitiveType,
			(const void*)(PtrSize)ptr,
			m_drawCount,
			m_stride);
	}

	ANKI_COUNTER_INC(GL_DRAWCALLS_COUNT, (U64)1);
}

} // end namespace anki
