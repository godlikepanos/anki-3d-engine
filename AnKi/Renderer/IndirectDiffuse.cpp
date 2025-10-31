// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuse.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>

namespace anki {

Error IndirectDiffuse::init()
{
	[[maybe_unused]] const Bool bRt = GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled && g_cvarRenderRtIndirectDiffuse;
	ANKI_ASSERT(bRt);

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/IndirectDiffuse.ankiprogbin", m_mainProg));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_mainProg);
	variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
	const ShaderProgramResourceVariant* variant;
	m_mainProg->getOrCreateVariant(variantInitInfo, variant);
	m_libraryGrProg.reset(&variant->getProgram());
	m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtMaterialFetchMiss.ankiprogbin", m_missProg));

	ShaderProgramResourceVariantInitInfo variantInitInfo2(m_missProg);
	variantInitInfo2.requestTechniqueAndTypes(ShaderTypeBit::kMiss, "RtMaterialFetch");
	m_missProg->getOrCreateVariant(variantInitInfo2, variant);
	m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtProg, m_sbtBuildGrProg, "SbtBuild"));

	m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
										GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));

	m_transientRtDesc1 = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "IndirectDiffuse #1");
	m_transientRtDesc1.bake();

	return Error::kNone;
}

void IndirectDiffuse::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	const Bool bRt = GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled && g_cvarRenderRtIndirectDiffuse;

	if(!bRt)
	{
		return;
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const RenderTargetHandle transientRt1 = rgraph.newRenderTarget(m_transientRtDesc1);

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	{
		AccelerationStructureVisibilityInfo asVis;
		GpuVisibilityLocalLightsOutput localLightsVis;
		getRenderer().getAccelerationStructureBuilder().getVisibilityInfo(asVis, localLightsVis);

		// Allocate SBT
		U32 sbtAlignment = (GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
							   ? sizeof(U32)
							   : GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
		sbtAlignment = computeCompoundAlignment(sbtAlignment, GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment);
		U8* sbtMem;
		sbtBuffer = RebarTransientMemoryPool::getSingleton().allocate(
			(GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount() + 2) * m_sbtRecordSize, sbtAlignment, sbtMem);
		sbtHandle = rgraph.importBuffer(sbtBuffer, BufferUsageBit::kNone);

		// Write the first 2 entries of the SBT
		ConstWeakArray<U8> shaderGroupHandles = m_libraryGrProg->getShaderGroupHandles();
		const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
		memcpy(sbtMem, &shaderGroupHandles[m_rayGenShaderGroupIdx * shaderHandleSize], shaderHandleSize);
		memcpy(sbtMem + m_sbtRecordSize, &shaderGroupHandles[m_missShaderGroupIdx * shaderHandleSize], shaderHandleSize);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtIndirectDiffuse build SBT");

		rpass.newBufferDependency(asVis.m_depedency, BufferUsageBit::kIndirectCompute | BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, buildSbtIndirectArgsBuff = asVis.m_buildSbtIndirectArgsBuffer, sbtBuffer,
					   visibleRenderableIndicesBuff = asVis.m_visibleRenderablesBuffer](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseSbtBuild);
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

			cmdb.dispatchComputeIndirect(buildSbtIndirectArgsBuff);
		});
	}

	// Ray gen
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtIndirectDiffuse");

		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavDispatchRays);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvDispatchRays);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvDispatchRays);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvDispatchRays);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			rpass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvDispatchRays);
		}
		rpass.newTextureDependency(getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvDispatchRays);
		rpass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kSrvDispatchRays);

		rpass.setWork([this, sbtBuffer, &ctx, transientRt1](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseRayGen);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_libraryGrProg.get());

			// More globals
			cmdb.bindSampler(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESH_LODS, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_TRANSFORMS, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_UNIFIED_GEOMETRY, 0, UnifiedGeometryBuffer::getSingleton().getBufferView());
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) \
	cmdb.bindSrv( \
		reg, 0, \
		BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
				   getAlignedRoundDown(getFormatInfo(Format::k##fmt).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())), \
		Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

			cmdb.bindConstantBuffer(0, 2, ctx.m_globalRenderingConstantsBuffer);

			rgraphCtx.bindSrv(0, 2, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());
			rgraphCtx.bindSrv(1, 2, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(2, 2, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(3, 2, getGBuffer().getColorRt(2));

			const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
			const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
			const Bool bSkySolidColor =
				(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));
			if(bSkySolidColor)
			{
				cmdb.bindSrv(4, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			}
			else if(sky->getSkyboxType() == SkyboxType::kImage2D)
			{
				cmdb.bindSrv(4, 2, TextureView(&sky->getImageResource().getTexture(), TextureSubresourceDesc::all()));
			}
			else
			{
				rgraphCtx.bindSrv(4, 2, getRenderer().getGeneratedSky().getEnvironmentMapRt());
			}

			cmdb.bindSrv(5, 2, GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferView());
			cmdb.bindSrv(6, 2, BufferView(getDummyGpuResources().m_buffer.get()));
			rgraphCtx.bindSrv(7, 2, getShadowMapping().getShadowmapRt());

			rgraphCtx.bindUav(0, 2, transientRt1);
			cmdb.bindUav(1, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::all()));

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());

			UVec4 dummyConsts;
			cmdb.setFastConstants(&dummyConsts, sizeof(dummyConsts));

			cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							  getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), 1);
		});
	}

	m_runCtx.m_rt = transientRt1;
}

} // end namespace anki
