// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

//==============================================================================
CommandBufferImpl::CommandBufferImpl(GrManager* manager)
	: VulkanObject(manager)
{
}

//==============================================================================
CommandBufferImpl::~CommandBufferImpl()
{
	if(m_handle)
	{
		getGrManagerImpl().deleteCommandBuffer(m_handle, m_secondLevel, m_tid);
	}
}

//==============================================================================
Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	m_secondLevel = init.m_secondLevel;
	m_tid = Thread::getCurrentThreadId();

	m_handle = getGrManagerImpl().newCommandBuffer(m_tid, m_secondLevel);
	ANKI_ASSERT(m_handle);

	return ErrorCode::NONE;
}

//==============================================================================
void CommandBufferImpl::commandChecks()
{
	ANKI_ASSERT(Thread::getCurrentThreadId() == m_tid
		&& "Commands must be recorder by the thread this command buffer was "
		   "created");
}

} // end namespace anki