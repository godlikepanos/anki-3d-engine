// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Fence.h>
#include <anki/gr/gl/FenceImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Fence::Fence(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Fence::~Fence()
{
}

Bool Fence::clientWait(F64 seconds)
{
	if(m_impl->m_signaled.load())
	{
		return true;
	}

	if(seconds == 0.0)
	{
		return false;
	}

	class CheckFenceCommand final : public GlCommand
	{
	public:
		FenceImpl* m_fence; // Use the ptr to impl because there is a barrier that covers racing condition.
		F64 m_timeout;
		Barrier* m_barrier;

		CheckFenceCommand(FenceImpl* fence, F64 timeout, Barrier* barrier)
			: m_fence(fence)
			, m_timeout(timeout)
			, m_barrier(barrier)
		{
		}

		Error operator()(GlState&)
		{
			GLenum out = glClientWaitSync(m_fence->m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, m_timeout * 1e+9);

			if(out == GL_ALREADY_SIGNALED || out == GL_CONDITION_SATISFIED)
			{
				m_fence->m_signaled.store(true);
			}
			else if(out == GL_TIMEOUT_EXPIRED)
			{
				// Do nothing
			}
			else
			{
				ANKI_ASSERT(out == GL_WAIT_FAILED);
				return ErrorCode::FUNCTION_FAILED;
			}

			m_barrier->wait();

			return ErrorCode::NONE;
		}
	};

	Barrier barrier(2);

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());
	cmdb->m_impl->pushBackNewCommand<CheckFenceCommand>(m_impl.get(), seconds, &barrier);
	cmdb->flush();

	barrier.wait();

	return m_impl->m_signaled.load();
}

} // end namespace anki
