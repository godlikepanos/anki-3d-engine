// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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

		BufferCreateCommand(Buffer* buff)
			: m_buff(buff)
		{
		}

		Error operator()(GlState&)
		{
			BufferImpl& impl = static_cast<BufferImpl&>(*m_buff);

			impl.init();

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);
			(void)oldState;
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

			return Error::NONE;
		}
	};

	BufferImpl* impl = manager->getAllocator().newInstance<BufferImpl>(manager, inf.getName());
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	impl->preInit(inf);

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<BufferCreateCommand>(impl);
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
