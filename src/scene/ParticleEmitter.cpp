// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/ParticleEmitter.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/InstanceNode.h"
#include "anki/scene/Misc.h"
#include "anki/resource/Model.h"
#include "anki/util/Functions.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/Gr.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

const U COMPONENTS = 3 + 1 + 1; // 3 position, 1 size, 1 alpha
const PtrSize VERT_SIZE = COMPONENTS * sizeof(F32);

//==============================================================================
static F32 getRandom(F32 initial, F32 deviation)
{
	return (deviation == 0.0) 
		? initial
		: initial + randFloat(deviation) * 2.0 - deviation;
}

//==============================================================================
static Vec3 getRandom(const Vec3& initial, const Vec3& deviation)
{
	if(deviation == Vec3(0.0))
	{
		return initial;
	}
	else
	{
		Vec3 out;
		for(U i = 0; i < 3; i++)
		{
			out[i] = getRandom(initial[i], deviation[i]);
		}
		return out;
	}
}

//==============================================================================
// ParticleBase                                                                =
//==============================================================================

//==============================================================================
void ParticleBase::revive(const ParticleEmitter& pe, const Transform& trf,
	F32 /*prevUpdateTime*/, F32 crntTime)
{
	ANKI_ASSERT(isDead());
	const ParticleEmitterProperties& props = pe;

	// life
	m_timeOfDeath = getRandom(crntTime + props.m_particle.m_life,
		props.m_particle.m_lifeDeviation);
	m_timeOfBirth = crntTime;
}

//==============================================================================
// ParticleSimple                                                              =
//==============================================================================

//==============================================================================
void ParticleSimple::simulate(const ParticleEmitter& pe,
	F32 prevUpdateTime, F32 crntTime)
{
	F32 dt = crntTime - prevUpdateTime;

	ANKI_ASSERT(
		static_cast<const ParticleEmitterProperties&>(pe).
		m_particle.m_gravity.getLength() > 0.0);

	Vec4 xp = m_position;
	Vec4 xc = m_acceleration * (dt * dt) + m_velocity * dt + xp;

	m_position = xc;

	m_velocity += m_acceleration * dt;
}

//==============================================================================
void ParticleSimple::revive(const ParticleEmitter& pe, const Transform& trf,
	F32 prevUpdateTime, F32 crntTime)
{
	ParticleBase::revive(pe, trf, prevUpdateTime, crntTime);
	m_velocity = Vec4(0.0);

	const ParticleEmitterProperties& props = pe;

	m_acceleration = getRandom(props.m_particle.m_gravity,
			props.m_particle.m_gravityDeviation).xyz0();

	// Set the initial position
	m_position = getRandom(props.m_particle.m_startingPos,
		props.m_particle.m_startingPosDeviation).xyz0();

	m_position += trf.getOrigin();
}

//==============================================================================
// Particle                                                                    =
//==============================================================================

#if 0

//==============================================================================
Particle::Particle(
	const char* name, SceneGraph* scene, // SceneNode
	// RigidBody
	PhysicsWorld* masterContainer, const RigidBody::Initializer& init_)
	:	ParticleBase(name, scene, PT_PHYSICS)
{
	RigidBody::Initializer init = init_;

	getSceneGraph().getPhysics().newPhysicsObject<RigidBody>(body, init);

	sceneNodeProtected.rigidBodyC = body;
}

//==============================================================================
Particle::~Particle()
{
	getSceneGraph().getPhysics().deletePhysicsObject(body);
}

//==============================================================================
void Particle::revive(const ParticleEmitter& pe,
	F32 prevUpdateTime, F32 crntTime)
{
	ParticleBase::revive(pe, prevUpdateTime, crntTime);

	const ParticleEmitterProperties& props = pe;

	// pre calculate
	Bool forceFlag = props.forceEnabled;
	Bool worldGravFlag = props.wordGravityEnabled;

	// activate it (Bullet stuff)
	body->forceActivationState(ACTIVE_TAG);
	body->activate();
	body->clearForces();
	body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
	body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));

	// force
	if(forceFlag)
	{
		Vec3 forceDir = getRandom(props.particle.forceDirection,
			props.particle.forceDirectionDeviation);
		forceDir.normalize();

		if(!pe.identityRotation)
		{
			// the forceDir depends on the particle emitter rotation
			forceDir = pe.getWorldTransform().getRotation() * forceDir;
		}

		F32 forceMag = getRandom(props.particle.forceMagnitude,
			props.particle.forceMagnitudeDeviation);

		body->applyCentralForce(toBt(forceDir * forceMag));
	}

	// gravity
	if(!worldGravFlag)
	{
		body->setGravity(toBt(getRandom(props.particle.gravity,
			props.particle.gravityDeviation)));
	}

	// Starting pos. In local space
	Vec3 pos = getRandom(props.particle.startingPos,
		props.particle.startingPosDeviation);

	if(pe.identityRotation)
	{
		pos += pe.getWorldTransform().getOrigin();
	}
	else
	{
		pos.transform(pe.getWorldTransform());
	}

	btTransform trf(
		toBt(Transform(pos, pe.getWorldTransform().getRotation(), 1.0)));
	body->setWorldTransform(trf);
}
#endif

//==============================================================================
// ParticleEmitterRenderComponent                                              =
//==============================================================================

/// The derived render component for particle emitters.
class ParticleEmitterRenderComponent: public RenderComponent
{
public:
	ParticleEmitter* m_node;

	ParticleEmitterRenderComponent(ParticleEmitter* node)
	:	RenderComponent(node),
		m_node(node)
	{}

	ANKI_USE_RESULT Error buildRendering(RenderingBuildData& data) override
	{
		return m_node->buildRendering(data);
	}

	const Material& getMaterial() override
	{
		return m_node->m_particleEmitterResource->getMaterial();
	}

	void getRenderWorldTransform(U index, Transform& trf) override
	{
		m_node->getRenderWorldTransform(index, trf);
	}

	Bool getHasWorldTransforms() override
	{
		return true;
	}
};

//==============================================================================
// MoveFeedbackComponent                                                       =
//==============================================================================

/// Feedback component
class MoveFeedbackComponent: public SceneComponent
{
public:
	MoveFeedbackComponent(ParticleEmitter* node)
	:	SceneComponent(SceneComponent::Type::NONE, node)
	{}

	ANKI_USE_RESULT Error update(
		SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<ParticleEmitter&>(node).onMoveComponentUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// ParticleEmitter                                                             =
//==============================================================================

//==============================================================================
ParticleEmitter::ParticleEmitter(SceneGraph* scene)
:	SceneNode(scene)
{}

//==============================================================================
ParticleEmitter::~ParticleEmitter()
{
	// Delete simple particles
	if(m_simulationType == SimulationType::SIMPLE)
	{
		for(ParticleBase* part : m_particles)
		{
			getSceneAllocator().deleteInstance(part);
		}
	}

	m_particles.destroy(getSceneAllocator());
	m_transforms.destroy(getSceneAllocator());
}

//==============================================================================
Error ParticleEmitter::create(
	const CString& name, const CString& filename)
{
	ANKI_CHECK(SceneNode::create(name));
	SceneComponent* comp;

	// Load resource
	ANKI_CHECK(m_particleEmitterResource.load(filename, &getResourceManager()));

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);

	addComponent(comp);
	comp->setAutomaticCleanup(true);

	// Spatial component
	comp = getSceneAllocator().newInstance<SpatialComponent>(this, &m_obb);
	addComponent(comp);
	comp->setAutomaticCleanup(true);

	// Render component
	ParticleEmitterRenderComponent* rcomp = 
		getSceneAllocator().newInstance<ParticleEmitterRenderComponent>(this);

	ANKI_CHECK(rcomp->create());

	addComponent(comp);
	comp->setAutomaticCleanup(true);

	// Other
	m_obb.setCenter(Vec4(0.0));
	m_obb.setExtend(Vec4(1.0, 1.0, 1.0, 0.0));
	m_obb.setRotation(Mat3x4::getIdentity());

	// copy the resource to me
	ParticleEmitterProperties& me = *this;
	const ParticleEmitterProperties& other =
		m_particleEmitterResource->getProperties();
	me = other;

	if(m_usePhysicsEngine)
	{
		createParticlesSimulation(&getSceneGraph());
		m_simulationType = SimulationType::PHYSICS_ENGINE;
	}
	else
	{
		createParticlesSimpleSimulation();
		m_simulationType = SimulationType::SIMPLE;
	}

	// Create the vertex buffer and object
	PtrSize buffSize = m_maxNumOfParticles * VERT_SIZE * 3;
	ANKI_CHECK(m_vertBuff.create(&getSceneGraph().getGrManager(), 
		GL_ARRAY_BUFFER, nullptr, buffSize, 
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

	m_vertBuffMapping = 
		static_cast<U8*>(m_vertBuff.getPersistentMappingAddress());

	return ErrorCode::NONE;
}

//==============================================================================
Error ParticleEmitter::buildRendering(RenderingBuildData& data)
{
	ANKI_ASSERT(data.m_subMeshIndicesCount <= m_transforms.getSize() + 1);

	Error err = ErrorCode::NONE;

	if(m_aliveParticlesCount == 0)
	{
		return ErrorCode::NONE;
	}

	RenderingKey key = data.m_key;
	key.m_lod = 0;

	PipelineHandle ppline;
	err = m_particleEmitterResource->getMaterial().getProgramPipeline(
		key, ppline);
	
	if(!err)
	{
		ppline.bind(data.m_jobs);

		PtrSize offset = 
			(getGlobalTimestamp() % 3) * (m_vertBuff.getSize() / 3);

		// Position
		m_vertBuff.bindVertexBuffer(data.m_jobs, 
			3, GL_FLOAT, false, VERT_SIZE, offset + 0, 0);

		// Scale
		m_vertBuff.bindVertexBuffer(data.m_jobs, 
			1, GL_FLOAT, false, VERT_SIZE, offset + sizeof(F32) * 3, 6);

		// Alpha
		m_vertBuff.bindVertexBuffer(data.m_jobs, 
			1, GL_FLOAT, false, VERT_SIZE, offset + sizeof(F32) * 4, 7);

		data.m_jobs.drawArrays(GL_POINTS, 
			m_aliveParticlesCount,
			data.m_subMeshIndicesCount);
	}

	return err;
}

//==============================================================================
void ParticleEmitter::onMoveComponentUpdate(MoveComponent& move)
{
	m_identityRotation =
		move.getWorldTransform().getRotation() == Mat3x4::getIdentity();
}

//==============================================================================
void ParticleEmitter::createParticlesSimulation(SceneGraph* scene)
{
#if 0
	collShape = getSceneAllocator().newInstance<btSphereShape>(particle.size);

	RigidBody::Initializer binit;
	binit.shape = collShape;
	binit.group = PhysicsWorld::CG_PARTICLE;
	binit.mask = PhysicsWorld::CG_MAP;

	particles.reserve(maxNumOfParticles);

	for(U i = 0; i < maxNumOfParticles; i++)
	{
		binit.mass = getRandom(particle.mass, particle.massDeviation);

		Particle* part = getSceneGraph().newSceneNode<Particle>(
			nullptr, &scene->getPhysics(), binit);

		part->size = getRandom(particle.size, particle.sizeDeviation);
		part->alpha = getRandom(particle.alpha, particle.alphaDeviation);
		part->getRigidBody()->forceActivationState(DISABLE_SIMULATION);

		particles.push_back(part);
	}
#endif
}

//==============================================================================
void ParticleEmitter::createParticlesSimpleSimulation()
{
	m_particles.create(getSceneAllocator(), m_maxNumOfParticles);

	for(U i = 0; i < m_maxNumOfParticles; i++)
	{
		ParticleSimple* part = 
			getSceneAllocator().newInstance<ParticleSimple>();

		part->m_size = 
			getRandom(m_particle.m_size, m_particle.m_sizeDeviation);
		part->m_alpha = 
			getRandom(m_particle.m_alpha, m_particle.m_alphaDeviation);

		m_particles[i] = part;
	}
}

//==============================================================================
Error ParticleEmitter::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff
	//
	Vec4 aabbmin(MAX_F32, MAX_F32, MAX_F32, 0.0);
	Vec4 aabbmax(MIN_F32, MIN_F32, MIN_F32, 0.0);
	m_aliveParticlesCount = 0;

	F32* verts = (F32*)(m_vertBuffMapping 
		+ (getGlobalTimestamp() % 3) * (m_vertBuff.getSize() / 3));
	F32* verts_base = verts;
	(void)verts_base;

	for(ParticleBase* p : m_particles)
	{
		if(p->isDead())
		{
			// if its already dead so dont deactivate it again
			continue;
		}

		if(p->getTimeOfDeath() < crntTime)
		{
			// Just died
			p->kill();
		}
		else
		{
			// It's alive

			// Do checks
			ANKI_ASSERT(((PtrSize)verts + VERT_SIZE 
				- (PtrSize)m_vertBuffMapping) <= m_vertBuff.getSize());

			// This will calculate a new world transformation
			p->simulate(*this, prevUpdateTime, crntTime);

			const Vec4& origin = p->getPosition();

			for(U i = 0; i < 3; i++)
			{
				aabbmin[i] = std::min(aabbmin[i], origin[i]);
				aabbmax[i] = std::max(aabbmax[i], origin[i]);
			}

			F32 lifePercent = (crntTime - p->getTimeOfBirth())
				/ (p->getTimeOfDeath() - p->getTimeOfBirth());

			verts[0] = origin.x();
			verts[1] = origin.y();
			verts[2] = origin.z();

			// XXX set a flag for scale
			verts[3] = p->m_size + (lifePercent * m_particle.m_sizeAnimation);

			// Set alpha
			if(m_particle.m_alphaAnimation)
			{
				verts[4] = sin((lifePercent) * getPi<F32>()) * p->m_alpha;
			}
			else
			{
				verts[4] = p->m_alpha;
			}

			++m_aliveParticlesCount;
			verts += 5;
		}
	}

	if(m_aliveParticlesCount != 0)
	{
		Vec4 min = aabbmin - m_particle.m_size;
		Vec4 max = aabbmax + m_particle.m_size;
		Vec4 center = (min + max) / 2.0;

		m_obb = Obb(center, Mat3x4::getIdentity(), max - center);
	}
	else
	{
		m_obb = Obb(Vec4(0.0), Mat3x4::getIdentity(), Vec4(0.001));
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
			if(particlesCount >= m_particlesPerEmittion)
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

	// Do something more
	return doInstancingCalcs();
}

//==============================================================================
Error ParticleEmitter::doInstancingCalcs()
{
	Error err = ErrorCode::NONE;

#if 0
	//
	// Gather the move components of the instances
	//
	SceneFrameDArrayAuto<MoveComponent*> instanceMoves(
		getFrameAllocator());
	U instanceMovesCount = 0;
	Timestamp instancesTimestamp = 0;

	err = instanceMoves.create(64);
	if(err)	return err;

	err = SceneNode::visitChildren([&](SceneNode& sn) -> Error
	{	
		if(sn.tryGetComponent<InstanceComponent>())
		{
			MoveComponent& move = sn.getComponent<MoveComponent>();

			instanceMoves[instanceMovesCount++] = &move;

			instancesTimestamp = 
				std::max(instancesTimestamp, move.getTimestamp());
		}

		return ErrorCode::NONE;
	});

	//
	// If instancing
	//
	if(instanceMovesCount > 0)
	{
		Bool transformsNeedUpdate = false;

		// Check if an instance was added or removed and reset the spatials and
		// the transforms
		if(instanceMovesCount != m_transforms.getSize())
		{
			transformsNeedUpdate = true;

			// Check if instances added or removed
			if(m_transforms.getSize() < instanceMovesCount)
			{
				// Instances added

				U diff = instanceMovesCount - m_transforms.getSize();

				while(diff-- != 0)
				{
					ObbSpatialComponent* newSpatial = getSceneAllocator().
						newInstance<ObbSpatialComponent>(this);

					if(newSpatial == nullptr)
					{
						err = ErrorCode::OUT_OF_MEMORY;
						break;
					}

					err = addComponent(newSpatial);
					if(err)
					{
						break;
					}
				}
			}
			else
			{
				// Instances removed

				// TODO
				ANKI_ASSERT(0 && "TODO");
			}

			err = m_transforms.resize(getSceneAllocator(), instanceMovesCount);
		}

		if(!err && (transformsNeedUpdate 
			|| m_transformsTimestamp < instancesTimestamp))
		{
			m_transformsTimestamp = instancesTimestamp;

			// Update the transforms
			for(U i = 0; i < instanceMovesCount; i++)
			{
				m_transforms[i] = instanceMoves[i]->getWorldTransform();
			}
		}

		// Update the spatials anyway
		if(!err)
		{
			U count = 0;
			SpatialComponent* meSpatial = this;
			err = iterateComponentsOfType<SpatialComponent>(
				[&](SpatialComponent& sp) -> Error
			{
				Error err2 = ErrorCode::NONE;

				// Skip the first
				if(&sp != meSpatial)	
				{
					ObbSpatialComponent* msp = 
						staticCastPtr<ObbSpatialComponent*>(&sp);

					if(msp)
					{
						Obb aobb = m_obb;
						aobb.setCenter(Vec4(0.0));
						msp->m_obb = aobb.getTransformed(m_transforms[count]);
						++count;
						msp->markForUpdate();
					}
					else
					{
						err2 = ErrorCode::OUT_OF_MEMORY;
					}
				}

				return err2;
			});

			ANKI_ASSERT(count == m_transforms.getSize());
		}
	} // end if instancing
#endif

	return err;
}

//==============================================================================
void ParticleEmitter::getRenderWorldTransform(U index, Transform& trf)
{
	ANKI_ASSERT(m_transforms.getSize() > 0);

	if(index == 0)
	{
		// Don't transform the particle positions. They are already in world 
		// space
		trf = Transform::getIdentity();
	}
	else
	{
		--index;
		ANKI_ASSERT(index < m_transforms.getSize());

		// The particle positions are already in word space. Move them back to
		// local space
		const MoveComponent& move = getComponent<MoveComponent>();
		Transform invTrf = move.getWorldTransform().getInverse();
		trf = m_transforms[index].combineTransformations(invTrf);
	}
}

} // end namespace anki
