// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/gl/FenceImpl.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/gl/CommandBufferImpl.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/gl/GrManagerImpl.h>
#include <AnKi/Gr/gl/RenderingThread.h>

namespace anki {

FenceImpl::~FenceImpl()
{
	class DeleteFenceCommand final : public GlCommand
	{
	public:
		GLsync m_fence;

		DeleteFenceCommand(GLsync fence)
			: m_fence(fence)
		{
		}

		Error operator()(GlState&)
		{
			glDeleteSync(m_fence);
			return Error::kNone;
		}
	};

	if(m_fence)
	{
		GrManagerImpl& manager = static_cast<GrManagerImpl&>(getManager());
		RenderingThread& thread = manager.getRenderingThread();

		if(!thread.isServerThread())
		{
			CommandBufferPtr commands;

			commands = manager.newCommandBuffer(CommandBufferInitInfo());
			static_cast<CommandBufferImpl&>(*commands).pushBackNewCommand<DeleteFenceCommand>(m_fence);
			static_cast<CommandBufferImpl&>(*commands).flush();
		}
		else
		{
			glDeleteSync(m_fence);
		}
	}
}

} // end namespace anki
