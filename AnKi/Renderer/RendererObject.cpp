// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/ThreadHive.h>

namespace anki {

RendererExternalSubsystems& RendererObject::getExternalSubsystems()
{
	return m_r->getExternalSubsystems();
}

const RendererExternalSubsystems& RendererObject::getExternalSubsystems() const
{
	return m_r->getExternalSubsystems();
}

HeapMemoryPool& RendererObject::getMemoryPool() const
{
	return m_r->getMemoryPool();
}

void* RendererObject::allocateRebarStagingMemory(PtrSize size, RebarGpuMemoryToken& token)
{
	return getExternalSubsystems().m_rebarStagingPool->allocateFrame(size, token);
}

void RendererObject::bindUniforms(CommandBufferPtr& cmdb, U32 set, U32 binding, const RebarGpuMemoryToken& token) const
{
	if(!token.isUnused())
	{
		cmdb->bindUniformBuffer(set, binding, getExternalSubsystems().m_rebarStagingPool->getBuffer(), token.m_offset,
								token.m_range);
	}
	else
	{
		cmdb->bindUniformBuffer(set, binding, m_r->getDummyBuffer(), 0, m_r->getDummyBuffer()->getSize());
	}
}

void RendererObject::bindStorage(CommandBufferPtr& cmdb, U32 set, U32 binding, const RebarGpuMemoryToken& token) const
{
	if(!token.isUnused())
	{
		cmdb->bindStorageBuffer(set, binding, getExternalSubsystems().m_rebarStagingPool->getBuffer(), token.m_offset,
								token.m_range);
	}
	else
	{
		cmdb->bindStorageBuffer(set, binding, m_r->getDummyBuffer(), 0, m_r->getDummyBuffer()->getSize());
	}
}

U32 RendererObject::computeNumberOfSecondLevelCommandBuffers(U32 drawcallCount) const
{
	const U32 drawcallsPerThread = drawcallCount / getExternalSubsystems().m_threadHive->getThreadCount();
	U32 secondLevelCmdbCount;
	if(drawcallsPerThread < kMinDrawcallsPerSecondaryCommandBuffer)
	{
		secondLevelCmdbCount = max(1u, drawcallCount / kMinDrawcallsPerSecondaryCommandBuffer);
	}
	else
	{
		secondLevelCmdbCount = getExternalSubsystems().m_threadHive->getThreadCount();
	}

	return secondLevelCmdbCount;
}

void RendererObject::registerDebugRenderTarget(CString rtName)
{
	m_r->registerDebugRenderTarget(this, rtName);
}

} // end namespace anki
