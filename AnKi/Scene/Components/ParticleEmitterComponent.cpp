// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ParticleEmitterComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Resource/ParticleEmitterResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Math.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

static Vec3 getRandom(const Vec3& min, const Vec3& max)
{
	Vec3 out;
	out.x() = mix(min.x(), max.x(), getRandomRange(0.0f, 1.0f));
	out.y() = mix(min.y(), max.y(), getRandomRange(0.0f, 1.0f));
	out.z() = mix(min.z(), max.z(), getRandomRange(0.0f, 1.0f));
	return out;
}

/// Particle base
class ParticleEmitterComponent::ParticleBase
{
public:
	Second m_timeOfBirth; ///< Keep the time of birth for nice effects
	Second m_timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead

	F32 m_initialSize;
	F32 m_finalSize;
	F32 m_crntSize;

	F32 m_initialAlpha;
	F32 m_finalAlpha;
	F32 m_crntAlpha;

	Vec3 m_crntPosition;

	Bool isDead() const
	{
		return m_timeOfDeath < 0.0;
	}

	/// Kill the particle
	void killCommon()
	{
		ANKI_ASSERT(m_timeOfDeath > 0.0);
		m_timeOfDeath = -1.0;
	}

	/// Revive the particle
	void reviveCommon(const ParticleEmitterProperties& props, Second crntTime)
	{
		ANKI_ASSERT(isDead());

		// life
		m_timeOfDeath = crntTime + getRandomRange(props.m_particle.m_minLife, props.m_particle.m_maxLife);
		m_timeOfBirth = crntTime;

		// Size
		m_initialSize = getRandomRange(props.m_particle.m_minInitialSize, props.m_particle.m_maxInitialSize);
		m_finalSize = getRandomRange(props.m_particle.m_minFinalSize, props.m_particle.m_maxFinalSize);

		// Alpha
		m_initialAlpha = getRandomRange(props.m_particle.m_minInitialAlpha, props.m_particle.m_maxInitialAlpha);
		m_finalAlpha = getRandomRange(props.m_particle.m_minFinalAlpha, props.m_particle.m_maxFinalAlpha);
	}

	/// Common sumulation code
	void simulateCommon(Second crntTime)
	{
		const F32 lifeFactor = F32((crntTime - m_timeOfBirth) / (m_timeOfDeath - m_timeOfBirth));

		m_crntSize = mix(m_initialSize, m_finalSize, lifeFactor);
		m_crntAlpha = mix(m_initialAlpha, m_finalAlpha, lifeFactor);
	}
};

/// Simple particle for simple simulation
class ParticleEmitterComponent::SimpleParticle : public ParticleEmitterComponent::ParticleBase
{
public:
	Vec3 m_velocity = Vec3(0.0f);
	Vec3 m_acceleration = Vec3(0.0f);

	void kill()
	{
		killCommon();
	}

	void revive(const ParticleEmitterProperties& props, const Transform& trf, Second crntTime)
	{
		reviveCommon(props, crntTime);
		m_velocity = Vec3(0.0f);

		m_acceleration = getRandom(props.m_particle.m_minGravity, props.m_particle.m_maxGravity);

		// Set the initial position
		m_crntPosition = getRandom(props.m_particle.m_minStartingPosition, props.m_particle.m_maxStartingPosition);
		m_crntPosition += trf.getOrigin().xyz();
	}

	void simulate(Second prevUpdateTime, Second crntTime)
	{
		simulateCommon(crntTime);

		const F32 dt = F32(crntTime - prevUpdateTime);

		const Vec3 xp = m_crntPosition;
		const Vec3 xc = m_acceleration * (dt * dt) + m_velocity * dt + xp;

		m_crntPosition = xc;

		m_velocity += m_acceleration * dt;
	}
};

/// Particle for bullet simulations
class ParticleEmitterComponent::PhysicsParticle : public ParticleEmitterComponent::ParticleBase
{
public:
	PhysicsBodyPtr m_body;

	PhysicsParticle(const PhysicsBodyInitInfo& init, SceneNode* node, ParticleEmitterComponent* component)
	{
		m_body = getExternalSubsystems(*node).m_physicsWorld->newInstance<PhysicsBody>(init);
		m_body->setUserData(component);
		m_body->activate(false);
		m_body->setMaterialGroup(PhysicsMaterialBit::kParticle);
		m_body->setMaterialMask(PhysicsMaterialBit::kStaticGeometry);
		m_body->setAngularFactor(Vec3(0.0f));
	}

	void kill()
	{
		killCommon();
		m_body->activate(false);
	}

	void revive(const ParticleEmitterProperties& props, const Transform& trf, Second crntTime)
	{
		reviveCommon(props, crntTime);

		// pre calculate
		const Bool forceFlag = props.forceEnabled();
		const Bool worldGravFlag = props.wordGravityEnabled();

		// Activate it
		m_body->activate(true);
		m_body->setLinearVelocity(Vec3(0.0f));
		m_body->setAngularVelocity(Vec3(0.0f));
		m_body->clearForces();

		// force
		if(forceFlag)
		{
			Vec3 forceDir = getRandom(props.m_particle.m_minForceDirection, props.m_particle.m_maxForceDirection);
			forceDir.normalize();

			// The forceDir depends on the particle emitter rotation
			forceDir = trf.getRotation().getRotationPart() * forceDir;

			const F32 forceMag =
				getRandomRange(props.m_particle.m_minForceMagnitude, props.m_particle.m_maxForceMagnitude);
			m_body->applyForce(forceDir * forceMag, Vec3(0.0f));
		}

		// gravity
		if(!worldGravFlag)
		{
			m_body->setGravity(getRandom(props.m_particle.m_minGravity, props.m_particle.m_maxGravity));
		}

		// Starting pos. In local space
		Vec3 pos = getRandom(props.m_particle.m_minStartingPosition, props.m_particle.m_maxStartingPosition);
		pos = trf.transform(pos);

		m_body->setTransform(Transform(pos.xyz0(), trf.getRotation(), 1.0f));
		m_crntPosition = pos;
	}

	void simulate([[maybe_unused]] Second prevUpdateTime, Second crntTime)
	{
		simulateCommon(crntTime);
		m_crntPosition = m_body->getTransform().getOrigin().xyz();
	}
};

ParticleEmitterComponent::ParticleEmitterComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_spatial(this)
{
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	m_simpleParticles.destroy(m_node->getMemoryPool());
	m_physicsParticles.destroy(m_node->getMemoryPool());

	GpuSceneMemoryPool& gpuScenePool = *getExternalSubsystems(*m_node).m_gpuSceneMemoryPool;
	gpuScenePool.deferredFree(m_gpuScenePositions);
	gpuScenePool.deferredFree(m_gpuSceneScales);
	gpuScenePool.deferredFree(m_gpuSceneAlphas);
	gpuScenePool.deferredFree(m_gpuSceneUniforms);

	if(m_gpuSceneParticleEmitterOffset != kMaxU32)
	{
		m_node->getSceneGraph().getAllGpuSceneContiguousArrays().deferredFree(
			GpuSceneContiguousArrayType::kParticleEmitters, m_gpuSceneParticleEmitterOffset);
	}

	m_spatial.removeFromOctree(m_node->getSceneGraph().getOctree());
}

void ParticleEmitterComponent::loadParticleEmitterResource(CString filename)
{
	// Load
	ParticleEmitterResourcePtr rsrc;
	const Error err = getExternalSubsystems(*m_node).m_resourceManager->loadResource(filename, rsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load particle emitter");
		return;
	}

	m_particleEmitterResource = std::move(rsrc);

	m_props = m_particleEmitterResource->getProperties();
	m_resourceUpdated = true;

	// Cleanup
	m_simpleParticles.destroy(m_node->getMemoryPool());
	m_physicsParticles.destroy(m_node->getMemoryPool());
	GpuSceneMemoryPool& gpuScenePool = *getExternalSubsystems(*m_node).m_gpuSceneMemoryPool;
	gpuScenePool.deferredFree(m_gpuScenePositions);
	gpuScenePool.deferredFree(m_gpuSceneScales);
	gpuScenePool.deferredFree(m_gpuSceneAlphas);
	gpuScenePool.deferredFree(m_gpuSceneUniforms);

	if(m_gpuSceneParticleEmitterOffset != kMaxU32)
	{
		m_node->getSceneGraph().getAllGpuSceneContiguousArrays().deferredFree(
			GpuSceneContiguousArrayType::kParticleEmitters, m_gpuSceneParticleEmitterOffset);
	}

	// Init particles
	m_simulationType = (m_props.m_usePhysicsEngine) ? SimulationType::kPhysicsEngine : SimulationType::kSimple;
	if(m_simulationType == SimulationType::kPhysicsEngine)
	{
		PhysicsCollisionShapePtr collisionShape =
			getExternalSubsystems(*m_node).m_physicsWorld->newInstance<PhysicsSphere>(
				m_props.m_particle.m_minInitialSize / 2.0f);

		PhysicsBodyInitInfo binit;
		binit.m_shape = std::move(collisionShape);

		m_physicsParticles.resizeStorage(m_node->getMemoryPool(), m_props.m_maxNumOfParticles);
		for(U32 i = 0; i < m_props.m_maxNumOfParticles; i++)
		{
			binit.m_mass = getRandomRange(m_props.m_particle.m_minMass, m_props.m_particle.m_maxMass);
			m_physicsParticles.emplaceBack(m_node->getMemoryPool(), binit, m_node, this);
		}
	}
	else
	{
		m_simpleParticles.create(m_node->getMemoryPool(), m_props.m_maxNumOfParticles);
	}

	// GPU scene allocations
	gpuScenePool.allocate(sizeof(Vec3) * m_props.m_maxNumOfParticles, alignof(F32), m_gpuScenePositions);
	gpuScenePool.allocate(sizeof(F32) * m_props.m_maxNumOfParticles, alignof(F32), m_gpuSceneAlphas);
	gpuScenePool.allocate(sizeof(F32) * m_props.m_maxNumOfParticles, alignof(F32), m_gpuSceneScales);
	gpuScenePool.allocate(m_particleEmitterResource->getMaterial()->getPrefilledLocalUniforms().getSizeInBytes(),
						  alignof(U32), m_gpuSceneUniforms);

	m_gpuSceneParticleEmitterOffset = U32(m_node->getSceneGraph().getAllGpuSceneContiguousArrays().allocate(
		GpuSceneContiguousArrayType::kParticleEmitters));
}

Error ParticleEmitterComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!m_particleEmitterResource.isCreated()) [[unlikely]]
	{
		updated = false;
		return Error::kNone;
	}

	updated = true;
	Vec3* positions;
	F32* scales;
	F32* alphas;

	Aabb aabbWorld;
	if(m_simulationType == SimulationType::kSimple)
	{
		simulate(info.m_previousTime, info.m_currentTime, WeakArray<SimpleParticle>(m_simpleParticles), positions,
				 scales, alphas, aabbWorld);
	}
	else
	{
		ANKI_ASSERT(m_simulationType == SimulationType::kPhysicsEngine);
		simulate(info.m_previousTime, info.m_currentTime, WeakArray<PhysicsParticle>(m_physicsParticles), positions,
				 scales, alphas, aabbWorld);
	}

	m_spatial.setBoundingShape(aabbWorld);
	m_spatial.update(info.m_node->getSceneGraph().getOctree());

	// Upload to the GPU scene
	GpuSceneMicroPatcher& patcher = *info.m_gpuSceneMicroPatcher;
	if(m_aliveParticleCount > 0)
	{
		patcher.newCopy(*info.m_framePool, m_gpuScenePositions.m_offset, sizeof(Vec3) * m_aliveParticleCount,
						positions);
		patcher.newCopy(*info.m_framePool, m_gpuSceneScales.m_offset, sizeof(F32) * m_aliveParticleCount, scales);
		patcher.newCopy(*info.m_framePool, m_gpuSceneAlphas.m_offset, sizeof(F32) * m_aliveParticleCount, alphas);
	}

	if(m_resourceUpdated)
	{
		GpuSceneParticleEmitter particles = {};
		particles.m_vertexOffsets[U32(VertexStreamId::kParticlePosition)] = U32(m_gpuScenePositions.m_offset);
		particles.m_vertexOffsets[U32(VertexStreamId::kParticleColor)] = U32(m_gpuSceneAlphas.m_offset);
		particles.m_vertexOffsets[U32(VertexStreamId::kParticleScale)] = U32(m_gpuSceneScales.m_offset);

		patcher.newCopy(*info.m_framePool, m_gpuSceneParticleEmitterOffset, sizeof(GpuSceneParticleEmitter),
						&particles);

		patcher.newCopy(*info.m_framePool, m_gpuSceneUniforms.m_offset,
						m_particleEmitterResource->getMaterial()->getPrefilledLocalUniforms().getSizeInBytes(),
						m_particleEmitterResource->getMaterial()->getPrefilledLocalUniforms().getBegin());
	}

	m_resourceUpdated = false;
	return Error::kNone;
}

template<typename TParticle>
void ParticleEmitterComponent::simulate(Second prevUpdateTime, Second crntTime, WeakArray<TParticle> particles,
										Vec3*& positions, F32*& scales, F32*& alphas, Aabb& aabbWorld)
{
	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff

	Vec3 aabbMin(kMaxF32);
	Vec3 aabbMax(kMinF32);
	m_aliveParticleCount = 0;

	positions = static_cast<Vec3*>(
		m_node->getFrameMemoryPool().allocate(m_props.m_maxNumOfParticles * sizeof(Vec3), alignof(Vec3)));
	scales = static_cast<F32*>(
		m_node->getFrameMemoryPool().allocate(m_props.m_maxNumOfParticles * sizeof(F32), alignof(F32)));
	alphas = static_cast<F32*>(
		m_node->getFrameMemoryPool().allocate(m_props.m_maxNumOfParticles * sizeof(F32), alignof(F32)));

	F32 maxParticleSize = -1.0f;

	for(TParticle& particle : particles)
	{
		if(particle.isDead())
		{
			// if its already dead so dont deactivate it again
			continue;
		}

		if(particle.m_timeOfDeath < crntTime)
		{
			// Just died
			particle.kill();
		}
		else
		{
			// It's alive

			// This will calculate a new world transformation
			particle.simulate(prevUpdateTime, crntTime);

			const Vec3& origin = particle.m_crntPosition;

			aabbMin = aabbMin.min(origin);
			aabbMax = aabbMax.max(origin);

			positions[m_aliveParticleCount] = origin;

			scales[m_aliveParticleCount] = particle.m_crntSize;
			maxParticleSize = max(maxParticleSize, particle.m_crntSize);

			alphas[m_aliveParticleCount] = clamp(particle.m_crntAlpha, 0.0f, 1.0f);

			++m_aliveParticleCount;
		}
	}

	// AABB
	if(m_aliveParticleCount != 0)
	{
		ANKI_ASSERT(maxParticleSize > 0.0f);
		const Vec3 min = aabbMin - maxParticleSize;
		const Vec3 max = aabbMax + maxParticleSize;
		aabbWorld = Aabb(min, max);
	}
	else
	{
		aabbWorld = Aabb(Vec3(0.0f), Vec3(0.001f));
		positions = nullptr;
		alphas = scales = nullptr;
	}

	//
	// Emit new particles
	//
	if(m_timeLeftForNextEmission <= 0.0)
	{
		U particleCount = 0; // How many particles I am allowed to emmit
		for(TParticle& particle : particles)
		{
			if(!particle.isDead())
			{
				// its alive so skip it
				continue;
			}

			particle.revive(m_props, m_node->getWorldTransform(), crntTime);

			// do the rest
			++particleCount;
			if(particleCount >= m_props.m_particlesPerEmission)
			{
				break;
			}
		} // end for all particles

		m_timeLeftForNextEmission = m_props.m_emissionPeriod;
	} // end if can emit
	else
	{
		m_timeLeftForNextEmission -= crntTime - prevUpdateTime;
	}
}

void ParticleEmitterComponent::setupRenderableQueueElements(RenderingTechnique technique, StackMemoryPool& tmpPool,
															WeakArray<RenderableQueueElement>& outRenderables) const
{
	if(!(m_particleEmitterResource->getMaterial()->getRenderingTechniques() & RenderingTechniqueBit(1 << technique))
	   || m_aliveParticleCount == 0)
	{
		outRenderables.setArray(nullptr, 0);
		return;
	}

	RenderingKey key;
	key.setRenderingTechnique(technique);
	ShaderProgramPtr prog;
	m_particleEmitterResource->getRenderingInfo(key, prog);

	RenderableQueueElement* el = static_cast<RenderableQueueElement*>(
		tmpPool.allocate(sizeof(RenderableQueueElement), alignof(RenderableQueueElement)));

	el->m_mergeKey = 0; // Not mergable
	el->m_program = prog.get();
	el->m_worldTransformsOffset = 0;
	el->m_uniformsOffset = U32(m_gpuSceneUniforms.m_offset);
	el->m_geometryOffset = m_gpuSceneParticleEmitterOffset;
	el->m_boneTransformsOffset = 0;
	el->m_vertexCount = 6 * m_aliveParticleCount;
	el->m_firstVertex = 0;
	el->m_indexed = false;
	el->m_primitiveTopology = PrimitiveTopology::kTriangles;
	el->m_aabbMin = m_spatial.getAabbWorldSpace().getMin().xyz();
	el->m_aabbMax = m_spatial.getAabbWorldSpace().getMax().xyz();

	outRenderables.setArray(el, 1);
}

} // end namespace anki
