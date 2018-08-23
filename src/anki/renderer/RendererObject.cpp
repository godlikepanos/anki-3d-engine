// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/RendererObject.h>
#include <anki/renderer/Renderer.h>
#include <anki/util/Enum.h>
#include <anki/util/ThreadHive.h>

namespace anki
{

GrManager& RendererObject::getGrManager()
{
	return m_r->getGrManager();
}

const GrManager& RendererObject::getGrManager() const
{
	return m_r->getGrManager();
}

HeapAllocator<U8> RendererObject::getAllocator() const
{
	return m_r->getAllocator();
}

ResourceManager& RendererObject::getResourceManager()
{
	return m_r->getResourceManager();
}

void* RendererObject::allocateFrameStagingMemory(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token)
{
	return m_r->getStagingGpuMemoryManager().allocateFrame(size, usage, token);
}

void RendererObject::bindUniforms(CommandBufferPtr& cmdb, U set, U binding, const StagingGpuMemoryToken& token) const
{
	if(token && !token.isUnused())
	{
		cmdb->bindUniformBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	}
	else
	{
		cmdb->bindUniformBuffer(set, binding, m_r->getDummyBuffer(), 0, m_r->getDummyBuffer()->getSize());
	}
}

void RendererObject::bindStorage(CommandBufferPtr& cmdb, U set, U binding, const StagingGpuMemoryToken& token) const
{
	if(token && !token.isUnused())
	{
		cmdb->bindStorageBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	}
	else
	{
		cmdb->bindStorageBuffer(set, binding, m_r->getDummyBuffer(), 0, m_r->getDummyBuffer()->getSize());
	}
}

U32 RendererObject::computeNumberOfSecondLevelCommandBuffers(U32 drawcallCount) const
{
	const U drawcallsPerThread = drawcallCount / m_r->getThreadHive().getThreadCount();
	U secondLevelCmdbCount;
	if(drawcallsPerThread < MIN_DRAWCALLS_PER_2ND_LEVEL_COMMAND_BUFFER)
	{
		secondLevelCmdbCount = max<U>(1u, drawcallCount / MIN_DRAWCALLS_PER_2ND_LEVEL_COMMAND_BUFFER);
	}
	else
	{
		secondLevelCmdbCount = m_r->getThreadHive().getThreadCount();
	}

	return secondLevelCmdbCount;
}

} // end namespace anki
