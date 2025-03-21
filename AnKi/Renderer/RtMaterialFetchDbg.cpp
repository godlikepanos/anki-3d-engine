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
#include <AnKi/Shaders/Include/MaterialTypes.h>

namespace anki {

Error RtMaterialFetchDbg::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtProg, m_sbtBuildGrProg, "SbtBuild"));

	// Ray gen and miss
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtMaterialFetchDbg.ankiprogbin", m_rtProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_rtProg);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
		const ShaderProgramResourceVariant* variant;
		m_rtProg->getOrCreateVariant(variantInitInfo, variant);
		m_libraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();
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

	m_rtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
															 Format::kR8G8B8A8_Unorm, "RtMaterialFetch");
	m_rtDesc.bake();

	return Error::kNone;
}

void RtMaterialFetchDbg::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	BufferHandle visibilityDep;
	BufferView visibleRenderableIndicesBuff, sbtBuildIndirectArgsBuff;
	getRenderer().getAccelerationStructureBuilder().getVisibilityInfo(visibilityDep, visibleRenderableIndicesBuff, sbtBuildIndirectArgsBuff);

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	{
		// Allocate SBT
		U32 sbtAlignment = (GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
							   ? sizeof(U32)
							   : GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
		sbtAlignment = computeCompoundAlignment(sbtAlignment, GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment);
		U8* sbtMem;
		sbtBuffer = RebarTransientMemoryPool::getSingleton().allocate(
			(GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount() + 2) * m_sbtRecordSize, sbtAlignment, sbtMem);
		sbtHandle = rgraph.importBuffer(sbtBuffer, BufferUsageBit::kUavCompute);

		// Write the first 2 entries of the SBT
		ConstWeakArray<U8> shaderGroupHandles = m_libraryGrProg->getShaderGroupHandles();
		const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
		memcpy(sbtMem, &shaderGroupHandles[m_rayGenShaderGroupIdx * shaderHandleSize], shaderHandleSize);
		memcpy(sbtMem + m_sbtRecordSize, &shaderGroupHandles[m_missShaderGroupIdx * shaderHandleSize], shaderHandleSize);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtMaterialFetch build SBT");

		rpass.newBufferDependency(visibilityDep, BufferUsageBit::kSrvCompute | BufferUsageBit::kIndirectCompute);
		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, sbtBuildIndirectArgsBuff, sbtBuffer, visibleRenderableIndicesBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtMaterialFetchSbtBuild);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_sbtBuildGrProg.get());

			cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindSrv(1, 0, visibleRenderableIndicesBuff);
			cmdb.bindSrv(2, 0, BufferView(&m_libraryGrProg->getShaderGroupHandlesGpuBuffer()));
			cmdb.bindUav(0, 0, sbtBuffer);

			RtShadowsSbtBuildConstants consts = {};
			ANKI_ASSERT(m_sbtRecordSize % 4 == 0);
			consts.m_sbtRecordDwordSize = m_sbtRecordSize / 4;
			const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
			ANKI_ASSERT(shaderHandleSize % 4 == 0);
			consts.m_shaderHandleDwordSize = shaderHandleSize / 4;
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchComputeIndirect(sbtBuildIndirectArgsBuff);
		});
	}

	// Ray gen
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDesc);

		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtMaterialFetch");

		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavTraceRays);
		rpass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kTraceRaysSrv);

		rpass.setWork([this, sbtBuffer, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtMaterialFetchRayGen);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_libraryGrProg.get());

			// More globals
			cmdb.bindSampler(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESH_LODS, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_TRANSFORMS, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) \
	cmdb.bindSrv( \
		reg, 0, \
		BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
				   getAlignedRoundDown(getFormatInfo(Format::k##fmt).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())), \
		Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

			cmdb.bindConstantBuffer(0, 2, ctx.m_globalRenderingConstantsBuffer);

			rgraphCtx.bindSrv(0, 2, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());

			// Fill the rest of the interface resources
			cmdb.bindSrv(1, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(2, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(3, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(4, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(5, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(GpuSceneGlobalIlluminationProbe)));
			cmdb.bindSrv(6, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(PixelFailedSsr)));
			cmdb.bindSrv(7, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));

			for(U32 i = 0; i < 6; ++i)
			{
				cmdb.bindUav(i, 2, TextureView(getDummyGpuResources().m_texture3DUav.get(), TextureSubresourceDesc::firstSurface()));
			}

			rgraphCtx.bindUav(7, 2, m_runCtx.m_rt);
			cmdb.bindUav(8, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::firstSurface()));

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());

			Vec4 dummy;
			cmdb.setFastConstants(&dummy, sizeof(dummy));

			cmdb.traceRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
						   getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), 1);
		});
	}
}

} // end namespace anki
