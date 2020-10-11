// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/FenceImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>

namespace anki
{

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
			return Error::NONE;
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