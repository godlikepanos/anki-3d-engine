// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/GpuParticleEmitterNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/GenericGpuComputeJobComponent.h>
#include <anki/resource/ResourceManager.h>
#include <shaders/glsl_cpp_common/GpuParticles.h>

namespace anki
{

class GpuParticleEmitterNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			GpuParticleEmitterNode& mnode = static_cast<GpuParticleEmitterNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

Error GpuParticleEmitterNode::init(const CString& filename)
{
	// Create program
	ANKI_CHECK(getResourceManager().loadResource("shaders/GpuParticles.glslp", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	// Load particle props
	ANKI_CHECK(getResourceManager().loadResource(filename, m_emitterRsrc));

	// Create a UBO with the props
	BufferInitInfo buffInit;
	buffInit.m_access = BufferMapAccessBit::WRITE;
	buffInit.m_usage = BufferUsageBit::UNIFORM_COMPUTE;
	buffInit.m_size = sizeof(GpuParticleEmitterProperties);
	m_propsBuff = getSceneGraph().getGrManager().newBuffer(buffInit);
	GpuParticleEmitterProperties* props =
		static_cast<GpuParticleEmitterProperties*>(m_propsBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));

	const ParticleEmitterProperties& inProps = m_emitterRsrc->getProperties();
	props->m_minGravity = inProps.m_particle.m_minGravity;
	props->m_minMass = inProps.m_particle.m_minMass;
	props->m_maxGravity = inProps.m_particle.m_maxGravity;
	props->m_maxMass = inProps.m_particle.m_maxMass;
	props->m_minForce = inProps.m_particle.m_minForceDirection * inProps.m_particle.m_minForceMagnitude;
	props->m_minLife = inProps.m_particle.m_minLife;
	props->m_maxForce = inProps.m_particle.m_maxForceDirection * inProps.m_particle.m_maxForceMagnitude;
	props->m_maxLife = inProps.m_particle.m_maxLife;
	props->m_particleCount = inProps.m_maxNumOfParticles;

	m_propsBuff->unmap();

	// Create the particle buffer
	buffInit.m_access = BufferMapAccessBit::WRITE;
	buffInit.m_usage = BufferUsageBit::STORAGE_ALL;
	buffInit.m_size = sizeof(GpuParticle) * inProps.m_maxNumOfParticles;
	m_particlesBuff = getSceneGraph().getGrManager().newBuffer(buffInit);
	GpuParticle* particle = static_cast<GpuParticle*>(m_particlesBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));
	const GpuParticle* end = particle + inProps.m_maxNumOfParticles;
	for(; particle < end; ++particle)
	{
		particle->m_life = -1.0f; // Force GPU to init the particle
	}

	m_particlesBuff->unmap();

	// Find the extend of the particles
	{
		const Vec3 maxForce = inProps.m_particle.m_maxForceDirection * inProps.m_particle.m_maxForceMagnitude;
		const Vec3& maxGravity = inProps.m_particle.m_maxGravity;
		const F32 maxMass = inProps.m_particle.m_maxMass;
		const F32 maxTime = inProps.m_particle.m_maxLife;

		const Vec3 totalForce = maxGravity * maxMass + maxForce;
		const Vec3 accelleration = totalForce / maxMass;
		const Vec3 velocity = accelleration * maxTime;
		m_maxDistanceAParticleCanGo = (velocity * maxTime).getLength();
	}

	// Create the components
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<SpatialComponent>(this, &m_spatialVolume);
	GenericGpuComputeJobComponent* gpuComp = newComponent<GenericGpuComputeJobComponent>();
	gpuComp->setCallback(
		[](GenericGpuComputeJobQueueElementContext& ctx, const void* userData) {
			static_cast<const GpuParticleEmitterNode*>(userData)->simulate(ctx.m_commandBuffer);
		},
		this);

	return Error::NONE;
}

void GpuParticleEmitterNode::simulate(CommandBufferPtr& cmdb) const
{
	// TODO
}

} // end namespace anki