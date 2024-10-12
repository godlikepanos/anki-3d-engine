// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GBufferPost.ankiprogbin", m_prog, m_grProg));
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

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Create pass
	GraphicsRenderPass& rpass = rgraph.newGraphicsRenderPass("GBuffPost");

	rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kSrcAlpha, BlendFactor::kZero, BlendFactor::kOne);
		cmdb.setBlendFactors(1, BlendFactor::kOne, BlendFactor::kSrcAlpha, BlendFactor::kZero, BlendFactor::kOne);

		// Bind all
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

		rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());

		cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

		cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);
		cmdb.bindSrv(0, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kDecal));
		cmdb.bindSrv(1, 0, getRenderer().getClusterBinning().getClustersBuffer());

		// Draw
		cmdb.draw(PrimitiveTopology::kTriangles, 3);

		// Restore state
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
		cmdb.setBlendFactors(1, BlendFactor::kOne, BlendFactor::kZero);
	});

	GraphicsRenderPassTargetDesc rt0(getRenderer().getGBuffer().getColorRt(0));
	rt0.m_loadOperation = RenderTargetLoadOperation::kLoad;
	GraphicsRenderPassTargetDesc rt1(getRenderer().getGBuffer().getColorRt(1));
	rt1.m_loadOperation = RenderTargetLoadOperation::kLoad;
	rpass.setRenderpassInfo({rt0, rt1});

	rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(0), TextureUsageBit::kAllRtvDsv);
	rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kAllRtvDsv);
	rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvPixel);

	rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kSrvPixel);
	rpass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kDecal),
							  BufferUsageBit::kSrvPixel);
}

} // end namespace anki
