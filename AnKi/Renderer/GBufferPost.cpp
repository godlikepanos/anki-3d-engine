// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GBufferPost.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error GBufferPost::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize GBufferPost pass");
	}
	return err;
}

Error GBufferPost::initInternal()
{
	ANKI_R_LOGV("Initializing GBufferPost pass");

	// Load shaders
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/GBufferPost.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kTileCount", getRenderer().getTileCounts());
	variantInitInfo.addConstant("kZSplitCount", getRenderer().getZSplitCount());

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg.reset(&variant->getProgram());

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.m_colorAttachments[1].m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.bake();

	return Error::kNone;
}

void GBufferPost::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(GBufferPost);

	if(GpuSceneArrays::Decal::getSingleton().getElementCount() == 0)
	{
		// If there are no decals don't bother
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create pass
	GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("GBuffPost");

	rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kSrcAlpha, BlendFactor::kZero, BlendFactor::kOne);
		cmdb.setBlendFactors(1, BlendFactor::kOne, BlendFactor::kSrcAlpha, BlendFactor::kZero, BlendFactor::kOne);

		// Bind all
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

		rgraphCtx.bindTexture(0, 1, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearRepeat.get());

		cmdb.bindConstantBuffer(0, 3, getRenderer().getClusterBinning().getClusteredShadingConstants());
		cmdb.bindUavBuffer(0, 4, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kDecal));
		cmdb.bindUavBuffer(0, 5, getRenderer().getClusterBinning().getClustersBuffer());

		cmdb.bindAllBindless(1);

		// Draw
		cmdb.draw(PrimitiveTopology::kTriangles, 3);

		// Restore state
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
		cmdb.setBlendFactors(1, BlendFactor::kOne, BlendFactor::kZero);
	});

	rpass.setFramebufferInfo(m_fbDescr, {getRenderer().getGBuffer().getColorRt(0), getRenderer().getGBuffer().getColorRt(1)});

	rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(0), TextureUsageBit::kAllFramebuffer);
	rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kAllFramebuffer);
	rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSampledFragment,
							   TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

	rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kUavFragmentRead);
	rpass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kDecal),
							  BufferUsageBit::kUavFragmentRead);
}

} // end namespace anki
