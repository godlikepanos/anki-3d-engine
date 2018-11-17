// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ParticleEmitterNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/resource/ModelResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/util/Functions.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/Gr.h>

namespace anki
{

static F32 getRandom(F32 min, F32 max)
{
	F32 factor = randFloat(1.0f);
	return mix(min, max, factor);
}

static Vec3 getRandom(const Vec3& min, const Vec3& max)
{
	Vec3 out;
	out.x() = mix(min.x(), max.x(), randFloat(1.0f));
	out.y() = mix(min.y(), max.y(), randFloat(1.0f));
	out.z() = mix(min.z(), max.z(), randFloat(1.0f));
	return out;
}

/// Particle base
class ParticleEmitterNode::ParticleBase
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

	Vec4 m_crntPosition;

	virtual ~ParticleBase()
	{
	}

	Bool isDead() const
	{
		return m_timeOfDeath < 0.0;
	}

	/// Kill the particle
	virtual void kill()
	{
		ANKI_ASSERT(m_timeOfDeath > 0.0);
		m_timeOfDeath = -1.0;
	}

	/// Revive the particle
	virtual void revive(
		const ParticleEmitterProperties& props, const Transform& trf, Second prevUpdateTime, Second crntTime)
	{
		ANKI_ASSERT(isDead());

		// life
		m_timeOfDeath = crntTime + getRandom(props.m_particle.m_minLife, props.m_particle.m_maxLife);
		m_timeOfBirth = crntTime;

		// Size
		m_initialSize = getRandom(props.m_particle.m_minInitialSize, props.m_particle.m_maxInitialSize);
		m_finalSize = getRandom(props.m_particle.m_minFinalSize, props.m_particle.m_maxFinalSize);

		// Alpha
		m_initialAlpha = getRandom(props.m_particle.m_minInitialAlpha, props.m_particle.m_maxInitialAlpha);
		m_finalAlpha = getRandom(props.m_particle.m_minFinalAlpha, props.m_particle.m_maxFinalAlpha);
	}

	/// Common sumulation code
	virtual void simulate(Second prevUpdateTime, Second crntTime)
	{
		const F32 lifeFactor = (crntTime - m_timeOfBirth) / (m_timeOfDeath - m_timeOfBirth);

		m_crntSize = mix(m_initialSize, m_finalSize, lifeFactor);
		m_crntAlpha = mix(m_initialAlpha, m_finalAlpha, lifeFactor);
	}
};

/// Simple particle for simple simulation
class ParticleEmitterNode::ParticleSimple : public ParticleEmitterNode::ParticleBase
{
public:
	Vec4 m_velocity = Vec4(0.0);
	Vec4 m_acceleration = Vec4(0.0);

	void revive(
		const ParticleEmitterProperties& props, const Transform& trf, Second prevUpdateTime, Second crntTime) override
	{
		ParticleBase::revive(props, trf, prevUpdateTime, crntTime);
		m_velocity = Vec4(0.0);

		m_acceleration = getRandom(props.m_particle.m_minGravity, props.m_particle.m_maxGravity).xyz0();

		// Set the initial position
		m_crntPosition =
			getRandom(props.m_particle.m_minStartingPosition, props.m_particle.m_maxStartingPosition).xyz0();

		m_crntPosition += trf.getOrigin();
	}

	void simulate(Second prevUpdateTime, Second crntTime) override
	{
		ParticleBase::simulate(prevUpdateTime, crntTime);

		Second dt = crntTime - prevUpdateTime;

		Vec4 xp = m_crntPosition;
		Vec4 xc = m_acceleration * (dt * dt) + m_velocity * dt + xp;

		m_crntPosition = xc;

		m_velocity += m_acceleration * dt;
	}
};

/// Particle for bullet simulations
class ParticleEmitterNode::PhysParticle : public ParticleEmitterNode::ParticleBase
{
public:
	PhysicsBodyPtr m_body;

	PhysParticle(const PhysicsBodyInitInfo& init, SceneNode* node)
	{
		m_body = node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
		m_body->setUserData(node);
		m_body->activate(false);
		m_body->setMaterialGroup(PhysicsMaterialBit::PARTICLE);
		m_body->setMaterialMask(PhysicsMaterialBit::STATIC_GEOMETRY);
		m_body->setAngularFactor(Vec3(0.0f, 0.0f, 0.0f));
	}

	void kill() override
	{
		ParticleBase::kill();
		m_body->activate(false);
	}

	void revive(
		const ParticleEmitterProperties& props, const Transform& trf, Second prevUpdateTime, Second crntTime) override
	{
		ParticleBase::revive(props, trf, prevUpdateTime, crntTime);

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

			// the forceDir depends on the particle emitter rotation
			forceDir = trf.getRotation().getRotationPart() * forceDir;

			const F32 forceMag = getRandom(props.m_particle.m_minForceMagnitude, props.m_particle.m_maxForceMagnitude);
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
		m_crntPosition = pos.xyz0();
	}

	void simulate(Second prevUpdateTime, Second crntTime) override
	{
		ParticleBase::simulate(prevUpdateTime, crntTime);
		m_crntPosition = m_body->getTransform().getOrigin();
	}
};

/// The derived render component for particle emitters.
class ParticleEmitterNode::MyRenderComponent : public MaterialRenderComponent
{
public:
	MyRenderComponent(SceneNode* node)
		: MaterialRenderComponent(
			  node, static_cast<ParticleEmitterNode*>(node)->m_particleEmitterResource->getMaterial())
	{
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const override
	{
		static_cast<const ParticleEmitterNode&>(getSceneNode()).setupRenderableQueueElement(el);
	}
};

/// Feedback component
class ParticleEmitterNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false; // Don't care about updates for this component

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<ParticleEmitterNode&>(node).onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

ParticleEmitterNode::ParticleEmitterNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

ParticleEmitterNode::~ParticleEmitterNode()
{
	// Delete simple particles
	for(ParticleBase* part : m_particles)
	{
		getAllocator().deleteInstance(part);
	}

	m_particles.destroy(getAllocator());
}

Error ParticleEmitterNode::init(const CString& filename)
{
	// Load resource
	ANKI_CHECK(getResourceManager().loadResource(filename, m_particleEmitterResource));

	// Move component
	newComponent<MoveComponent>();

	// Move component feedback
	newComponent<MoveFeedbackComponent>();

	// Spatial component
	newComponent<SpatialComponent>(this, &m_obb);

	// Render component
	newComponent<MyRenderComponent>(this);

	// Other
	m_obb.setCenter(Vec4(0.0));
	m_obb.setExtend(Vec4(1.0, 1.0, 1.0, 0.0));
	m_obb.setRotation(Mat3x4::getIdentity());

	// copy the resource to me
	ParticleEmitterProperties& me = *this;
	const ParticleEmitterProperties& other = m_particleEmitterResource->getProperties();
	me = other;

	if(m_usePhysicsEngine)
	{
		createParticlesPhysicsSimulation(&getSceneGraph());
		m_simulationType = SimulationType::PHYSICS_ENGINE;
	}
	else
	{
		createParticlesSimpleSimulation();
		m_simulationType = SimulationType::SIMPLE;
	}

	// Create the vertex buffer and object
	m_vertBuffSize = m_maxNumOfParticles * VERTEX_SIZE;

	return Error::NONE;
}

void ParticleEmitterNode::drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	ANKI_ASSERT(userData.getSize() == 1);

	const ParticleEmitterNode& self = *static_cast<const ParticleEmitterNode*>(userData[0]);

	// Early exit
	if(ANKI_UNLIKELY(self.m_aliveParticlesCount == 0))
	{
		return;
	}

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(!ctx.m_debugDraw)
	{
		// Load verts
		StagingGpuMemoryToken token;
		void* gpuStorage = ctx.m_stagingGpuAllocator->allocateFrame(
			self.m_aliveParticlesCount * VERTEX_SIZE, StagingGpuMemoryType::VERTEX, token);
		memcpy(gpuStorage, self.m_verts, self.m_aliveParticlesCount * VERTEX_SIZE);

		// Program
		ShaderProgramPtr prog;
		self.m_particleEmitterResource->getRenderingInfo(ctx.m_key.m_lod, prog);
		cmdb->bindShaderProgram(prog);

		// Vertex attribs
		cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
		cmdb->setVertexAttribute(1, 0, Format::R32_SFLOAT, sizeof(Vec3));
		cmdb->setVertexAttribute(2, 0, Format::R32_SFLOAT, sizeof(Vec3) + sizeof(F32));

		// Vertex buff
		cmdb->bindVertexBuffer(0, token.m_buffer, token.m_offset, VERTEX_SIZE, VertexStepRate::INSTANCE);

		// Uniforms
		Array<Mat4, 1> trf = {{Mat4::getIdentity()}};
		static_cast<const MaterialRenderComponent&>(self.getComponent<RenderComponent>())
			.allocateAndSetupUniforms(self.m_particleEmitterResource->getMaterial()->getDescriptorSetIndex(),
				ctx,
				trf,
				trf,
				*ctx.m_stagingGpuAllocator);

		// Draw
		cmdb->drawArrays(PrimitiveTopology::TRIANGLE_STRIP, 4, self.m_aliveParticlesCount, 0, 0);
	}
	else
	{
		// TODO
	}
}

void ParticleEmitterNode::onMoveComponentUpdate(MoveComponent& move)
{
	m_identityRotation = move.getWorldTransform().getRotation() == Mat3x4::getIdentity();

	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	sp.markForUpdate();
}

void ParticleEmitterNode::createParticlesPhysicsSimulation(SceneGraph* scene)
{
	PhysicsCollisionShapePtr collisionShape =
		getSceneGraph().getPhysicsWorld().newInstance<PhysicsSphere>(m_particle.m_minInitialSize / 2.0f);

	PhysicsBodyInitInfo binit;
	binit.m_shape = collisionShape;

	m_particles.create(getAllocator(), m_maxNumOfParticles);

	for(U i = 0; i < m_maxNumOfParticles; i++)
	{
		binit.m_mass = getRandom(m_particle.m_minMass, m_particle.m_maxMass);

		PhysParticle* part = getAllocator().newInstance<PhysParticle>(binit, this);

		m_particles[i] = part;
	}
}

void ParticleEmitterNode::createParticlesSimpleSimulation()
{
	m_particles.create(getAllocator(), m_maxNumOfParticles);

	for(U i = 0; i < m_maxNumOfParticles; i++)
	{
		ParticleSimple* part = getAllocator().newInstance<ParticleSimple>();

		m_particles[i] = part;
	}
}

Error ParticleEmitterNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff
	//
	Vec4 aabbmin(MAX_F32, MAX_F32, MAX_F32, 0.0f);
	Vec4 aabbmax(MIN_F32, MIN_F32, MIN_F32, 0.0f);
	m_aliveParticlesCount = 0;

	F32* verts = reinterpret_cast<F32*>(getFrameAllocator().allocate(m_vertBuffSize));
	m_verts = verts;

	const F32* verts_base = verts;
	(void)verts_base;

	F32 maxParticleSize = -1.0f;

	for(ParticleBase* p : m_particles)
	{
		if(p->isDead())
		{
			// if its already dead so dont deactivate it again
			continue;
		}

		if(p->m_timeOfDeath < crntTime)
		{
			// Just died
			p->kill();
		}
		else
		{
			// It's alive

			// Do checks
			ANKI_ASSERT((PtrSize(verts) + VERTEX_SIZE - PtrSize(verts_base)) <= m_vertBuffSize);

			// This will calculate a new world transformation
			p->simulate(prevUpdateTime, crntTime);

			const Vec4& origin = p->m_crntPosition;

			aabbmin = aabbmin.min(origin);
			aabbmax = aabbmax.max(origin);

			verts[0] = origin.x();
			verts[1] = origin.y();
			verts[2] = origin.z();

			verts[3] = p->m_crntSize;
			maxParticleSize = max(maxParticleSize, p->m_crntSize);

			verts[4] = clamp(p->m_crntAlpha, 0.0f, 1.0f);

			++m_aliveParticlesCount;
			verts += 5;
		}
	}

	if(m_aliveParticlesCount != 0)
	{
		ANKI_ASSERT(maxParticleSize > 0.0f);
		Vec4 min = aabbmin - maxParticleSize;
		Vec4 max = aabbmax + maxParticleSize;
		Vec4 center = (min + max) / 2.0;

		m_obb = Obb(center.xyz0(), Mat3x4::getIdentity(), (max - center).xyz0());
	}
	else
	{
		m_obb = Obb(Vec4(0.0), Mat3x4::getIdentity(), Vec4(Vec3(0.001f), 0.0f));
		m_verts = nullptr;
	}

	getComponent<SpatialComponent>().markForUpdate();

	//
	// Emit new particles
	//
	if(m_timeLeftForNextEmission <= 0.0)
	{
		MoveComponent& move = getComponent<MoveComponent>();

		U particlesCount = 0; // How many particles I am allowed to emmit
		for(ParticleBase* pp : m_particles)
		{
			ParticleBase& p = *pp;
			if(!p.isDead())
			{
				// its alive so skip it
				continue;
			}

			p.revive(*this, move.getWorldTransform(), prevUpdateTime, crntTime);

			// do the rest
			++particlesCount;
			if(particlesCount >= m_particlesPerEmission)
			{
				break;
			}
		} // end for all particles

		m_timeLeftForNextEmission = m_emissionPeriod;
	} // end if can emit
	else
	{
		m_timeLeftForNextEmission -= crntTime - prevUpdateTime;
	}

	return Error::NONE;
}

} // end namespace anki
