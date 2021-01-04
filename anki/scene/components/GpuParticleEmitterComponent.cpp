// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/GpuParticleEmitterComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/resource/ResourceManager.h>
#include <anki/shaders/include/ParticleTypes.h>

namespace anki
{

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

	// Load particle props
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(filename, m_particleEmitterResource));
	const ParticleEmitterProperties& inProps = m_particleEmitterResource->getProperties();
	m_maxParticleCount = inProps.m_maxNumOfParticles;

	// Create program
	ANKI_CHECK(
		m_node->getSceneGraph().getResourceManager().loadResource("shaders/GpuParticlesSimulation.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();
	m_workgroupSizeX = variant->getWorkgroupSizes()[0];

	// Create a UBO with the props
	{
		BufferInitInfo buffInit("GpuParticlesProps");
		buffInit.m_mapAccess = BufferMapAccessBit::WRITE;
		buffInit.m_usage = BufferUsageBit::UNIFORM_COMPUTE;
		buffInit.m_size = sizeof(GpuParticleEmitterProperties);
		m_propsBuff = m_node->getSceneGraph().getGrManager().newBuffer(buffInit);
		GpuParticleEmitterProperties* props =
			static_cast<GpuParticleEmitterProperties*>(m_propsBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));

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

		m_propsBuff->unmap();
	}

	// Create the particle buffer
	{
		BufferInitInfo buffInit("GpuParticles");
		buffInit.m_mapAccess = BufferMapAccessBit::WRITE;
		buffInit.m_usage = BufferUsageBit::ALL_STORAGE;
		buffInit.m_size = sizeof(GpuParticle) * m_maxParticleCount;
		m_particlesBuff = m_node->getSceneGraph().getGrManager().newBuffer(buffInit);

		GpuParticle* particle =
			static_cast<GpuParticle*>(m_particlesBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));
		const GpuParticle* end = particle + m_maxParticleCount;
		for(; particle < end; ++particle)
		{
			particle->m_life = -1.0f; // Force GPU to init the particle
		}

		m_particlesBuff->unmap();
	}

	// Create the rand buffer
	{
		BufferInitInfo buffInit("GpuParticlesRand");
		buffInit.m_mapAccess = BufferMapAccessBit::WRITE;
		buffInit.m_usage = BufferUsageBit::ALL_STORAGE;
		buffInit.m_size = sizeof(U32) + MAX_RAND_FACTORS * sizeof(F32);
		m_randFactorsBuff = m_node->getSceneGraph().getGrManager().newBuffer(buffInit);

		F32* randFactors = static_cast<F32*>(m_randFactorsBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));

		*reinterpret_cast<U32*>(randFactors) = MAX_RAND_FACTORS;
		++randFactors;

		const F32* randFactorsEnd = randFactors + MAX_RAND_FACTORS;
		for(; randFactors < randFactorsEnd; ++randFactors)
		{
			*randFactors = getRandomRange(0.0f, 1.0f);
		}

		m_randFactorsBuff->unmap();
	}

	// Create the sampler
	{
		SamplerInitInfo sinit("GpuParticles");
		sinit.m_addressing = SamplingAddressing::CLAMP;
		m_nearestAnyClampSampler = m_node->getSceneGraph().getGrManager().newSampler(sinit);
	}

	// Find the extend of the particles
	{
		const Vec3 maxForce = inProps.m_particle.m_maxForceDirection * inProps.m_particle.m_maxForceMagnitude;
		const Vec3& maxGravity = inProps.m_particle.m_maxGravity;
		const F32 maxMass = inProps.m_particle.m_maxMass;
		const F32 maxTime = F32(inProps.m_particle.m_maxLife);

		const Vec3 totalForce = maxGravity * maxMass + maxForce;
		const Vec3 accelleration = totalForce / maxMass;
		const Vec3 velocity = accelleration * maxTime;
		m_maxDistanceAParticleCanGo = (velocity * maxTime).getLength();
	}

	return Error::NONE;
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
	cmdb->bindStorageBuffer(1, 0, m_particlesBuff, 0, MAX_PTR_SIZE);
	cmdb->bindUniformBuffer(1, 1, m_propsBuff, 0, MAX_PTR_SIZE);
	cmdb->bindStorageBuffer(1, 2, m_randFactorsBuff, 0, MAX_PTR_SIZE);
	cmdb->bindSampler(1, 3, m_nearestAnyClampSampler);

	StagingGpuMemoryToken token;
	GpuParticleSimulationState* unis =
		static_cast<GpuParticleSimulationState*>(ctx.m_stagingGpuAllocator->allocateFrame(
			sizeof(GpuParticleSimulationState), StagingGpuMemoryType::UNIFORM, token));

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
		static const Mat4 identity = Mat4::getIdentity();

		RenderComponent::allocateAndSetupUniforms(m_particleEmitterResource->getMaterial(), ctx,
												  ConstWeakArray<Mat4>(&identity, 1),
												  ConstWeakArray<Mat4>(&identity, 1), *ctx.m_stagingGpuAllocator);

		cmdb->bindStorageBuffer(0, 1, m_particlesBuff, 0, MAX_PTR_SIZE);

		StagingGpuMemoryToken token;
		Vec4* extraUniforms = static_cast<Vec4*>(
			ctx.m_stagingGpuAllocator->allocateFrame(sizeof(Vec4), StagingGpuMemoryType::UNIFORM, token));
		*extraUniforms = ctx.m_cameraTransform.getColumn(2);
		cmdb->bindUniformBuffer(0, 2, token.m_buffer, token.m_offset, token.m_range);

		// Draw
		cmdb->setLineWidth(8.0f);
		cmdb->drawArrays(PrimitiveTopology::LINES, m_maxParticleCount * 2);
	}
	else
	{
		// TODO
	}
}

Error GpuParticleEmitterComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	if(ANKI_UNLIKELY(!m_particleEmitterResource.isCreated()))
	{
		updated = false;
		return Error::NONE;
	}

	updated = m_markedForUpdate;
	m_markedForUpdate = false;

	m_dt = crntTime - prevTime;

	return Error::NONE;
}

void GpuParticleEmitterComponent::setWorldTransform(const Transform& trf)
{
	m_worldPosition = trf.getOrigin().xyz();
	m_worldRotation = trf.getRotation();

	m_worldAabb.setMin(m_worldPosition - m_maxDistanceAParticleCanGo);
	m_worldAabb.setMax(m_worldPosition + m_maxDistanceAParticleCanGo);

	m_markedForUpdate = true;
}

} // end namespace anki
