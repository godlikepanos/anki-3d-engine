// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/RenderingPass.h>
#include <anki/renderer/Renderer.h>
#include <anki/util/Enum.h>

namespace anki
{

GrManager& RenderingPass::getGrManager()
{
	return m_r->getGrManager();
}

const GrManager& RenderingPass::getGrManager() const
{
	return m_r->getGrManager();
}

HeapAllocator<U8> RenderingPass::getAllocator() const
{
	return m_r->getAllocator();
}

StackAllocator<U8> RenderingPass::getFrameAllocator() const
{
	return m_r->getFrameAllocator();
}

ResourceManager& RenderingPass::getResourceManager()
{
	return m_r->getResourceManager();
}

void* RenderingPass::allocateFrameStagingMemory(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token)
{
	return m_r->getStagingGpuMemoryManager().allocateFrame(size, usage, token);
}

void RenderingPass::bindUniforms(CommandBufferPtr& cmdb, U set, U binding, const StagingGpuMemoryToken& token) const
{
	if(token && !token.isUnused())
	{
		cmdb->bindUniformBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	}
	else
	{
		cmdb->bindUniformBuffer(set, binding, m_r->getDummyBuffer(), 0, m_r->getDummyBufferSize());
	}
}

void RenderingPass::bindStorage(CommandBufferPtr& cmdb, U set, U binding, const StagingGpuMemoryToken& token) const
{
	if(token && !token.isUnused())
	{
		cmdb->bindStorageBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	}
	else
	{
		cmdb->bindStorageBuffer(set, binding, m_r->getDummyBuffer(), 0, m_r->getDummyBufferSize());
	}
}

} // end namespace anki
