// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
			return ErrorCode::NONE;
		}
	};

	if(m_fence)
	{
		GrManager& manager = getManager();
		RenderingThread& thread = manager.getImplementation().getRenderingThread();

		if(!thread.isServerThread())
		{
			CommandBufferPtr commands;

			commands = manager.newInstance<CommandBuffer>(CommandBufferInitInfo());
			commands->m_impl->pushBackNewCommand<DeleteFenceCommand>(m_fence);
			commands->flush();
		}
		else
		{
			glDeleteSync(m_fence);
		}
	}
}

} // end namespace anki