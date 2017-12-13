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

Buffer* Buffer::newInstance(GrManager* manager, const BufferInitInfo& inf)
{
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
			BufferImpl& impl = static_cast<BufferImpl&>(*m_buff);

			impl.init(m_size, m_usage, m_access);

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);
			(void)oldState;
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

			return Error::NONE;
		}
	};

	BufferImpl* impl = manager->getAllocator().newInstance<BufferImpl>(manager);

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<BufferCreateCommand>(
		impl, inf.m_size, inf.m_usage, inf.m_access);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

void* Buffer::map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
{
	ANKI_GL_SELF(BufferImpl);
	return self.map(offset, range, access);
}

void Buffer::unmap()
{
	ANKI_GL_SELF(BufferImpl);
	self.unmap();
}

} // end namespace anki
