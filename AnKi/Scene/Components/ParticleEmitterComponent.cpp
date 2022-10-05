// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ParticleEmitterComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Components/RenderComponent.h>
#include <AnKi/Resource/ParticleEmitterResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Math.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

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
		m_body = node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
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
{
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	m_simpleParticles.destroy(m_node->getMemoryPool());
	m_physicsParticles.destroy(m_node->getMemoryPool());
}

Error ParticleEmitterComponent::loadParticleEmitterResource(CString filename)
{
	// Create the debug drawer
	if(!m_dbgImage.isCreated())
	{
		ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource("EngineAssets/ParticleEmitter.ankitex",
																			 m_dbgImage));
	}

	// Load
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(filename, m_particleEmitterResource));
	m_props = m_particleEmitterResource->getProperties();

	// Cleanup
	m_simpleParticles.destroy(m_node->getMemoryPool());
	m_physicsParticles.destroy(m_node->getMemoryPool());

	// Init particles
	m_simulationType = (m_props.m_usePhysicsEngine) ? SimulationType::kPhysicsEngine : SimulationType::kSimple;
	if(m_simulationType == SimulationType::kPhysicsEngine)
	{
		PhysicsCollisionShapePtr collisionShape = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsSphere>(
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

	m_vertBuffSize = m_props.m_maxNumOfParticles * kVertexSize;

	return Error::kNone;
}

Error ParticleEmitterComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(ANKI_UNLIKELY(!m_particleEmitterResource.isCreated()))
	{
		updated = false;
		return Error::kNone;
	}

	updated = true;

	if(m_simulationType == SimulationType::kSimple)
	{
		simulate(info.m_previousTime, info.m_currentTime, WeakArray<SimpleParticle>(m_simpleParticles));
	}
	else
	{
		ANKI_ASSERT(m_simulationType == SimulationType::kPhysicsEngine);
		simulate(info.m_previousTime, info.m_currentTime, WeakArray<PhysicsParticle>(m_physicsParticles));
	}

	return Error::kNone;
}

template<typename TParticle>
void ParticleEmitterComponent::simulate(Second prevUpdateTime, Second crntTime, WeakArray<TParticle> particles)
{
	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff

	Vec3 aabbMin(kMaxF32);
	Vec3 aabbMax(kMinF32);
	m_aliveParticleCount = 0;

	F32* verts = static_cast<F32*>(m_node->getFrameMemoryPool().allocate(m_vertBuffSize, alignof(F32)));
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
			ANKI_ASSERT((ptrToNumber(verts) + kVertexSize - ptrToNumber(m_verts)) <= m_vertBuffSize);

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

			particle.revive(m_props, m_transform, crntTime);

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

void ParticleEmitterComponent::draw(RenderQueueDrawContext& ctx) const
{
	// Early exit
	if(ANKI_UNLIKELY(m_aliveParticleCount == 0))
	{
		return;
	}

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(!ctx.m_debugDraw)
	{
		// Load verts
		StagingGpuMemoryToken token;
		void* gpuStorage = ctx.m_stagingGpuAllocator->allocateFrame(m_aliveParticleCount * kVertexSize,
																	StagingGpuMemoryType::kVertex, token);
		memcpy(gpuStorage, m_verts, m_aliveParticleCount * kVertexSize);

		// Program
		ShaderProgramPtr prog;
		m_particleEmitterResource->getRenderingInfo(ctx.m_key, prog);
		cmdb->bindShaderProgram(prog);

		// Vertex attribs
		cmdb->setVertexAttribute(U32(VertexAttributeId::kPosition), 0, Format::kR32G32B32_Sfloat, 0);
		cmdb->setVertexAttribute(U32(VertexAttributeId::SCALE), 0, Format::kR32_Sfloat, sizeof(Vec3));
		cmdb->setVertexAttribute(U32(VertexAttributeId::ALPHA), 0, Format::kR32_Sfloat, sizeof(Vec3) + sizeof(F32));

		// Vertex buff
		cmdb->bindVertexBuffer(0, token.m_buffer, token.m_offset, kVertexSize, VertexStepRate::kInstance);

		// Uniforms
		Array<Mat3x4, 1> trf = {Mat3x4::getIdentity()};
		RenderComponent::allocateAndSetupUniforms(m_particleEmitterResource->getMaterial(), ctx, trf, trf,
												  *ctx.m_stagingGpuAllocator);

		// Draw
		cmdb->drawArrays(PrimitiveTopology::kTriangleStrip, 4, m_aliveParticleCount, 0, 0);
	}
	else
	{
		const Vec4 tsl = (m_worldBoundingVolume.getMin() + m_worldBoundingVolume.getMax()) / 2.0f;
		const Vec4 scale = (m_worldBoundingVolume.getMax() - m_worldBoundingVolume.getMin()) / 2.0f;

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

		const Vec3 pos = m_transform.getOrigin().xyz();
		m_node->getSceneGraph().getDebugDrawer().drawBillboardTextures(
			ctx.m_projectionMatrix, ctx.m_viewMatrix, ConstWeakArray<Vec3>(&pos, 1), Vec4(1.0f),
			ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), m_dbgImage->getTextureView(),
			ctx.m_sampler, Vec2(0.75f), *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

		// Restore state
		if(!enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::kLess);
		}
	}
}

} // end namespace anki
