// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Fence.h>
#include <anki/gr/gl/FenceImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/util/HighRezTimer.h>

namespace anki
{

Bool Fence::clientWait(Second seconds)
{
	if(static_cast<FenceImpl&>(*this).m_signaled.load())
	{
		return true;
	}

	class CheckFenceCommand final : public GlCommand
	{
	public:
		FencePtr m_fence;
		Second m_timeout;
		Second m_flushTime;
		Barrier* m_barrier;

		CheckFenceCommand(FencePtr fence, Second timeout, Second flushTime, Barrier* barrier)
			: m_fence(fence)
			, m_timeout(timeout)
			, m_flushTime(flushTime)
			, m_barrier(barrier)
		{
		}

		Error operator()(GlState&)
		{
			// Since there is a delay between flushing the cmdb and returning this result try to adjust the time we
			// wait
			Second timeToWait;
			if(m_timeout != 0.0)
			{
				timeToWait = m_timeout - (HighRezTimer::getCurrentTime() - m_flushTime);
				timeToWait = max(0.0, timeToWait);
			}
			else
			{
				timeToWait = 0.0;
			}

			FenceImpl& impl = static_cast<FenceImpl&>(*m_fence);
			GLenum out = glClientWaitSync(impl.m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, timeToWait * 1e+9);

			if(out == GL_ALREADY_SIGNALED || out == GL_CONDITION_SATISFIED)
			{
				impl.m_signaled.store(true);
			}
			else if(out == GL_TIMEOUT_EXPIRED)
			{
				// Do nothing
			}
			else
			{
				ANKI_ASSERT(out == GL_WAIT_FAILED);
				return Error::FUNCTION_FAILED;
			}

			if(m_barrier)
			{
				m_barrier->wait();
			}

			return Error::NONE;
		}
	};

	CommandBufferPtr cmdb = getManager().newCommandBuffer(CommandBufferInitInfo());

	if(seconds == 0.0)
	{
		// Send a cmd that will update the fence's status in case someone calls clientWait with seconds==0.0 all the
		// time
		static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CheckFenceCommand>(FencePtr(this), seconds, 0.0,
																					 nullptr);
		static_cast<CommandBufferImpl&>(*cmdb).flush();

		return false;
	}
	else
	{
		Barrier barrier(2);

		Second flushTime = HighRezTimer::getCurrentTime();

		static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CheckFenceCommand>(FencePtr(this), seconds, flushTime,
																					 &barrier);
		static_cast<CommandBufferImpl&>(*cmdb).flush();

		barrier.wait();

		return static_cast<FenceImpl&>(*this).m_signaled.load();
	}
}

} // end namespace anki
