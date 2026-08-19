// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ReSTIRDI.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error ReSTIRDI::init()
{
	TextureInitInfo texInit = getRenderer().create2DRenderTargetInitInfo(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR32G32B32A32_Uint,
		TextureUsageBit::kAllSrv | TextureUsageBit::kUavCompute, "ReSTIRDI: Reservoir #1");
	m_reservoirTextures[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);
	texInit.setName("ReSTIRDI: Reservoir #2");
	m_reservoirTextures[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	m_initialCandidatesRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR32G32B32A32_Uint, "ReSTIRDI: Initial Candidates");
	m_initialCandidatesRtDesc.bake();

	m_shadedPixelsRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, getRenderer().getHdrFormat(), "ReSTIRDI");
	m_shadedPixelsRtDesc.bake();

	ANKI_CHECK(m_groundTruthGrProg.load("ShaderBinaries/ReSTIRDI.ankiprogbin", {}, "GroundTruth"));
	ANKI_CHECK(m_genInitialCandidatesGrProg.load("ShaderBinaries/ReSTIRDI.ankiprogbin", {}, "GenInitialCandidates"));
	ANKI_CHECK(m_sampleReuseGrProg.load("ShaderBinaries/ReSTIRDI.ankiprogbin", {}, "SampleReuse"));

	return Error::kNone;
}

void ReSTIRDI::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(ReSTIRDI);

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_shadedPixelsRtDesc);
	const RenderTargetHandle initialCandidatesHandle = rgraph.newRenderTarget(m_initialCandidatesRtDesc);

	const U32 readIdx = getRenderer().getFrameCount() & 1;
	const RenderTargetHandle readReservoirHandle =
		rgraph.importRenderTarget(m_reservoirTextures[readIdx].get(), m_texturesFirstImport, TextureUsageBit::kSrvCompute);
	const RenderTargetHandle writeReservoirHandle =
		rgraph.importRenderTarget(m_reservoirTextures[!readIdx].get(), m_texturesFirstImport, TextureUsageBit::kSrvCompute);
	m_texturesFirstImport = false;

	// Initial candidates
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReSTIRDI: Initial Candidates");

		rpass.newTextureDependency(initialCandidatesHandle, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(0), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

		rpass.newAccelerationStructureDependency(getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);

		rpass.setWork([this, initialCandidatesHandle](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReSTIRDI);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_genInitialCandidatesGrProg.get());

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.bindSrv(0, 0, GpuSceneArrays::Light::getSingleton().getBufferViewSafe());
			cmdb.bindSrv(1, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
			cmdb.bindSrv(2, 0, getClusterBinning().getClustersBuffer());

			rgraphCtx.bindSrv(3, 0, getAccelerationStructureBuilder().getAccelerationStructureHandle());

			rgraphCtx.bindSrv(4, 0, getGBuffer().getColorRt(0));
			rgraphCtx.bindSrv(5, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(6, 0, getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(7, 0, getGBuffer().getDepthRt());

			rgraphCtx.bindUav(0, 0, initialCandidatesHandle);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		});
	}

	// Reservoir reuse
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReSTIRDI: Sample Reuse");

		rpass.newTextureDependency(writeReservoirHandle, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(readReservoirHandle, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(initialCandidatesHandle, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(getGBuffer().getColorRt(0), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getMotionVectors().getAdjustedMotionVectorsRt(), TextureUsageBit::kSrvCompute);

		rpass.newAccelerationStructureDependency(getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);

		rpass.setWork([this, initialCandidatesHandle, readReservoirHandle, writeReservoirHandle](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReSTIRDI);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_sampleReuseGrProg.get());

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.bindSrv(0, 0, GpuSceneArrays::Light::getSingleton().getBufferViewSafe());
			cmdb.bindSrv(1, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
			cmdb.bindSrv(2, 0, getClusterBinning().getClustersBuffer());

			rgraphCtx.bindSrv(3, 0, getAccelerationStructureBuilder().getAccelerationStructureHandle());

			rgraphCtx.bindSrv(4, 0, getGBuffer().getColorRt(0));
			rgraphCtx.bindSrv(5, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(6, 0, getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(7, 0, getGBuffer().getDepthRt());

			rgraphCtx.bindSrv(8, 0, initialCandidatesHandle);
			rgraphCtx.bindSrv(9, 0, readReservoirHandle);
			rgraphCtx.bindSrv(10, 0, getMotionVectors().getAdjustedMotionVectorsRt());
			rgraphCtx.bindUav(0, 0, writeReservoirHandle);
			rgraphCtx.bindUav(1, 0, m_runCtx.m_rt);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		});
	}
}

} // end namespace anki
