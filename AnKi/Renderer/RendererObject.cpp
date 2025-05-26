// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>

namespace anki {

Renderer& RendererObject::getRenderer()
{
	return Renderer::getSingleton();
}

void RendererObject::registerDebugRenderTarget(CString rtName)
{
	getRenderer().registerDebugRenderTarget(this, rtName);
}

Error RendererObject::loadShaderProgram(CString filename, ConstWeakArray<SubMutation> mutators, ShaderProgramResourcePtr& rsrc,
										ShaderProgramPtr& grProg, CString technique, ShaderTypeBit shaderTypes)
{
	if(!rsrc.isCreated())
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(filename, rsrc));
	}

	ShaderProgramResourceVariantInitInfo initInf(rsrc);
	for(SubMutation pair : mutators)
	{
		initInf.addMutation(pair.m_mutatorName, pair.m_value);
	}

	if(technique.isEmpty())
	{
		technique = "Unnamed";
	}

	if(!shaderTypes)
	{
		U32 techniqueIdx = kMaxU32;
		for(U32 i = 0; i < rsrc->getBinary().m_techniques.getSize(); ++i)
		{
			if(technique == rsrc->getBinary().m_techniques[i].m_name.getBegin())
			{
				techniqueIdx = i;
				break;
			}
		}
		ANKI_ASSERT(techniqueIdx != kMaxU32);
		const ShaderTypeBit techniqueShaderTypes = rsrc->getBinary().m_techniques[techniqueIdx].m_shaderTypes;

		if(techniqueShaderTypes == (ShaderTypeBit::kCompute | ShaderTypeBit::kPixel | ShaderTypeBit::kVertex))
		{
			if(g_preferComputeCVar)
			{
				shaderTypes = ShaderTypeBit::kCompute;
			}
			else
			{
				shaderTypes = ShaderTypeBit::kPixel | ShaderTypeBit::kVertex;
			}
		}
		else if(techniqueShaderTypes == ShaderTypeBit::kCompute)
		{
			shaderTypes = techniqueShaderTypes;
		}
		else if(techniqueShaderTypes == (ShaderTypeBit::kPixel | ShaderTypeBit::kVertex))
		{
			shaderTypes = techniqueShaderTypes;
		}
		else
		{
			ANKI_ASSERT(!"Can't figure out a sensible default");
		}
	}

	initInf.requestTechniqueAndTypes(shaderTypes, technique);

	const ShaderProgramResourceVariant* variant;
	rsrc->getOrCreateVariant(initInf, variant);

	if(variant)
	{
		grProg.reset(&variant->getProgram());
	}
	else
	{
		grProg.reset(nullptr);
	}

	return Error::kNone;
}

void RendererObject::zeroBuffer(Buffer* buff)
{
	CommandBufferInitInfo cmdbInit("Zero buffer");
	cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

	cmdb->zeroBuffer(BufferView(buff));

	FencePtr fence;
	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);

	fence->clientWait(16.0_sec);
}

CString RendererObject::generateTempPassName(const Char* fmt, ...)
{
	Array<Char, 128> buffer;

	va_list args;
	va_start(args, fmt);
	const I len = vsnprintf(&buffer[0], sizeof(buffer), fmt, args);
	va_end(args);

	if(len > 0 && len < I(sizeof(buffer)))
	{
		Char* str = static_cast<Char*>(getRenderer().getFrameMemoryPool().allocate(len + 1, 1));
		memcpy(str, buffer.getBegin(), len + 1);
		return str;
	}
	else
	{
		ANKI_R_LOGE("generateTempPassName() failed. Ignoring error");
		return "**failed**";
	}
}

void RendererObject::fillBuffers(CommandBuffer& cmdb, ConstWeakArray<BufferView> buffers, U32 value)
{
	ANKI_ASSERT(buffers.getSize() > 0);

	cmdb.pushDebugMarker("fillBuffers", Vec3(1.0, 1.0, 0.0));

	cmdb.bindShaderProgram(&getRenderer().getFillBufferProgram());

	for(const BufferView& view : buffers)
	{
		cmdb.bindUav(0, 0, BufferView(&view.getBuffer()));

		ANKI_ASSERT(view.getOffset() % sizeof(U32) == 0);
		ANKI_ASSERT(view.getRange() % sizeof(U32) == 0);
		const UVec4 consts(value, U32(view.getOffset() / sizeof(U32)), U32(view.getRange() / sizeof(U32)), 0);
		cmdb.setFastConstants(&consts, sizeof(consts));

		ANKI_ASSERT(view.getRange() % sizeof(U32) == 0);
		const U32 elementCount = U32(view.getRange() / sizeof(U32));
		cmdb.dispatchCompute((elementCount + 63) / 64, 1, 1);
	}

	cmdb.popDebugMarker();
}

Error RtMaterialFetchRendererObject::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtBuildProg, m_sbtBuildGrProg, "Build"));
	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtBuildProg, m_sbtPatchGrProg, "PatchRaygenAndMiss"));
	return Error::kNone;
}

void RtMaterialFetchRendererObject::buildShaderBindingTablePass(CString passName, ShaderProgram* library, U32 raygenHandleIdx, U32 missHandleIdx,
																U32 sbtRecordSize, RenderGraphBuilder& rgraph, BufferHandle& sbtHandle,
																BufferView& sbtBuffer)
{
	BufferHandle visibilityDep;
	BufferView visibleRenderableIndicesBuff, buildSbtIndirectArgsBuff;
	getRenderer().getAccelerationStructureBuilder().getVisibilityInfo(visibilityDep, visibleRenderableIndicesBuff, buildSbtIndirectArgsBuff);

	// Allocate SBT
	U32 sbtAlignment = (GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
						   ? sizeof(U32)
						   : GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
	sbtAlignment = computeCompoundAlignment(sbtAlignment, GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment);

	sbtBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(
		(GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount() + 2) * sbtRecordSize, sbtAlignment);
	sbtHandle = rgraph.importBuffer(sbtBuffer, BufferUsageBit::kNone);

	// Create the pass
	NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass(passName);

	rpass.newBufferDependency(visibilityDep, BufferUsageBit::kIndirectCompute | BufferUsageBit::kSrvCompute);
	rpass.newBufferDependency(sbtHandle, BufferUsageBit::kUavCompute);

	rpass.setWork([this, buildSbtIndirectArgsBuff, sbtBuffer, visibleRenderableIndicesBuff, lib = ShaderProgramPtr(library), sbtRecordSize,
				   raygenHandleIdx, missHandleIdx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(btBuild);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_sbtBuildGrProg.get());

		cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
		cmdb.bindSrv(1, 0, visibleRenderableIndicesBuff);
		cmdb.bindSrv(2, 0, BufferView(&lib->getShaderGroupHandlesGpuBuffer()));

		cmdb.bindUav(0, 0, sbtBuffer);

		RtShadowsSbtBuildConstants consts = {};
		ANKI_ASSERT(sbtRecordSize % 4 == 0);
		consts.m_sbtRecordDwordSize = sbtRecordSize / 4;
		const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
		ANKI_ASSERT(shaderHandleSize % 4 == 0);
		consts.m_shaderHandleDwordSize = shaderHandleSize / 4;
		consts.m_raygenHandleIndex = raygenHandleIdx;
		consts.m_missHandleIndex = missHandleIdx;
		cmdb.setFastConstants(&consts, sizeof(consts));

		cmdb.dispatchComputeIndirect(buildSbtIndirectArgsBuff);
	});
}

void RtMaterialFetchRendererObject::patchShaderBindingTablePass(CString passName, ShaderProgram* library, U32 raygenHandleIdx, U32 missHandleIdx,
																U32 sbtRecordSize, RenderGraphBuilder& rgraph, BufferHandle sbtHandle,
																BufferView sbtBuffer)
{
	NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass(passName);

	rpass.newBufferDependency(sbtHandle, BufferUsageBit::kUavCompute);

	rpass.setWork(
		[this, sbtBuffer, lib = ShaderProgramPtr(library), sbtRecordSize, raygenHandleIdx, missHandleIdx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(btBuild);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_sbtPatchGrProg.get());

			cmdb.bindSrv(0, 0, BufferView(&lib->getShaderGroupHandlesGpuBuffer()));

			cmdb.bindUav(0, 0, sbtBuffer);

			RtShadowsSbtBuildConstants consts = {};
			ANKI_ASSERT(sbtRecordSize % 4 == 0);
			consts.m_sbtRecordDwordSize = sbtRecordSize / 4;
			const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
			ANKI_ASSERT(shaderHandleSize % 4 == 0);
			consts.m_shaderHandleDwordSize = shaderHandleSize / 4;
			consts.m_raygenHandleIndex = raygenHandleIdx;
			consts.m_missHandleIndex = missHandleIdx;
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchCompute(1, 1, 1);
		});
}

} // end namespace anki
