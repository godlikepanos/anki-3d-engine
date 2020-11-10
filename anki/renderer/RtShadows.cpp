// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/RtShadows.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/resource/ShaderProgramResourceSystem.h>

namespace anki
{

RtShadows::~RtShadows()
{
}

Error RtShadows::init(const ConfigSet& cfg)
{
	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize ray traced shadows");
	}

	return Error::NONE;
}

Error RtShadows::initInternal(const ConfigSet& cfg)
{
	// Get the program
	const ShaderProgramResourceSystem& shaderSystem = getResourceManager().getShaderProgramResourceSystem();
	const ShaderProgramRaytracingLibrary* library = nullptr;
	for(const ShaderProgramRaytracingLibrary& lib : shaderSystem.getRayTracingLibraries())
	{
		if(lib.getLibraryName() == "RtShadows")
		{
			library = &lib;
			break;
		}
	}
	ANKI_ASSERT(library);
	m_grProg = library->getShaderProgram();

	// RT descriptor
	m_rtDescr = m_r->create2DRenderTargetDescription(m_r->getWidth() / 2, m_r->getHeight() / 2, Format::R8G8B8A8_UNORM,
													 "RtShadows");
	m_rtDescr.bake();

	return Error::NONE;
}

void RtShadows::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	buildSbtAndTlas();

	m_runCtx.m_tlasHandle = rgraph.importAccelerationStructure(m_runCtx.m_tlas, AccelerationStructureUsageBit::NONE);

	// Build TLAS
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadowsBuildTlas");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<RtShadows*>(rgraphCtx.m_userData)->runBuildAs(rgraphCtx);
			},
			this, 0);

		rpass.newDependency(RenderPassDependency(m_runCtx.m_tlasHandle, AccelerationStructureUsageBit::BUILD));
	}

	// RT
	if(0) // TODO
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<RtShadows*>(rgraphCtx.m_userData)->run(rgraphCtx); },
			this, 0);

		rpass.newDependency(RenderPassDependency(m_runCtx.m_rt, TextureUsageBit::IMAGE_TRACE_RAYS_WRITE));
		rpass.newDependency(
			RenderPassDependency(m_runCtx.m_tlasHandle, AccelerationStructureUsageBit::TRACE_RAYS_READ));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_TRACE_RAYS));
	}
}

void RtShadows::runBuildAs(RenderPassWorkContext& rgraphCtx)
{
	rgraphCtx.m_commandBuffer->buildAccelerationStructure(m_runCtx.m_tlas);
}

void RtShadows::run(RenderPassWorkContext& rgraphCtx)
{
	const RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const ClusterBinOut& rsrc = ctx.m_clusterBinOut;

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearRepeat);
	rgraphCtx.bindImage(0, 1, m_runCtx.m_rt, TextureSubresourceInfo());
	cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindTexture(0, 3, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindAccelerationStructure(0, 4, m_runCtx.m_tlasHandle);

	bindUniforms(cmdb, 0, 4, ctx.m_lightShadingUniformsToken);

	bindUniforms(cmdb, 0, 5, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 6, rsrc.m_spotLightsToken);
	rgraphCtx.bindColorTexture(0, 7, m_r->getShadowMapping().getShadowmapRt());

	bindStorage(cmdb, 0, 8, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 9, rsrc.m_indicesToken);

	cmdb->traceRays(m_runCtx.m_sbtBuffer, m_runCtx.m_sbtOffset, m_runCtx.m_hitGroupCount, 1, m_r->getWidth() / 2,
					m_r->getHeight() / 2, 1);
}

void RtShadows::buildSbtAndTlas()
{
	// Get some things
	RenderingContext& ctx = *m_runCtx.m_ctx;

	ANKI_ASSERT(ctx.m_renderQueue->m_rayTracingQueue);
	ConstWeakArray<RayTracingInstanceQueueElement> instanceElements =
		ctx.m_renderQueue->m_rayTracingQueue->m_rayTracingInstances;
	const U32 instanceCount = instanceElements.getSize();
	ANKI_ASSERT(instanceCount > 0);

	const U32 sbtRecordSize = getGrManager().getDeviceCapabilities().m_sbtRecordSize;
	const U32 shaderHandleSize = getGrManager().getDeviceCapabilities().m_shaderGroupHandleSize;

	const U32 extraSbtRecords = 1 + 1; // Raygen + miss

	m_runCtx.m_hitGroupCount = instanceCount;

	// Allocate SBT
	StagingGpuMemoryToken token;
	U8* sbt = allocateStorage<U8*>(sbtRecordSize * (instanceCount + extraSbtRecords), token);
	const U8* sbtStart = sbt;
	(void)sbtStart;
	m_runCtx.m_sbtBuffer = token.m_buffer;
	m_runCtx.m_sbtOffset = token.m_offset;

	// Set the miss and ray gen handles
	memcpy(sbt, &m_grProg->getShaderGroupHandles()[0], shaderHandleSize);
	sbt += sbtRecordSize;
	memcpy(sbt + sbtRecordSize, &m_grProg->getShaderGroupHandles()[shaderHandleSize], shaderHandleSize);
	sbt += sbtRecordSize;

	// Create the instances. Allocate but not construct to save some CPU time
	void* instancesMem = ctx.m_tempAllocator.getMemoryPool().allocate(
		sizeof(AccelerationStructureInstance) * instanceCount, alignof(AccelerationStructureInstance));
	WeakArray<AccelerationStructureInstance> instances(static_cast<AccelerationStructureInstance*>(instancesMem),
													   instanceCount);

	// Init SBT and instances
	ANKI_ASSERT(sbtRecordSize >= shaderHandleSize + sizeof(ModelGpuDescriptor));
	for(U32 instanceIdx = 0; instanceIdx < instanceCount; ++instanceIdx)
	{
		const RayTracingInstanceQueueElement& element = instanceElements[instanceIdx];

		// Init instance
		AccelerationStructureInstance& out = instances[instanceIdx];
		::new(&out) AccelerationStructureInstance();
		out.m_bottomLevel.reset(element.m_bottomLevelAccelerationStructure);
		memcpy(&out.m_transform, &element.m_modelDescriptor.m_worldTransform[0], sizeof(out.m_transform));
		out.m_sbtRecordIndex = instanceIdx + extraSbtRecords; // Add the raygen and miss
		out.m_mask = 0xFF;

		// Init SBT record
		memcpy(sbt, element.m_shaderGroupHandles[RayType::SHADOWS], shaderHandleSize);
		memcpy(sbt + shaderHandleSize, &element.m_modelDescriptor, sizeof(element.m_modelDescriptor));
		sbt += sbtRecordSize;
	}

	ANKI_ASSERT(sbtStart + sbtRecordSize * (instanceCount + extraSbtRecords) == sbt);

	// Create the TLAS
	AccelerationStructureInitInfo initInf("RtShadows");
	initInf.m_type = AccelerationStructureType::TOP_LEVEL;
	initInf.m_topLevel.m_instances = instances;
	m_runCtx.m_tlas = getGrManager().newAccelerationStructure(initInf);

	// Need a cleanup
	for(U32 instanceIdx = 0; instanceIdx < instanceCount; ++instanceIdx)
	{
		instances[instanceIdx].m_bottomLevel.reset(nullptr);
	}
}

} // end namespace anki
