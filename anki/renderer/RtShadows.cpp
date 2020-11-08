// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/RtShadows.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Renderer.h>
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
	// TODO
}

void RtShadows::buildSbtAndTlas(BufferPtr& sbtBuffer, PtrSize& sbtOffset, AccelerationStructurePtr& tlas)
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

	// Allocate SBT
	StagingGpuMemoryToken token;
	U8* sbt = allocateStorage<U8*>(sbtRecordSize * (instanceCount + extraSbtRecords), token);
	const U8* sbtStart = sbt;
	(void)sbtStart;
	sbtBuffer = token.m_buffer;
	sbtOffset = token.m_offset;

	// Set the miss and ray gen handles
	memcpy(sbt, &m_grProg->getShaderGroupHandles()[0], shaderHandleSize);
	sbt += sbtRecordSize;
	memcpy(sbt + sbtRecordSize, &m_grProg->getShaderGroupHandles()[sbtRecordSize], shaderHandleSize);
	sbt += sbtRecordSize;

	// Create the instances
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
		out.m_bottomLevel = AccelerationStructurePtr(element.m_bottomLevelAccelerationStructure);
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
	tlas = getGrManager().newAccelerationStructure(initInf);
}

} // end namespace anki
