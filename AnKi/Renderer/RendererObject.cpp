// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/ThreadHive.h>

namespace anki {

GrManager& RendererObject::getGrManager()
{
	return m_r->getGrManager();
}

const GrManager& RendererObject::getGrManager() const
{
	return m_r->getGrManager();
}

HeapMemoryPool& RendererObject::getMemoryPool() const
{
	return m_r->getMemoryPool();
}

ResourceManager& RendererObject::getResourceManager()
{
	return m_r->getResourceManager();
}

void* RendererObject::allocateFrameStagingMemory(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token)
{
	return m_r->getStagingGpuMemory().allocateFrame(size, usage, token);
}

void RendererObject::bindUniforms(CommandBufferPtr& cmdb, U32 set, U32 binding,
								  const StagingGpuMemoryToken& token) const
{
	if(!token.isUnused())
	{
		cmdb->bindUniformBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	}
	else
	{
		cmdb->bindUniformBuffer(set, binding, m_r->getDummyBuffer(), 0, m_r->getDummyBuffer()->getSize());
	}
}

void RendererObject::bindStorage(CommandBufferPtr& cmdb, U32 set, U32 binding, const StagingGpuMemoryToken& token) const
{
	if(!token.isUnused())
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
	const U32 drawcallsPerThread = drawcallCount / m_r->getThreadHive().getThreadCount();
	U32 secondLevelCmdbCount;
	if(drawcallsPerThread < kMinDrawcallsPerSecondaryCommandBuffer)
	{
		secondLevelCmdbCount = max(1u, drawcallCount / kMinDrawcallsPerSecondaryCommandBuffer);
	}
	else
	{
		secondLevelCmdbCount = m_r->getThreadHive().getThreadCount();
	}

	return secondLevelCmdbCount;
}

void RendererObject::registerDebugRenderTarget(CString rtName)
{
	m_r->registerDebugRenderTarget(this, rtName);
}

const ConfigSet& RendererObject::getConfig() const
{
	return m_r->getConfig();
}

} // end namespace anki
