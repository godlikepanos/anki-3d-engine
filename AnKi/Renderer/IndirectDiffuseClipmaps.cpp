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
	m_clipmapInfo[0].m_size = Vec3(g_indirectDiffuseClipmap0SizeCVar);

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		for(U32 i = 0; i < 3; ++i)
		{
			TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
				g_indirectDiffuseClipmap0ProbesPerDimCVar, g_indirectDiffuseClipmap0ProbesPerDimCVar, Format::kR16G16B16A16_Sfloat,
				TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap #%u comp #%u", clipmap, i));
			volumeInit.m_depth = g_indirectDiffuseClipmap0ProbesPerDimCVar;
			volumeInit.m_type = TextureType::k3D;

			m_clipmapVolumes[clipmap].m_perColorComponent[i] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
		}
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", {}, m_prog, m_tmpVisGrProg, "Test"));
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

	return Error::kNone;
}

void IndirectDiffuseClipmaps::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	Array2d<RenderTargetHandle, kIndirectDiffuseClipmapCount, 3> volumeRts;
	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		for(U32 c = 0; c < 3; ++c)
		{
			if(!m_clipmapsImportedOnce)
			{
				volumeRts[clipmap][c] =
					rgraph.importRenderTarget(m_clipmapVolumes[clipmap].m_perColorComponent[c].get(), TextureUsageBit::kSrvCompute);
			}
			else
			{
				volumeRts[clipmap][c] = rgraph.importRenderTarget(m_clipmapVolumes[clipmap].m_perColorComponent[c].get());
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
			for(U32 c = 0; c < 3; ++c)
			{
				pass.newTextureDependency(volumeRts[clipmap][c], TextureUsageBit::kUavCompute);
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
			cmdb.bindSrv(1, 2, TextureView(&getRenderer().getDummyTexture2d(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(2, 2, TextureView(&getRenderer().getDummyTexture2d(), TextureSubresourceDesc::all()));
			cmdb.bindSrv(3, 2, TextureView(&getRenderer().getDummyTexture2d(), TextureSubresourceDesc::all()));

			const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
			const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
			const Bool bSkySolidColor =
				(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));
			if(bSkySolidColor)
			{
				cmdb.bindSrv(4, 2, TextureView(&getRenderer().getDummyTexture2d(), TextureSubresourceDesc::all()));
			}
			else if(sky->getSkyboxType() == SkyboxType::kImage2D)
			{
				cmdb.bindSrv(4, 2, TextureView(&sky->getImageResource().getTexture(), TextureSubresourceDesc::all()));
			}
			else
			{
				rgraphCtx.bindSrv(4, 2, getRenderer().getGeneratedSky().getEnvironmentMapRt());
			}

			cmdb.bindSrv(5, 2, BufferView(&getRenderer().getDummyBuffer(), 0, sizeof(U32)));
			cmdb.bindSrv(6, 2, BufferView(&getRenderer().getDummyBuffer(), 0, sizeof(U32)));
			rgraphCtx.bindSrv(7, 2, getRenderer().getShadowMapping().getShadowmapRt());

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				for(U32 c = 0; c < 3; ++c)
				{
					rgraphCtx.bindUav(c, 2, volumeRts[clipmap][c]);
				}

				const Vec4 consts(F32(U32(g_indirectDiffuseClipmap0SizeCVar) << clipmap));
				cmdb.setFastConstants(&consts, sizeof(consts));

				const U32 probeCount = m_clipmapVolumes[clipmap].m_perColorComponent[0]->getWidth();
				cmdb.traceRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							   probeCount, probeCount, probeCount);
			}
		});
	}

	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps test");

		const U32 clipmap = 2;

		for(U32 c = 0; c < 3; ++c)
		{
			pass.newTextureDependency(volumeRts[clipmap][c], TextureUsageBit::kSrvCompute);
		}
		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(m_runCtx.m_tmpRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, volumeRts, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tmpVisGrProg.get());

			const Vec4 consts(F32(U32(g_indirectDiffuseClipmap0SizeCVar) << clipmap));
			cmdb.setFastConstants(&consts, sizeof(consts));

			rgraphCtx.bindSrv(0, 0, volumeRts[clipmap][0]);
			rgraphCtx.bindSrv(1, 0, volumeRts[clipmap][1]);
			rgraphCtx.bindSrv(2, 0, volumeRts[clipmap][2]);
			rgraphCtx.bindSrv(3, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(4, 0, getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindUav(0, 0, m_runCtx.m_tmpRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}
}

} // end namespace anki
