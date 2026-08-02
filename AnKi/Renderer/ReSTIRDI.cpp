// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ReSTIRDI.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error ReSTIRDI::init()
{
	m_rtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
															 getRenderer().getHdrFormat(), "ReSTIRDI");
	m_rtDesc.bake();

	ANKI_CHECK(m_groundTruthGrProg.load("ShaderBinaries/ReSTIRDI.ankiprogbin", {}, "GroundTruth"));

	return Error::kNone;
}

void ReSTIRDI::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(ReSTIRDI);

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDesc);

	NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReSTIRDI ground truth");

	rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavCompute);
	rpass.newTextureDependency(getGBuffer().getColorRt(0), TextureUsageBit::kSrvCompute);
	rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
	rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
	rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

	rpass.newAccelerationStructureDependency(getAccelerationStructureBuilder().getAccelerationStructureHandle(),
											 AccelerationStructureUsageBit::kSrvCompute);

	rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(ReSTIRDI);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_groundTruthGrProg.get());

		cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

		// The ground truth doesn't cull so it iterates the whole light array and not the visible subset. Use the safe variant because the array is
		// empty when the scene has no lights and a zero sized BufferView is illegal
		cmdb.bindSrv(0, 0, GpuSceneArrays::Light::getSingleton().getBufferViewSafe());

		rgraphCtx.bindSrv(1, 0, getGBuffer().getColorRt(0));
		rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(1));
		rgraphCtx.bindSrv(3, 0, getGBuffer().getColorRt(2));
		rgraphCtx.bindSrv(4, 0, getGBuffer().getDepthRt());
		rgraphCtx.bindSrv(5, 0, getAccelerationStructureBuilder().getAccelerationStructureHandle());

		rgraphCtx.bindUav(0, 0, m_runCtx.m_rt);

		dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
	});
}

} // end namespace anki
