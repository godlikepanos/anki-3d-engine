// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RtMaterialFetchDbg.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error RtMaterialFetchDbg::init()
{
	ANKI_CHECK(RtMaterialFetchRendererObject::init());

	// Ray gen and miss
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtMaterialFetchDbg.ankiprogbin", m_rtProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_rtProg);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
		const ShaderProgramResourceVariant* variant;
		m_rtProg->getOrCreateVariant(variantInitInfo, variant);
		m_libraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();

		m_shaderGroupHandlesBuff = variant->getShaderGroupHandlesBuffer();
	}

	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtMaterialFetchMiss.ankiprogbin", m_missProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_missProg);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kMiss, "RtMaterialFetch");
		const ShaderProgramResourceVariant* variant;
		m_missProg->getOrCreateVariant(variantInitInfo, variant);
		m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();
	}

	m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
										GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));

	m_rtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
															 Format::kR8G8B8A8_Unorm, "RtMaterialFetch");
	m_rtDesc.bake();

	return Error::kNone;
}

void RtMaterialFetchDbg::populateRenderGraph()
{
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	buildShaderBindingTablePass("RtMaterialFetchDbg: Build SBT", m_shaderGroupHandlesBuff, m_rayGenShaderGroupIdx, m_missShaderGroupIdx,
								m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);

	// Ray gen
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDesc);

		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtMaterialFetch");

		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavDispatchRays);

		setRgenSpace2Dependencies(rpass);

		rpass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kSrvDispatchRays);

		rpass.setWork([this, sbtBuffer](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtMaterialFetchRayGen);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_libraryGrProg.get());

			// Space 0 globals
#include <AnKi/Shaders/Include/MaterialBindings.def.h>

			bindRgenSpace2Resources(rgraphCtx);
			rgraphCtx.bindUav(0, 2, m_runCtx.m_rt);

			Vec4 dummy[3];
			cmdb.setFastConstants(&dummy, sizeof(dummy));

			cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							  getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, 1);
		});
	}
}

} // end namespace anki
