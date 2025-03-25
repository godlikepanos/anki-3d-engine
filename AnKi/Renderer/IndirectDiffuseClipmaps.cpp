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

	m_clipmapInfo[0].m_probeCounts = Vec3(F32(g_indirectDiffuseClipmap0ProbesPerDimCVar));
	m_clipmapInfo[1].m_probeCounts = Vec3(F32(g_indirectDiffuseClipmap1ProbesPerDimCVar));
	m_clipmapInfo[2].m_probeCounts = Vec3(F32(g_indirectDiffuseClipmap2ProbesPerDimCVar));
	m_clipmapInfo[0].m_size = Vec3(g_indirectDiffuseClipmap0SizeCVar);
	m_clipmapInfo[1].m_size = Vec3(g_indirectDiffuseClipmap1SizeCVar);
	m_clipmapInfo[2].m_size = Vec3(g_indirectDiffuseClipmap2SizeCVar);

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		for(U32 dir = 0; dir < 6; ++dir)
		{
			TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
				U32(m_clipmapInfo[clipmap].m_probeCounts.x()), U32(m_clipmapInfo[clipmap].m_probeCounts.y()), Format::kB10G11R11_Ufloat_Pack32,
				TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap #%u dir #%u", clipmap, dir));
			volumeInit.m_depth = U32(m_clipmapInfo[clipmap].m_probeCounts.z());
			volumeInit.m_type = TextureType::k3D;

			m_clipmapVolumes[clipmap].m_directions[dir] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
		}
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", {}, m_prog, m_tmpVisGrProg, "Test"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", {}, m_prog, m_visProbesGrProg, "VisualizeProbes"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtProg, m_sbtBuildGrProg, "SbtBuild"));

	{
		ShaderProgramResourcePtr tmpProg;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", tmpProg));
		ANKI_ASSERT(tmpProg == m_prog);

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
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

	Array2d<RenderTargetHandle, kIndirectDiffuseClipmapCount, 6> volumeRts;
	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		for(U32 dir = 0; dir < 6; ++dir)
		{
			if(!m_clipmapsImportedOnce)
			{
				volumeRts[clipmap][dir] = rgraph.importRenderTarget(m_clipmapVolumes[clipmap].m_directions[dir].get(), TextureUsageBit::kSrvCompute);
			}
			else
			{
				volumeRts[clipmap][dir] = rgraph.importRenderTarget(m_clipmapVolumes[clipmap].m_directions[dir].get());
			}
		}
	}
	m_clipmapsImportedOnce = true;

	m_runCtx.m_tmpRt = rgraph.newRenderTarget(m_tmpRtDesc);

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

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			for(U32 dir = 0; dir < 6; ++dir)
			{
				pass.newTextureDependency(volumeRts[clipmap][dir], TextureUsageBit::kUavCompute);
			}
		}
		pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			pass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvTraceRays);
		}
		pass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvTraceRays);
		pass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												AccelerationStructureUsageBit::kTraceRaysSrv);

		pass.setWork([this, volumeRts, &ctx, sbtBuffer](RenderPassWorkContext& rgraphCtx) {
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

			cmdb.bindUav(7, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::firstSurface()));
			cmdb.bindUav(8, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::firstSurface()));

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				for(U32 dir = 0; dir < 6; ++dir)
				{
					rgraphCtx.bindUav(dir, 2, volumeRts[clipmap][dir]);
				}

				const UVec4 consts(clipmap);
				cmdb.setFastConstants(&consts, sizeof(consts));

				const U32 probeCount = m_clipmapVolumes[clipmap].m_directions[0]->getWidth();
				cmdb.traceRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							   probeCount, probeCount, probeCount);
			}
		});
	}

	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps test");

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			for(U32 dir = 0; dir < 6; ++dir)
			{
				pass.newTextureDependency(volumeRts[clipmap][dir], TextureUsageBit::kSrvCompute);
			}
		}
		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(m_runCtx.m_tmpRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, volumeRts, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tmpVisGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getColorRt(2));
			cmdb.bindSrv(2, 0, TextureView(&m_blueNoiseImg->getTexture(), TextureSubresourceDesc::firstSurface()));

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				for(U32 dir = 0; dir < 6; ++dir)
				{
					rgraphCtx.bindSrv(clipmap * 6 + dir + 3, 0, volumeRts[clipmap][dir]);
				}
			}

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

	const UVec4 consts(0u);
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

	const U32 probeCount = U32(m_clipmapInfo[0].m_probeCounts.x() * m_clipmapInfo[0].m_probeCounts.y() * m_clipmapInfo[0].m_probeCounts.z());

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		for(U32 dir = 0; dir < 6; ++dir)
		{
			cmdb.bindSrv(clipmap * 6 + dir, 0, TextureView(m_clipmapVolumes[clipmap].m_directions[dir].get(), TextureSubresourceDesc::all()));
		}
	}

	cmdb.draw(PrimitiveTopology::kTriangles, 36, probeCount);
}

} // end namespace anki
