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
		return;
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("GBuffPost");

	rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(0), TextureUsageBit::kUavCompute);
	rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kUavCompute);
	rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kUavCompute);
	rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

	rpass.newBufferDependency(getRenderer().getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);

	rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		rgraphCtx.bindUav(0, 0, getRenderer().getGBuffer().getColorRt(0));
		rgraphCtx.bindUav(1, 0, getRenderer().getGBuffer().getColorRt(1));
		rgraphCtx.bindUav(2, 0, getRenderer().getGBuffer().getColorRt(2));

		rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());
		cmdb.bindSrv(1, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kDecal));
		cmdb.bindSrv(2, 0, getRenderer().getClusterBinning().getClustersBuffer());

		cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

		cmdb.bindShaderProgram(m_grProg.get());

		dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
	});
}

} // end namespace anki
