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
	buildShaderBindingTablePass("RtMaterialFetchDbg: Build SBT", m_libraryGrProg.get(), m_rayGenShaderGroupIdx, m_missShaderGroupIdx, m_sbtRecordSize,
								rgraph, sbtHandle, sbtBuffer);

	// Ray gen
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDesc);

		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtMaterialFetch");

		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavDispatchRays);
		rpass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kSrvDispatchRays);

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

			U32 srv = 0;
			rgraphCtx.bindSrv(srv++, 2, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());

			cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));

			cmdb.bindSrv(srv++, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(GpuSceneGlobalIlluminationProbe)));
			cmdb.bindSrv(srv++, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(PixelFailedSsr)));

			for(U32 i = 0; i < kIndirectDiffuseClipmapCount * 3; ++i)
			{
				cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture3DSrv.get(), TextureSubresourceDesc::all()));
			}

			for(U32 i = 0; i < 3; ++i)
			{
				cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			}

			rgraphCtx.bindUav(0, 2, m_runCtx.m_rt);
			cmdb.bindUav(1, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::firstSurface()));

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());
			cmdb.bindSampler(2, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());

			Vec4 dummy;
			cmdb.setFastConstants(&dummy, sizeof(dummy));

			cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							  getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), 1);
		});
	}
}

} // end namespace anki
