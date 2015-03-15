// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/BufferHandle.h"
#include "anki/gr/GlHandleDeferredDeleter.h"
#include "anki/gr/ClientBufferHandle.h"
#include "anki/gr/gl/BufferImpl.h"
#include "anki/gr/GlDevice.h"

namespace anki {

//==============================================================================
/// Create buffer command
class GlBufferCreateCommand: public GlCommand
{
public:
	BufferHandle m_buff;
	GLenum m_target;
	ClientBufferHandle m_data;
	PtrSize m_size;
	GLbitfield m_flags;
	Bool8 m_empty;

	GlBufferCreateCommand(
		BufferHandle buff, GLenum target, ClientBufferHandle data, 
		GLenum flags)
	:	m_buff(buff), 
		m_target(target), 
		m_data(data), 
		m_flags(flags), 
		m_empty(false)
	{}

	GlBufferCreateCommand(
		BufferHandle buff, GLenum target, PtrSize size, GLenum flags)
	:	m_buff(buff), 
		m_target(target), 
		m_size(size), 
		m_flags(flags), 
		m_empty(true)
	{}

	Error operator()(CommandBufferImpl*)
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
BufferHandle::BufferHandle()
{}

//==============================================================================
BufferHandle::~BufferHandle()
{}

//==============================================================================
Error BufferHandle::create(CommandBufferHandle& commands,
	GLenum target, ClientBufferHandle& data, GLenum flags)
{
	ANKI_ASSERT(!isCreated());

	using Alloc = GlAllocator<BufferImpl>;

	using DeleteCommand = 
		GlDeleteObjectCommand<BufferImpl, GlAllocator<U8>>;

	using Deleter = GlHandleDeferredDeleter<BufferImpl, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._getRenderingThread().getDevice(),
		commands._getRenderingThread().getDevice()._getAllocator(), 
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
Error BufferHandle::create(CommandBufferHandle& commands,
	GLenum target, PtrSize size, GLenum flags)
{
	ANKI_ASSERT(!isCreated());

	using Alloc = GlAllocator<BufferImpl>;

	using DeleteCommand = 
		GlDeleteObjectCommand<BufferImpl, GlAllocator<U8>>;

	using Deleter = GlHandleDeferredDeleter<BufferImpl, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._getRenderingThread().getDevice(),
		commands._getRenderingThread().getDevice()._getAllocator(), 
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
void BufferHandle::write(CommandBufferHandle& commands, 
	ClientBufferHandle& data, PtrSize readOffset, PtrSize writeOffset, 
	PtrSize size)
{
	class Command: public GlCommand
	{
	public:
		BufferHandle m_buff;
		ClientBufferHandle m_data;
		PtrSize m_readOffset;
		PtrSize m_writeOffset;
		PtrSize m_size;

		Command(BufferHandle& buff, ClientBufferHandle& data, 
			PtrSize readOffset, PtrSize writeOffset, PtrSize size)
			:	m_buff(buff), m_data(data), m_readOffset(readOffset), 
				m_writeOffset(writeOffset), m_size(size)
		{}

		Error operator()(CommandBufferImpl*)
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
void BufferHandle::bindShaderBufferInternal(CommandBufferHandle& commands,
	I32 offset, I32 size, U32 bindingPoint)
{
	class Command: public GlCommand
	{
	public:
		BufferHandle m_buff;
		I32 m_offset;
		I32 m_size;
		U8 m_binding;

		Command(BufferHandle& buff, I32 offset, I32 size, U8 binding)
			: m_buff(buff), m_offset(offset), m_size(size), m_binding(binding)
		{}

		Error operator()(CommandBufferImpl*)
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
void BufferHandle::bindVertexBuffer(
	CommandBufferHandle& commands, 
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
		BufferHandle m_buff;
		U32 m_elementSize;
		GLenum m_type;
		Bool8 m_normalized;
		U32 m_stride;
		U32 m_offset;
		U32 m_attribLocation;

		Command(BufferHandle& buff, U32 elementSize, GLenum type, 
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

		Error operator()(CommandBufferImpl*)
		{
			BufferImpl& buff = m_buff._get();
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
void BufferHandle::bindIndexBuffer(CommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		BufferHandle m_buff;

		Command(BufferHandle& buff)
		:	m_buff(buff)
		{}

		Error operator()(CommandBufferImpl*)
		{
			BufferImpl& buff = m_buff._get();
			buff.setTarget(GL_ELEMENT_ARRAY_BUFFER);
			buff.bind();

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
PtrSize BufferHandle::getSize() const
{
	return (serializeOnGetter()) ? 0 : _get().getSize();
}

//==============================================================================
GLenum BufferHandle::getTarget() const
{
	return (serializeOnGetter()) ? GL_NONE : _get().getTarget();
}

//==============================================================================
void* BufferHandle::getPersistentMappingAddress()
{
	return 
		(serializeOnGetter()) ? nullptr : _get().getPersistentMappingAddress();
}

} // end namespace anki
