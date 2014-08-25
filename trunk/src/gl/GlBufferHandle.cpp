// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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

	void operator()(GlCommandBuffer*)
	{
		if(!m_empty)
		{
			GlBuffer newBuff(m_target, m_data.getSize(), 
				m_data.getBaseAddress(), m_flags);
			m_buff._get() = std::move(newBuff);
		}
		else
		{
			GlBuffer newBuff(m_target, m_size, nullptr, m_flags);
			m_buff._get() = std::move(newBuff);
		}

		GlHandleState oldState = m_buff._setState(GlHandleState::CREATED);
		(void)oldState;
		ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
	}
};

//==============================================================================
GlBufferHandle::GlBufferHandle()
{}

//==============================================================================
GlBufferHandle::GlBufferHandle(GlCommandBufferHandle& commands,
	GLenum target, GlClientBufferHandle& data, GLenum flags)
{
	ANKI_ASSERT(!isCreated());

	using Alloc = GlGlobalHeapAllocator<GlBuffer>;

	using DeleteCommand = 
		GlDeleteObjectCommand<GlBuffer, GlGlobalHeapAllocator<U8>>;

	using Deleter = GlHandleDeferredDeleter<GlBuffer, Alloc, DeleteCommand>;

	*static_cast<Base::Base*>(this) = Base::Base(
		&commands._getQueue().getManager(),
		commands._getQueue().getManager()._getAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	// Fire the command
	commands._pushBackNewCommand<GlBufferCreateCommand>(
		*this, target, data, flags);
}

//==============================================================================
GlBufferHandle::GlBufferHandle(GlCommandBufferHandle& commands,
	GLenum target, PtrSize size, GLenum flags)
{
	ANKI_ASSERT(!isCreated());

	using Alloc = GlGlobalHeapAllocator<GlBuffer>;

	using DeleteCommand = 
		GlDeleteObjectCommand<GlBuffer, GlGlobalHeapAllocator<U8>>;

	using Deleter = GlHandleDeferredDeleter<GlBuffer, Alloc, DeleteCommand>;

	*static_cast<Base::Base*>(this) = Base::Base(
		&commands._getQueue().getManager(),
		commands._getQueue().getManager()._getAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	// Fire the command
	commands._pushBackNewCommand<GlBufferCreateCommand>(
		*this, target, size, flags);
}

//==============================================================================
GlBufferHandle::~GlBufferHandle()
{}

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

		void operator()(GlCommandBuffer*)
		{
			ANKI_ASSERT(m_readOffset + m_size <= m_data.getSize());

			m_buff._get().write(
				(U8*)m_data.getBaseAddress() + m_readOffset, 
				m_writeOffset, 
				m_size);
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

		void operator()(GlCommandBuffer*)
		{
			U32 offset = (m_offset != -1) ? m_offset : 0;
			U32 size = (m_size != -1) ? m_size : m_buff._get().getSize();

			m_buff._get().setBindingRange(m_binding, offset, size);
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
		{}

		void operator()(GlCommandBuffer*)
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

		void operator()(GlCommandBuffer*)
		{
			GlBuffer& buff = m_buff._get();
			buff.setTarget(GL_ELEMENT_ARRAY_BUFFER);
			buff.bind();
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
PtrSize GlBufferHandle::getSize() const
{
	serializeOnGetter();
	return _get().getSize();
}

//==============================================================================
GLenum GlBufferHandle::getTarget() const
{
	serializeOnGetter();
	return _get().getTarget();
}

//==============================================================================
void* GlBufferHandle::getPersistentMappingAddress()
{
	serializeOnGetter();
	return _get().getPersistentMappingAddress();
}

} // end namespace anki
