// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/ThreadHive.h>

namespace anki {

Renderer& RendererObject::getRenderer()
{
	return MainRenderer::getSingleton().getOffscreenRenderer();
}

void* RendererObject::allocateRebarStagingMemory(PtrSize size, RebarAllocation& token)
{
	return RebarTransientMemoryPool::getSingleton().allocateFrame(size, token);
}

void RendererObject::bindUniforms(CommandBufferPtr& cmdb, U32 set, U32 binding, const RebarAllocation& token) const
{
	if(!token.isUnused())
	{
		cmdb->bindUniformBuffer(set, binding, RebarTransientMemoryPool::getSingleton().getBuffer(), token.m_offset, token.m_range);
	}
	else
	{
		cmdb->bindUniformBuffer(set, binding, getRenderer().getDummyBuffer(), 0, getRenderer().getDummyBuffer()->getSize());
	}
}

void RendererObject::bindStorage(CommandBufferPtr& cmdb, U32 set, U32 binding, const RebarAllocation& token) const
{
	if(!token.isUnused())
	{
		cmdb->bindStorageBuffer(set, binding, RebarTransientMemoryPool::getSingleton().getBuffer(), token.m_offset, token.m_range);
	}
	else
	{
		cmdb->bindStorageBuffer(set, binding, getRenderer().getDummyBuffer(), 0, getRenderer().getDummyBuffer()->getSize());
	}
}

U32 RendererObject::computeNumberOfSecondLevelCommandBuffers(U32 drawcallCount) const
{
	const U32 drawcallsPerThread = drawcallCount / CoreThreadHive::getSingleton().getThreadCount();
	U32 secondLevelCmdbCount;
	if(drawcallsPerThread < kMinDrawcallsPerSecondaryCommandBuffer)
	{
		secondLevelCmdbCount = max(1u, drawcallCount / kMinDrawcallsPerSecondaryCommandBuffer);
	}
	else
	{
		secondLevelCmdbCount = CoreThreadHive::getSingleton().getThreadCount();
	}

	return secondLevelCmdbCount;
}

void RendererObject::registerDebugRenderTarget(CString rtName)
{
	getRenderer().registerDebugRenderTarget(this, rtName);
}

Error RendererObject::loadShaderProgram(CString filename, ShaderProgramResourcePtr& rsrc, ShaderProgramPtr& grProg)
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(filename, rsrc));
	const ShaderProgramResourceVariant* variant;
	rsrc->getOrCreateVariant(variant);
	grProg = variant->getProgram();

	return Error::kNone;
}

} // end namespace anki
