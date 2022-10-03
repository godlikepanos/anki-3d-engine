// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/GpuParticleEmitterComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/RenderComponent.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Shaders/Include/ParticleTypes.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(GpuParticleEmitterComponent)

GpuParticleEmitterComponent::GpuParticleEmitterComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
}

GpuParticleEmitterComponent::~GpuParticleEmitterComponent()
{
}

Error GpuParticleEmitterComponent::loadParticleEmitterResource(CString filename)
{
	m_markedForUpdate = true;

	// Create the debug drawer
	if(!m_dbgImage.isCreated())
	{
		ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource("EngineAssets/ParticleEmitter.ankitex",
																			 m_dbgImage));
	}

	// Load particle props
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(filename, m_particleEmitterResource));
	const ParticleEmitterProperties& inProps = m_particleEmitterResource->getProperties();
	m_maxParticleCount = inProps.m_maxNumOfParticles;

	// Create program
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(
		"ShaderBinaries/GpuParticlesSimulation.ankiprogbin", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();
	m_workgroupSizeX = variant->getWorkgroupSizes()[0];

	// Create a UBO with the props
	{
		BufferInitInfo buffInit("GpuParticlesProps");
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_usage = BufferUsageBit::kUniformCompute;
		buffInit.m_size = sizeof(GpuParticleEmitterProperties);
		m_propsBuff = m_node->getSceneGraph().getGrManager().newBuffer(buffInit);
		GpuParticleEmitterProperties* props =
			static_cast<GpuParticleEmitterProperties*>(m_propsBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

		props->m_minGravity = inProps.m_particle.m_minGravity;
		props->m_minMass = inProps.m_particle.m_minMass;
		props->m_maxGravity = inProps.m_particle.m_maxGravity;
		props->m_maxMass = inProps.m_particle.m_maxMass;
		props->m_minForce = inProps.m_particle.m_minForceDirection * inProps.m_particle.m_minForceMagnitude;
		props->m_minLife = F32(inProps.m_particle.m_minLife);
		props->m_maxForce = inProps.m_particle.m_maxForceDirection * inProps.m_particle.m_maxForceMagnitude;
		props->m_maxLife = F32(inProps.m_particle.m_maxLife);
		props->m_minStartingPosition = inProps.m_particle.m_minStartingPosition;
		props->m_maxStartingPosition = inProps.m_particle.m_maxStartingPosition;
		props->m_particleCount = inProps.m_maxNumOfParticles;

		m_propsBuff->flush(0, kMaxPtrSize);
		m_propsBuff->unmap();
	}

	// Create the particle buffer
	{
		BufferInitInfo buffInit("GpuParticles");
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_usage = BufferUsageBit::kAllStorage;
		buffInit.m_size = sizeof(GpuParticle) * m_maxParticleCount;
		m_particlesBuff = m_node->getSceneGraph().getGrManager().newBuffer(buffInit);

		GpuParticle* particle =
			static_cast<GpuParticle*>(m_particlesBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
		const GpuParticle* end = particle + m_maxParticleCount;
		for(; particle < end; ++particle)
		{
			particle->m_life = -1.0f; // Force GPU to init the particle
		}

		m_particlesBuff->flush(0, kMaxPtrSize);
		m_particlesBuff->unmap();
	}

	// Create the rand buffer
	{
		BufferInitInfo buffInit("GpuParticlesRand");
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_usage = BufferUsageBit::kAllStorage;
		buffInit.m_size = sizeof(U32) + kMaxRandFactors * sizeof(F32);
		m_randFactorsBuff = m_node->getSceneGraph().getGrManager().newBuffer(buffInit);

		F32* randFactors = static_cast<F32*>(m_randFactorsBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

		*reinterpret_cast<U32*>(randFactors) = kMaxRandFactors;
		++randFactors;

		const F32* randFactorsEnd = randFactors + kMaxRandFactors;
		for(; randFactors < randFactorsEnd; ++randFactors)
		{
			*randFactors = getRandomRange(0.0f, 1.0f);
		}

		m_randFactorsBuff->flush(0, kMaxPtrSize);
		m_randFactorsBuff->unmap();
	}

	// Create the sampler
	{
		SamplerInitInfo sinit("GpuParticles");
		sinit.m_addressing = SamplingAddressing::kClamp;
		m_nearestAnyClampSampler = m_node->getSceneGraph().getGrManager().newSampler(sinit);
	}

	// Find the extend of the particles
	if(inProps.m_emitterBoundingVolumeMin.x() >= inProps.m_emitterBoundingVolumeMax.x())
	{
		const Vec3 maxForce = inProps.m_particle.m_maxForceDirection * inProps.m_particle.m_maxForceMagnitude;
		const Vec3& maxGravity = inProps.m_particle.m_maxGravity;
		const F32 maxMass = inProps.m_particle.m_maxMass;
		const F32 maxLife = F32(inProps.m_particle.m_maxLife);

		const F32 dt = 1.0f / 30.0f;
		const Vec3 initialForce = maxGravity * maxMass + maxForce;
		const Vec3 initialAcceleration = initialForce / maxMass;
		const Vec3 velocity = initialAcceleration * dt;

		F32 life = maxLife;
		Vec3 pos(0.0f);
		while(life > dt)
		{
			pos = maxGravity * (dt * dt) + velocity * dt + pos;
			life -= dt;
		}

		m_emitterBoundingBoxLocal = Aabb(-Vec3(pos.getLength()), Vec3(pos.getLength()));
	}
	else
	{
		m_emitterBoundingBoxLocal = Aabb(inProps.m_emitterBoundingVolumeMin, inProps.m_emitterBoundingVolumeMax);
	}

	return Error::kNone;
}

void GpuParticleEmitterComponent::simulate(GenericGpuComputeJobQueueElementContext& ctx) const
{
	if(ANKI_UNLIKELY(!m_particleEmitterResource.isCreated()))
	{
		return;
	}

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

	// Bind resources
	cmdb->bindStorageBuffer(1, 0, m_particlesBuff, 0, kMaxPtrSize);
	cmdb->bindUniformBuffer(1, 1, m_propsBuff, 0, kMaxPtrSize);
	cmdb->bindStorageBuffer(1, 2, m_randFactorsBuff, 0, kMaxPtrSize);
	cmdb->bindSampler(1, 3, m_nearestAnyClampSampler);

	StagingGpuMemoryToken token;
	GpuParticleSimulationState* unis =
		static_cast<GpuParticleSimulationState*>(ctx.m_stagingGpuAllocator->allocateFrame(
			sizeof(GpuParticleSimulationState), StagingGpuMemoryType::kUniform, token));

	unis->m_viewProjMat = ctx.m_viewProjectionMatrix;
	unis->m_unprojectionParams = ctx.m_projectionMatrix.extractPerspectiveUnprojectionParams();
	unis->m_randomIndex = rand();
	unis->m_dt = F32(m_dt);
	unis->m_emitterPosition = m_worldPosition;
	unis->m_emitterRotation = m_worldRotation;
	unis->m_invViewRotation = Mat3x4(Vec3(0.0f), ctx.m_cameraTransform.getRotationPart());
	cmdb->bindUniformBuffer(1, 4, token.m_buffer, token.m_offset, token.m_range);

	// Dispatch
	const U32 workgroupCount = (m_maxParticleCount + m_workgroupSizeX - 1) / m_workgroupSizeX;
	cmdb->dispatchCompute(workgroupCount, 1, 1);
}

void GpuParticleEmitterComponent::draw(RenderQueueDrawContext& ctx) const
{
	if(ANKI_UNLIKELY(!m_particleEmitterResource.isCreated()))
	{
		return;
	}

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(!ctx.m_debugDraw)
	{
		// Program
		ShaderProgramPtr prog;
		m_particleEmitterResource->getRenderingInfo(ctx.m_key, prog);
		cmdb->bindShaderProgram(prog);

		// Resources
		static const Array<Mat3x4, 1> identity = {Mat3x4::getIdentity()};

		RenderComponent::allocateAndSetupUniforms(m_particleEmitterResource->getMaterial(), ctx, identity, identity,
												  *ctx.m_stagingGpuAllocator);

		cmdb->bindStorageBuffer(kMaterialSetLocal, kMaterialBindingFirstNonStandardLocal, m_particlesBuff, 0,
								kMaxPtrSize);

		// Draw
		cmdb->setLineWidth(8.0f);
		cmdb->drawArrays(PrimitiveTopology::kLines, m_maxParticleCount * 2);
	}
	else
	{
		const Vec4 tsl = (m_worldAabb.getMin() + m_worldAabb.getMax()) / 2.0f;
		const Vec4 scale = (m_worldAabb.getMax() - m_worldAabb.getMin()) / 2.0f;

		// Set non uniform scale. Add a margin to avoid flickering
		Mat3 nonUniScale = Mat3::getZero();
		nonUniScale(0, 0) = scale.x();
		nonUniScale(1, 1) = scale.y();
		nonUniScale(2, 2) = scale.z();

		const Mat4 mvp = ctx.m_viewProjectionMatrix * Mat4(tsl.xyz1(), Mat3::getIdentity() * nonUniScale, 1.0f);

		const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDepthTestOn);
		if(enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::kLess);
		}
		else
		{
			cmdb->setDepthCompareOperation(CompareOperation::kAlways);
		}

		m_node->getSceneGraph().getDebugDrawer().drawCubes(
			ConstWeakArray<Mat4>(&mvp, 1), Vec4(1.0f, 0.0f, 1.0f, 1.0f), 2.0f,
			ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), 2.0f, *ctx.m_stagingGpuAllocator,
			cmdb);

		m_node->getSceneGraph().getDebugDrawer().drawBillboardTextures(
			ctx.m_projectionMatrix, ctx.m_viewMatrix, ConstWeakArray<Vec3>(&m_worldPosition, 1), Vec4(1.0f),
			ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), m_dbgImage->getTextureView(),
			ctx.m_sampler, Vec2(0.75f), *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

		// Restore state
		if(!enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::kLess);
		}
	}
}

Error GpuParticleEmitterComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(ANKI_UNLIKELY(!m_particleEmitterResource.isCreated()))
	{
		updated = false;
		return Error::kNone;
	}

	updated = m_markedForUpdate;
	m_markedForUpdate = false;

	m_dt = info.m_dt;

	return Error::kNone;
}

void GpuParticleEmitterComponent::setWorldTransform(const Transform& trf)
{
	m_worldPosition = trf.getOrigin().xyz();
	m_worldRotation = trf.getRotation();

	m_worldAabb.setMin(m_worldPosition + m_emitterBoundingBoxLocal.getMin().xyz());
	m_worldAabb.setMax(m_worldPosition + m_emitterBoundingBoxLocal.getMax().xyz());

	m_markedForUpdate = true;
}

} // end namespace anki
