// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlBufferHandle.h"
#include "anki/gl/GlHandleDeferredDeleter.h"
#include "anki/gl/GlClientBufferHandle.h"
#include "anki/gl/GlBuffer.h"
#include "anki/gl/GlDevice.h"

namespace anki {

//==============================================================================
/// Create buffer command
class GlBufferCreateCommand: public GlCommand
{
public:
	GlBufferHandle m_buff;
	GLenum m_target;
	GlClientBufferHandle m_data;
	PtrSize m_size;
	GLbitfield m_flags;
	Bool8 m_empty;

	GlBufferCreateCommand(
		GlBufferHandle buff, GLenum target, GlClientBufferHandle data, 
		GLenum flags)
	:	m_buff(buff), 
		m_target(target), 
		m_data(data), 
		m_flags(flags), 
		m_empty(false)
	{}

	GlBufferCreateCommand(
		GlBufferHandle buff, GLenum target, PtrSize size, GLenum flags)
	:	m_buff(buff), 
		m_target(target), 
		m_size(size), 
		m_flags(flags), 
		m_empty(true)
	{}

	Error operator()(GlCommandBuffer*)
	{
		Error err = ErrorCode::NONE;

		if(!m_empty)
		{
			err = m_buff._get().create(m_target, m_data.getSize(), 
				m_data.getBaseAddress(), m_flags);
		}
		else
		{
			err = m_buff._get().create(m_target, m_size, nullptr, m_flags);
		}

		GlHandleState oldState = m_buff._setState(
			(err) ? GlHandleState::ERROR : GlHandleState::CREATED);

		(void)oldState;
		ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);

		return err;
	}
};

//==============================================================================
GlBufferHandle::GlBufferHandle()
{}

//==============================================================================
GlBufferHandle::~GlBufferHandle()
{}

//==============================================================================
Error GlBufferHandle::create(GlCommandBufferHandle& commands,
	GLenum target, GlClientBufferHandle& data, GLenum flags)
{
	ANKI_ASSERT(!isCreated());

	using Alloc = GlAllocator<GlBuffer>;

	using DeleteCommand = 
		GlDeleteObjectCommand<GlBuffer, GlAllocator<U8>>;

	using Deleter = GlHandleDeferredDeleter<GlBuffer, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._getQueue().getDevice(),
		commands._getQueue().getDevice()._getAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		// Fire the command
		commands._pushBackNewCommand<GlBufferCreateCommand>(
			*this, target, data, flags);
	}

	return err;
}

//==============================================================================
Error GlBufferHandle::create(GlCommandBufferHandle& commands,
	GLenum target, PtrSize size, GLenum flags)
{
	ANKI_ASSERT(!isCreated());

	using Alloc = GlAllocator<GlBuffer>;

	using DeleteCommand = 
		GlDeleteObjectCommand<GlBuffer, GlAllocator<U8>>;

	using Deleter = GlHandleDeferredDeleter<GlBuffer, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._getQueue().getDevice(),
		commands._getQueue().getDevice()._getAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);
		
		// Fire the command
		commands._pushBackNewCommand<GlBufferCreateCommand>(
			*this, target, size, flags);
	}

	return err;
}

//==============================================================================
void GlBufferHandle::write(GlCommandBufferHandle& commands, 
	GlClientBufferHandle& data, PtrSize readOffset, PtrSize writeOffset, 
	PtrSize size)
{
	class Command: public GlCommand
	{
	public:
		GlBufferHandle m_buff;
		GlClientBufferHandle m_data;
		PtrSize m_readOffset;
		PtrSize m_writeOffset;
		PtrSize m_size;

		Command(GlBufferHandle& buff, GlClientBufferHandle& data, 
			PtrSize readOffset, PtrSize writeOffset, PtrSize size)
			:	m_buff(buff), m_data(data), m_readOffset(readOffset), 
				m_writeOffset(writeOffset), m_size(size)
		{}

		Error operator()(GlCommandBuffer*)
		{
			ANKI_ASSERT(m_readOffset + m_size <= m_data.getSize());

			m_buff._get().write(
				(U8*)m_data.getBaseAddress() + m_readOffset, 
				m_writeOffset, 
				m_size);

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(
		*this, data, readOffset, writeOffset, size);
}

//==============================================================================
void GlBufferHandle::bindShaderBufferInternal(GlCommandBufferHandle& commands,
	I32 offset, I32 size, U32 bindingPoint)
{
	class Command: public GlCommand
	{
	public:
		GlBufferHandle m_buff;
		I32 m_offset;
		I32 m_size;
		U8 m_binding;

		Command(GlBufferHandle& buff, I32 offset, I32 size, U8 binding)
			: m_buff(buff), m_offset(offset), m_size(size), m_binding(binding)
		{}

		Error operator()(GlCommandBuffer*)
		{
			U32 offset = (m_offset != -1) ? m_offset : 0;
			U32 size = (m_size != -1) ? m_size : m_buff._get().getSize();

			m_buff._get().setBindingRange(m_binding, offset, size);

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, offset, size, bindingPoint);
}

//==============================================================================
void GlBufferHandle::bindVertexBuffer(
	GlCommandBufferHandle& commands, 
	U32 elementSize,
	GLenum type,
	Bool normalized,
	PtrSize stride,
	PtrSize offset,
	U32 attribLocation)
{
	class Command: public GlCommand
	{
	public:
		GlBufferHandle m_buff;
		U32 m_elementSize;
		GLenum m_type;
		Bool8 m_normalized;
		U32 m_stride;
		U32 m_offset;
		U32 m_attribLocation;

		Command(GlBufferHandle& buff, U32 elementSize, GLenum type, 
			Bool8 normalized, U32 stride, U32 offset, U32 attribLocation)
		:	m_buff(buff), 
			m_elementSize(elementSize), 
			m_type(type), 
			m_normalized(normalized), 
			m_stride(stride),
			m_offset(offset),
			m_attribLocation(attribLocation)
		{
			ANKI_ASSERT(m_elementSize != 0);
		}

		Error operator()(GlCommandBuffer*)
		{
			GlBuffer& buff = m_buff._get();
			ANKI_ASSERT(m_offset < m_buff.getSize());
			
			buff.setTarget(GL_ARRAY_BUFFER);
			buff.bind();

			glEnableVertexAttribArray(m_attribLocation);
			glVertexAttribPointer(
				m_attribLocation, 
				m_elementSize, 
				m_type, 
				m_normalized,
				m_stride, 
				reinterpret_cast<const GLvoid*>(m_offset));

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, elementSize, type, 
		normalized, stride, offset, attribLocation);
}

//==============================================================================
void GlBufferHandle::bindIndexBuffer(GlCommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		GlBufferHandle m_buff;

		Command(GlBufferHandle& buff)
		:	m_buff(buff)
		{}

		Error operator()(GlCommandBuffer*)
		{
			GlBuffer& buff = m_buff._get();
			buff.setTarget(GL_ELEMENT_ARRAY_BUFFER);
			buff.bind();

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
PtrSize GlBufferHandle::getSize() const
{
	return (serializeOnGetter()) ? 0 : _get().getSize();
}

//==============================================================================
GLenum GlBufferHandle::getTarget() const
{
	return (serializeOnGetter()) ? GL_NONE : _get().getTarget();
}

//==============================================================================
void* GlBufferHandle::getPersistentMappingAddress()
{
	return 
		(serializeOnGetter()) ? nullptr : _get().getPersistentMappingAddress();
}

} // end namespace anki
