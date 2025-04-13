// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>

namespace anki {

Error IndirectDiffuseClipmaps::init()
{
	m_tmpRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
																Format::kR8G8B8A8_Unorm, "Test");
	m_tmpRtDesc.bake();

	m_clipmapInfo[0].m_probeCounts =
		UVec3(g_indirectDiffuseClipmapProbesXZCVar, g_indirectDiffuseClipmapProbesYCVar, g_indirectDiffuseClipmapProbesXZCVar);
	m_clipmapInfo[1].m_probeCounts = m_clipmapInfo[0].m_probeCounts;
	m_clipmapInfo[2].m_probeCounts = m_clipmapInfo[0].m_probeCounts;
	m_clipmapInfo[0].m_size = Vec3(g_indirectDiffuseClipmap0XZSizeCVar, g_indirectDiffuseClipmap0YSizeCVar, g_indirectDiffuseClipmap0XZSizeCVar);
	m_clipmapInfo[1].m_size = Vec3(g_indirectDiffuseClipmap1XZSizeCVar, g_indirectDiffuseClipmap1YSizeCVar, g_indirectDiffuseClipmap1XZSizeCVar);
	m_clipmapInfo[2].m_size = Vec3(g_indirectDiffuseClipmap2XZSizeCVar, g_indirectDiffuseClipmap2YSizeCVar, g_indirectDiffuseClipmap2XZSizeCVar);

	U32 probesPerClipmap = 0;
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		const U32 count = m_clipmapInfo[i].m_probeCounts.x() * m_clipmapInfo[i].m_probeCounts.y() * m_clipmapInfo[i].m_probeCounts.z();
		m_clipmapInfo[i].m_probeCountsTotal = count;
		if(i == 0)
		{
			probesPerClipmap = count;
		}
		else
		{
			ANKI_ASSERT(probesPerClipmap == count);
		}
	}

	// Create the lighting result texture
	m_radianceDesc = getRenderer().create2DRenderTargetDescription(probesPerClipmap, kRaysPerProbePerFrame, Format::kB10G11R11_Ufloat_Pack32,
																   "IndirectDiffuse light result");
	m_radianceDesc.bake();

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		for(U32 dir = 0; dir < 6; ++dir)
		{
			TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
				m_clipmapInfo[clipmap].m_probeCounts.x() * (g_indirectDiffuseClipmapRadianceCacheProbeSize + 2),
				m_clipmapInfo[clipmap].m_probeCounts.z() * (g_indirectDiffuseClipmapRadianceCacheProbeSize + 2), Format::kB10G11R11_Ufloat_Pack32,
				TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmapRadiance #%u", clipmap));
			volumeInit.m_depth = m_clipmapInfo[clipmap].m_probeCounts.y();
			volumeInit.m_type = TextureType::k3D;

			m_radianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
		}
	}

	Array<SubMutation, 1> mutation = {"RAYS_PER_PROBE_PER_FRAME", kRaysPerProbePerFrame};

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_tmpVisGrProg, "Test"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_visProbesGrProg, "VisualizeProbes"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_populateCachesGrProg, "PopulateCaches"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtProg, m_sbtBuildGrProg, "SbtBuild"));

	{
		ShaderProgramResourcePtr tmpProg;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", tmpProg));
		ANKI_ASSERT(tmpProg == m_prog);

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
		variantInitInfo.addMutation("RAYS_PER_PROBE_PER_FRAME", kRaysPerProbePerFrame);
		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
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

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_blueNoiseImg));

	return Error::kNone;
}

void IndirectDiffuseClipmaps::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const RenderTargetHandle radianceRt = rgraph.newRenderTarget(m_radianceDesc);
	m_runCtx.m_tmpRt = rgraph.newRenderTarget(m_tmpRtDesc);

	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> radianceVolumes;
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		if(m_texturesImportedOnce)
		{
			radianceVolumes[i] = rgraph.importRenderTarget(m_radianceVolumes[i].get());
		}
		else
		{
			radianceVolumes[i] = rgraph.importRenderTarget(m_radianceVolumes[i].get(), TextureUsageBit::kSrvCompute);
		}
	}

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	{
		BufferHandle visibilityDep;
		BufferView visibleRenderableIndicesBuff, buildSbtIndirectArgsBuff;
		getRenderer().getAccelerationStructureBuilder().getVisibilityInfo(visibilityDep, visibleRenderableIndicesBuff, buildSbtIndirectArgsBuff);

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
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflections build SBT");

		rpass.newBufferDependency(visibilityDep, BufferUsageBit::kIndirectCompute | BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, buildSbtIndirectArgsBuff, sbtBuffer, visibleRenderableIndicesBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsSbtBuild);
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

	// Do ray tracing around the probes
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps");

		pass.newTextureDependency(radianceRt, TextureUsageBit::kUavCompute);
		pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			pass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvTraceRays);
		}
		pass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvTraceRays);
		pass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												AccelerationStructureUsageBit::kTraceRaysSrv);

		pass.setWork([this, radianceRt, &ctx, sbtBuffer](RenderPassWorkContext& rgraphCtx) {
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
			cmdb.bindSrv(1, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(2, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(3, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));

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

			cmdb.bindSrv(5, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(U32)));
			cmdb.bindSrv(6, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(U32)));
			rgraphCtx.bindSrv(7, 2, getRenderer().getShadowMapping().getShadowmapRt());

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());

			rgraphCtx.bindUav(0, 2, radianceRt);
			cmdb.bindUav(1, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::firstSurface()));

			const UVec4 consts(0, kRaysPerProbePerFrame, 0, 0); // TODO
			cmdb.setFastConstants(&consts, sizeof(consts));

			const U32 probeCount = U32(m_clipmapInfo[0].m_probeCountsTotal);
			cmdb.traceRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
						   probeCount * kRaysPerProbePerFrame, 1, 1);
		});
	}

	// Update caches
	{
		const U32 clipmap = 0;

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps populate caches");

		pass.newTextureDependency(radianceRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(radianceVolumes[clipmap], TextureUsageBit::kUavCompute);

		pass.setWork([this, &ctx, clipmap, radianceRt, radianceVolume = radianceVolumes[clipmap]](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_populateCachesGrProg.get());

			rgraphCtx.bindSrv(0, 0, radianceRt);
			rgraphCtx.bindUav(0, 0, radianceVolume);
			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			const UVec4 consts(clipmap, g_indirectDiffuseClipmapRadianceCacheProbeSize, 0, 0);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchCompute(m_clipmapInfo[clipmap].m_probeCounts.x(), m_clipmapInfo[clipmap].m_probeCounts.y(),
								 m_clipmapInfo[clipmap].m_probeCounts.z());
		});
	}

	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps test");

		// TODO
		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(m_runCtx.m_tmpRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tmpVisGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getColorRt(2));
			cmdb.bindSrv(2, 0, TextureView(&m_blueNoiseImg->getTexture(), TextureSubresourceDesc::firstSurface()));

			rgraphCtx.bindUav(0, 0, m_runCtx.m_tmpRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}
}

void IndirectDiffuseClipmaps::drawDebugProbes(const RenderingContext& ctx, CommandBuffer& cmdb) const
{
	cmdb.bindShaderProgram(m_visProbesGrProg.get());

	const UVec4 consts(0u, g_indirectDiffuseClipmapRadianceCacheProbeSize, 0, 0);
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);
	cmdb.bindSrv(0, 0, TextureView(m_radianceVolumes[0].get(), TextureSubresourceDesc::all()));
	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

	cmdb.draw(PrimitiveTopology::kTriangles, 36, m_clipmapInfo[0].m_probeCountsTotal);
}

} // end namespace anki
