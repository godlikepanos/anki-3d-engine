// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Util/Enum.h>

namespace anki {

Renderer& RendererObject::getRenderer()
{
	return MainRenderer::getSingleton().getOffscreenRenderer();
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
	grProg.reset(&variant->getProgram());

	return Error::kNone;
}

Error RendererObject::loadShaderProgram(CString filename, ConstWeakArray<SubMutation> mutators, ShaderProgramResourcePtr& rsrc,
										ShaderProgramPtr& grProg)
{
	if(!rsrc.isCreated())
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(filename, rsrc));
	}

	ShaderProgramResourceVariantInitInfo initInf(rsrc);
	for(SubMutation pair : mutators)
	{
		initInf.addMutation(pair.m_mutatorName, pair.m_value);
	}

	const ShaderProgramResourceVariant* variant;
	rsrc->getOrCreateVariant(initInf, variant);

	grProg.reset(&variant->getProgram());

	return Error::kNone;
}

void RendererObject::zeroBuffer(Buffer* buff)
{
	CommandBufferInitInfo cmdbInit("Zero buffer");
	cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

	cmdb->fillBuffer(buff, 0, kMaxPtrSize, 0);

	FencePtr fence;
	cmdb->flush({}, &fence);

	fence->clientWait(16.0_sec);
}

} // end namespace anki
