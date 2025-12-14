// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GpuParticles.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Resource/ParticleEmitterResource2.h>
#include <AnKi/Scene/Components/ParticleEmitter2Component.h>

namespace anki {

Error GpuParticles::init()
{
	ParticleSimulationScratch scratchTemplate = {};
	scratchTemplate.m_aabbMin = IVec3(kMaxI32);
	scratchTemplate.m_aabbMax = IVec3(kMinI32);

	m_scratchBuffers.resize(kMaxEmittersToSimulate);

	CommandBufferInitInfo cmdbInit("Init particles");
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

	U32 count = 0;
	for(auto& buff : m_scratchBuffers)
	{
		BufferInitInfo buffInit(generateTempPassName("GPU particles scratch #%u", count++));
		buffInit.m_size = sizeof(ParticleSimulationScratch);
		buffInit.m_usage = BufferUsageBit::kSrvCompute | BufferUsageBit::kCopyDestination;
		buff = GrManager::getSingleton().newBuffer(buffInit);

		WeakArray<ParticleSimulationScratch> out;
		const BufferView src = RebarTransientMemoryPool::getSingleton().allocateCopyBuffer<ParticleSimulationScratch>(1, out);
		out[0] = scratchTemplate;

		cmdb->copyBufferToBuffer(src, BufferView(buff.get()));
	}

	cmdb->endRecording();
	FencePtr fence;
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
	fence->clientWait(kMaxSecond);

	return Error::kNone;
}

void GpuParticles::populateRenderGraph(RenderingContext& ctx)
{
	SceneBlockArray<ParticleEmitter2Component>& emitters = SceneGraph::getSingleton().getComponentArrays().getParticleEmitter2s();
	if(emitters.getSize() == 0) [[unlikely]]
	{
		return;
	}

	// Create the renderpass
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("GPU particle sim");

	pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
	pass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kUavCompute);

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		SceneBlockArray<ParticleEmitter2Component>& emitters = SceneGraph::getSingleton().getComponentArrays().getParticleEmitter2s();

		// Handle the readbacks
		for(ParticleEmitter2Component& emitter : emitters)
		{
			if(!emitter.isValid())
			{
				continue;
			}

			auto it = m_readbacks.find(emitter.getUuid());
			if(it != m_readbacks.getEnd())
			{
				// Emitter found, access the readback
				it->m_lastFrameSeen = getRenderer().getFrameCount();

				ParticleSimulationCpuFeedback feedback;
				PtrSize dataOut = 0;
				getRenderer().getReadbackManager().readMostRecentData(it->m_readback, &feedback, sizeof(feedback), dataOut);

				if(dataOut && feedback.m_aabbMin < feedback.m_aabbMax)
				{
					ANKI_ASSERT(feedback.m_uuid == emitter.getUuid());
					emitter.updateBoundingVolume(feedback.m_aabbMin, feedback.m_aabbMax);
				}
			}
			else
			{
				// Emitter not found, create new entry
				ReadbackData& data = *m_readbacks.emplace(emitter.getUuid());
				data.m_lastFrameSeen = getRenderer().getFrameCount();
			}
		}

		// Remove old emitters
		while(true)
		{
			Bool foundAnOldOne = false;
			for(auto it = m_readbacks.getBegin(); it != m_readbacks.getEnd(); ++it)
			{
				const U32 deleteAfterNFrames = 10;
				if(it->m_lastFrameSeen + deleteAfterNFrames < getRenderer().getFrameCount())
				{
					m_readbacks.erase(it);
					foundAnOldOne = true;
				}
			}

			if(!foundAnOldOne)
			{
				break;
			}
		}

		// Compute work
		U32 count = 0;
		for(ParticleEmitter2Component& emitter : emitters)
		{
			if(!emitter.isValid())
			{
				continue;
			}

			cmdb.pushDebugMarker(generateTempPassName("Emitter %u", emitter.getUuid()), Vec3(1.0f, 1.0f, 0.0f));

			const ParticleEmitterResource2& rsrc = emitter.getParticleEmitterResource();
			const ParticleEmitterResourceCommonProperties& commonProps = rsrc.getCommonProperties();

			auto it = m_readbacks.find(emitter.getUuid());
			ANKI_ASSERT(it != m_readbacks.getEnd());
			const BufferView readbackBuffer =
				getRenderer().getReadbackManager().allocateStructuredBuffer<ParticleSimulationCpuFeedback>(it->m_readback, 1);

			cmdb.bindShaderProgram(&rsrc.getShaderProgram());

			ParticleSimulationConstants consts;
			consts.m_viewProjMat = ctx.m_matrices.m_viewProjection;
			consts.m_invertedViewProjMat = ctx.m_matrices.m_invertedViewProjection;
			consts.m_unprojectionParams = ctx.m_matrices.m_unprojectionParameters;
			consts.m_randomNumber = getRandom() % kMaxU32;
			consts.m_dt = emitter.getDt();
			consts.m_gpuSceneParticleEmitterIndex = emitter.getGpuSceneParticleEmitter2Index();
			*allocateAndBindConstants<ParticleSimulationConstants>(cmdb, ANKI_PARTICLE_SIM_CONSTANTS, 0) = consts;

			rgraphCtx.bindSrv(ANKI_PARTICLE_SIM_DEPTH_BUFFER, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(ANKI_PARTICLE_SIM_NORMAL_BUFFER, 0, getGBuffer().getColorRt(2));
			cmdb.bindSrv(ANKI_PARTICLE_SIM_GPU_SCENE_TRANSFORMS, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

			cmdb.bindUav(ANKI_PARTICLE_SIM_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());
			cmdb.bindUav(ANKI_PARTICLE_SIM_GPU_SCENE_PARTICLE_EMITTERS, 0, GpuSceneArrays::ParticleEmitter2::getSingleton().getBufferView());
			cmdb.bindUav(ANKI_PARTICLE_SIM_SCRATCH, 0, BufferView(m_scratchBuffers[count++].get()));
			cmdb.bindUav(ANKI_PARTICLE_SIM_CPU_FEEDBACK, 0, readbackBuffer);

			const U32 numthreads = GrManager::getSingleton().getDeviceCapabilities().m_maxWaveSize;
			cmdb.dispatchCompute((commonProps.m_particleCount + numthreads - 1) / numthreads, 1, 1);

			cmdb.popDebugMarker();
		}
	});
}

} // end namespace anki
