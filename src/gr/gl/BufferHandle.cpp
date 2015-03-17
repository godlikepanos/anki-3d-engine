// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/BufferHandle.h"
#include "anki/gr/gl/DeferredDeleter.h"
#include "anki/gr/gl/BufferImpl.h"
#include "anki/gr/GrManager.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

//==============================================================================
/// Create buffer command
class BufferCreateCommand: public GlCommand
{
public:
	BufferHandle m_buff;
	GLenum m_target;
	const void* m_data;
	PtrSize m_size;
	GLbitfield m_flags;

	BufferCreateCommand(
		BufferHandle buff, GLenum target, const void* data, PtrSize size,
		GLenum flags)
	:	m_buff(buff), 
		m_target(target), 
		m_data(data), 
		m_size(size),
		m_flags(flags)
	{}

	Error operator()(CommandBufferImpl*) final
	{
		Error err = ErrorCode::NONE;

		err = m_buff.get().create(m_target, m_data, m_size, m_flags);

		GlObject::State oldState = m_buff.get().setStateAtomically(
			(err) ? GlObject::State::ERROR : GlObject::State::CREATED);

		(void)oldState;
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

		return err;
	}
};

//==============================================================================
class BufferWriteCommand: public GlCommand
{
public:
	BufferHandle m_buff;
	const void* m_data;
	PtrSize m_dataSize;
	PtrSize m_readOffset;
	PtrSize m_writeOffset;
	PtrSize m_size;

	BufferWriteCommand(BufferHandle& buff, const void* data, PtrSize dataSize,
		PtrSize readOffset, PtrSize writeOffset, PtrSize size)
	:	m_buff(buff), 
		m_data(data), 
		m_dataSize(dataSize),
		m_readOffset(readOffset), 
		m_writeOffset(writeOffset), 
		m_size(size)
	{}

	Error operator()(CommandBufferImpl*) final
	{
		ANKI_ASSERT(m_readOffset + m_size <= m_dataSize);

		m_buff.get().write(
			static_cast<const U8*>(m_data) + m_readOffset, 
			m_writeOffset, 
			m_size);

		return ErrorCode::NONE;
	}
};

//==============================================================================
class BufferBindVertexCommand: public GlCommand
{
public:
	BufferHandle m_buff;
	U32 m_elementSize;
	GLenum m_type;
	Bool8 m_normalized;
	U32 m_stride;
	U32 m_offset;
	U32 m_attribLocation;

	BufferBindVertexCommand(BufferHandle& buff, U32 elementSize, GLenum type, 
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
		BufferImpl& buff = m_buff.get();
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

//==============================================================================
class BindShaderBufferCommand: public GlCommand
{
public:
	BufferHandle m_buff;
	I32 m_offset;
	I32 m_size;
	U8 m_binding;

	BindShaderBufferCommand(BufferHandle& buff, 
		I32 offset, I32 size, U8 binding)
	:
		m_buff(buff), 
		m_offset(offset), 
		m_size(size), 
		m_binding(binding)
	{}

	Error operator()(CommandBufferImpl*) final
	{
		U32 offset = (m_offset != -1) ? m_offset : 0;
		U32 size = (m_size != -1) ? m_size : m_buff.get().getSize();

		m_buff.get().setBindingRange(m_binding, offset, size);

		return ErrorCode::NONE;
	}
};

//==============================================================================
class BindIndexBufferCommand: public GlCommand
{
public:
	BufferHandle m_buff;

	BindIndexBufferCommand(BufferHandle& buff)
	:	m_buff(buff)
	{}

	Error operator()(CommandBufferImpl*)
	{
		BufferImpl& buff = m_buff.get();
		buff.setTarget(GL_ELEMENT_ARRAY_BUFFER);
		buff.bind();

		return ErrorCode::NONE;
	}
};

//==============================================================================
// BufferHandle                                                                =
//==============================================================================

//==============================================================================
BufferHandle::BufferHandle()
{}

//==============================================================================
BufferHandle::~BufferHandle()
{}

//==============================================================================
Error BufferHandle::create(CommandBufferHandle& commands,
	GLenum target, const void* data, PtrSize size, GLenum flags)
{
	ANKI_ASSERT(!isCreated());

	using DeleteCommand = DeleteObjectCommand<BufferImpl>;

	using Deleter = DeferredDeleter<BufferImpl, DeleteCommand>;

	Error err = Base::create(commands.get().getManager(), Deleter());
	if(!err)
	{
		get().setStateAtomically(GlObject::State::TO_BE_CREATED);

		// Fire the command
		commands.get().pushBackNewCommand<BufferCreateCommand>(
			*this, target, data, size, flags);
	}

	return err;
}

//==============================================================================
void BufferHandle::write(CommandBufferHandle& commands, 
	const void* data, PtrSize dataSize, PtrSize readOffset, PtrSize writeOffset, 
	PtrSize size)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BufferWriteCommand>(
		*this, data, dataSize, readOffset, writeOffset, size);
}

//==============================================================================
void BufferHandle::bindShaderBufferInternal(CommandBufferHandle& commands,
	I32 offset, I32 size, U32 bindingPoint)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BindShaderBufferCommand>(
		*this, offset, size, bindingPoint);
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
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BufferBindVertexCommand>(
		*this, elementSize, type, normalized, stride, offset, attribLocation);
}

//==============================================================================
void BufferHandle::bindIndexBuffer(CommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BindIndexBufferCommand>(*this);
}

//==============================================================================
PtrSize BufferHandle::getSize() const
{
	return (get().serializeOnGetter()) ? 0 : get().getSize();
}

//==============================================================================
GLenum BufferHandle::getTarget() const
{
	return (get().serializeOnGetter()) ? GL_NONE : get().getTarget();
}

//==============================================================================
void* BufferHandle::getPersistentMappingAddress()
{
	return (get().serializeOnGetter()) 
		? nullptr : get().getPersistentMappingAddress();
}

} // end namespace anki
