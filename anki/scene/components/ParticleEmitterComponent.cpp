// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/ParticleEmitterComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/resource/ParticleEmitterResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/Math.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(ParticleEmitterComponent)

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
	void reviveCommon(const ParticleEmitterProperties& props, const Transform& trf, Second prevUpdateTime,
					  Second crntTime)
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
	void simulateCommon(Second prevUpdateTime, Second crntTime)
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

	void revive(const ParticleEmitterProperties& props, const Transform& trf, Second prevUpdateTime, Second crntTime)
	{
		reviveCommon(props, trf, prevUpdateTime, crntTime);
		m_velocity = Vec3(0.0f);

		m_acceleration = getRandom(props.m_particle.m_minGravity, props.m_particle.m_maxGravity);

		// Set the initial position
		m_crntPosition = getRandom(props.m_particle.m_minStartingPosition, props.m_particle.m_maxStartingPosition);
		m_crntPosition += trf.getOrigin().xyz();
	}

	void simulate(Second prevUpdateTime, Second crntTime)
	{
		simulateCommon(prevUpdateTime, crntTime);

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
		m_body = node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
		m_body->setUserData(component);
		m_body->activate(false);
		m_body->setMaterialGroup(PhysicsMaterialBit::PARTICLE);
		m_body->setMaterialMask(PhysicsMaterialBit::STATIC_GEOMETRY);
		m_body->setAngularFactor(Vec3(0.0f));
	}

	void kill()
	{
		killCommon();
		m_body->activate(false);
	}

	void revive(const ParticleEmitterProperties& props, const Transform& trf, Second prevUpdateTime, Second crntTime)
	{
		reviveCommon(props, trf, prevUpdateTime, crntTime);

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

	void simulate(Second prevUpdateTime, Second crntTime)
	{
		simulateCommon(prevUpdateTime, crntTime);
		m_crntPosition = m_body->getTransform().getOrigin().xyz();
	}
};

ParticleEmitterComponent::ParticleEmitterComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	m_simpleParticles.destroy(m_node->getAllocator());
	m_physicsParticles.destroy(m_node->getAllocator());
}

Error ParticleEmitterComponent::loadParticleEmitterResource(CString filename)
{
	// Cleanup
	m_simpleParticles.destroy(m_node->getAllocator());
	m_physicsParticles.destroy(m_node->getAllocator());

	// Load
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(filename, m_particleEmitterResource));
	m_props = m_particleEmitterResource->getProperties();

	// Init particles
	m_simulationType = (m_props.m_usePhysicsEngine) ? SimulationType::PHYSICS_ENGINE : SimulationType::SIMPLE;
	if(m_simulationType == SimulationType::PHYSICS_ENGINE)
	{
		PhysicsCollisionShapePtr collisionShape = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsSphere>(
			m_props.m_particle.m_minInitialSize / 2.0f);

		PhysicsBodyInitInfo binit;
		binit.m_shape = collisionShape;

		m_physicsParticles.resizeStorage(m_node->getAllocator(), m_props.m_maxNumOfParticles);
		for(U32 i = 0; i < m_props.m_maxNumOfParticles; i++)
		{
			binit.m_mass = getRandomRange(m_props.m_particle.m_minMass, m_props.m_particle.m_maxMass);
			m_physicsParticles.emplaceBack(m_node->getAllocator(), binit, m_node, this);
		}
	}
	else
	{
		m_simpleParticles.create(m_node->getAllocator(), m_props.m_maxNumOfParticles);
	}

	m_vertBuffSize = m_props.m_maxNumOfParticles * VERTEX_SIZE;

	return Error::NONE;
}

Error ParticleEmitterComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	if(ANKI_UNLIKELY(!m_particleEmitterResource.isCreated()))
	{
		updated = false;
		return Error::NONE;
	}

	updated = true;

	if(m_simulationType == SimulationType::SIMPLE)
	{
		simulate(prevTime, crntTime, WeakArray<SimpleParticle>(m_simpleParticles));
	}
	else
	{
		ANKI_ASSERT(m_simulationType == SimulationType::PHYSICS_ENGINE);
		simulate(prevTime, crntTime, WeakArray<PhysicsParticle>(m_physicsParticles));
	}

	return Error::NONE;
}

template<typename TParticle>
void ParticleEmitterComponent::simulate(Second prevUpdateTime, Second crntTime, WeakArray<TParticle> particles)
{
	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff

	Vec3 aabbMin(MAX_F32);
	Vec3 aabbMax(MIN_F32);
	m_aliveParticleCount = 0;

	F32* verts = reinterpret_cast<F32*>(m_node->getFrameAllocator().allocate(m_vertBuffSize));
	m_verts = verts;

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

			// Do checks
			ANKI_ASSERT((ptrToNumber(verts) + VERTEX_SIZE - ptrToNumber(m_verts)) <= m_vertBuffSize);

			// This will calculate a new world transformation
			particle.simulate(prevUpdateTime, crntTime);

			const Vec3& origin = particle.m_crntPosition;

			aabbMin = aabbMin.min(origin);
			aabbMax = aabbMax.max(origin);

			verts[0] = origin.x();
			verts[1] = origin.y();
			verts[2] = origin.z();

			verts[3] = particle.m_crntSize;
			maxParticleSize = max(maxParticleSize, particle.m_crntSize);

			verts[4] = clamp(particle.m_crntAlpha, 0.0f, 1.0f);

			++m_aliveParticleCount;
			verts += 5;
		}
	}

	// AABB
	if(m_aliveParticleCount != 0)
	{
		ANKI_ASSERT(maxParticleSize > 0.0f);
		const Vec3 min = aabbMin - maxParticleSize;
		const Vec3 max = aabbMax + maxParticleSize;
		m_worldBoundingVolume = Aabb(min, max);
	}
	else
	{
		m_worldBoundingVolume = Aabb(Vec3(0.0f), Vec3(0.001f));
		m_verts = nullptr;
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

			particle.revive(m_props, m_transform, prevUpdateTime, crntTime);

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

void ParticleEmitterComponent::drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	ANKI_ASSERT(userData.getSize() == 1);

	const ParticleEmitterComponent& self = *static_cast<const ParticleEmitterComponent*>(userData[0]);

	// Early exit
	if(ANKI_UNLIKELY(self.m_aliveParticleCount == 0))
	{
		return;
	}

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(!ctx.m_debugDraw)
	{
		// Load verts
		StagingGpuMemoryToken token;
		void* gpuStorage = ctx.m_stagingGpuAllocator->allocateFrame(self.m_aliveParticleCount * VERTEX_SIZE,
																	StagingGpuMemoryType::VERTEX, token);
		memcpy(gpuStorage, self.m_verts, self.m_aliveParticleCount * VERTEX_SIZE);

		// Program
		ShaderProgramPtr prog;
		self.m_particleEmitterResource->getRenderingInfo(ctx.m_key, prog);
		cmdb->bindShaderProgram(prog);

		// Vertex attribs
		cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
		cmdb->setVertexAttribute(1, 0, Format::R32_SFLOAT, sizeof(Vec3));
		cmdb->setVertexAttribute(2, 0, Format::R32_SFLOAT, sizeof(Vec3) + sizeof(F32));

		// Vertex buff
		cmdb->bindVertexBuffer(0, token.m_buffer, token.m_offset, VERTEX_SIZE, VertexStepRate::INSTANCE);

		// Uniforms
		Array<Mat4, 1> trf = {Mat4::getIdentity()};
		RenderComponent::allocateAndSetupUniforms(self.m_particleEmitterResource->getMaterial(), ctx, trf, trf,
												  *ctx.m_stagingGpuAllocator);

		// Draw
		cmdb->drawArrays(PrimitiveTopology::TRIANGLE_STRIP, 4, self.m_aliveParticleCount, 0, 0);
	}
	else
	{
		// TODO
	}
}

} // end namespace anki
