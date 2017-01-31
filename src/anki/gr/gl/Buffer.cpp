// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Buffer.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/BufferImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>

namespace anki
{

Buffer::Buffer(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Buffer::~Buffer()
{
}

class BufferCreateCommand final : public GlCommand
{
public:
	BufferPtr m_buff;
	PtrSize m_size;
	BufferUsageBit m_usage;
	BufferMapAccessBit m_access;

	BufferCreateCommand(Buffer* buff, PtrSize size, BufferUsageBit usage, BufferMapAccessBit access)
		: m_buff(buff)
		, m_size(size)
		, m_usage(usage)
		, m_access(access)
	{
	}

	Error operator()(GlState&)
	{
		BufferImpl& impl = *m_buff->m_impl;

		impl.init(m_size, m_usage, m_access);

		GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);

		(void)oldState;
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

		return ErrorCode::NONE;
	}
};

void Buffer::init(PtrSize size, BufferUsageBit usage, BufferMapAccessBit access)
{
	m_impl.reset(getAllocator().newInstance<BufferImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());

	cmdb->m_impl->pushBackNewCommand<BufferCreateCommand>(this, size, usage, access);
	cmdb->flush();
}

void* Buffer::map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
{
	// Wait for its creation
	if(m_impl->serializeRenderingThread())
	{
		return nullptr;
	}

	// Sanity checks
	ANKI_ASSERT(offset + range <= m_impl->m_size);
	ANKI_ASSERT(m_impl->m_persistentMapping);

	U8* ptr = static_cast<U8*>(m_impl->m_persistentMapping);
	ptr += offset;

#if ANKI_EXTRA_CHECKS
	ANKI_ASSERT(!m_impl->m_mapped);
	m_impl->m_mapped = true;
#endif

	return static_cast<void*>(ptr);
}

void Buffer::unmap()
{
#if ANKI_EXTRA_CHECKS
	ANKI_ASSERT(m_impl->m_mapped);
	m_impl->m_mapped = false;
#endif
}

} // end namespace anki
