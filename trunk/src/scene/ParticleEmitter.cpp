#include "anki/scene/ParticleEmitter.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/InstanceNode.h"
#include "anki/scene/Misc.h"
#include "anki/resource/Model.h"
#include "anki/util/Functions.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/gl/Drawcall.h"
#include <limits>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
F32 getRandom(F32 initial, F32 deviation)
{
	return (deviation == 0.0) 
		? initial
		: initial + randFloat(deviation) * 2.0 - deviation;
}

//==============================================================================
Vec3 getRandom(const Vec3& initial, const Vec3& deviation)
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
void ParticleBase::revive(const ParticleEmitter& pe,
	F32 /*prevUpdateTime*/, F32 crntTime)
{
	ANKI_ASSERT(isDead());
	const ParticleEmitterProperties& props = pe;

	// life
	timeOfDeath = getRandom(crntTime + props.particle.life,
		props.particle.lifeDeviation);
	timeOfBirth = crntTime;
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
		particle.gravity.getLength() > 0.0);

	Vec3 xp = position;
	Vec3 xc = acceleration * (dt * dt) + velocity * dt + xp;

	position = xc;

	velocity += acceleration * dt;
}

//==============================================================================
void ParticleSimple::revive(const ParticleEmitter& pe,
	F32 prevUpdateTime, F32 crntTime)
{
	ParticleBase::revive(pe, prevUpdateTime, crntTime);
	velocity = Vec3(0.0);

	const ParticleEmitterProperties& props = pe;

	acceleration = getRandom(props.particle.gravity,
			props.particle.gravityDeviation);

	// Set the initial position
	position = getRandom(props.particle.startingPos,
		props.particle.startingPosDeviation);

	position += pe.getWorldTransform().getOrigin();
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
// ParticleEmitter                                                             =
//==============================================================================

//==============================================================================
ParticleEmitter::ParticleEmitter(
	const char* name, SceneGraph* scene,
	const char* filename)
	:	SceneNode(name, scene),
		SpatialComponent(this),
		MoveComponent(this),
		RenderComponent(this),
		particles(getSceneAllocator()),
		transforms(getSceneAllocator()),
		clientBuffer(getSceneAllocator())
{
	addComponent(static_cast<MoveComponent*>(this));
	addComponent(static_cast<SpatialComponent*>(this));
	addComponent(static_cast<RenderComponent*>(this));

	// Load resource
	particleEmitterResource.load(filename);

	// copy the resource to me
	ParticleEmitterProperties& me = *this;
	const ParticleEmitterProperties& other =
		particleEmitterResource->getProperties();
	me = other;

	if(usePhysicsEngine)
	{
		createParticlesSimulation(scene);
		simulationType = PHYSICS_ENGINE_SIMULATION;
	}
	else
	{
		createParticlesSimpleSimulation(scene);
		simulationType = SIMPLE_SIMULATION;
	}

	timeLeftForNextEmission = 0.0;
	RenderComponent::init();

	// Create the vertex buffer and object
	//
	const U components = 3 + 1 + 1;
	PtrSize vertSize = components * sizeof(F32);

	vbo.create(GL_ARRAY_BUFFER, maxNumOfParticles * vertSize,
		nullptr, GL_DYNAMIC_DRAW);

	clientBuffer.resize(maxNumOfParticles * components, 0.0);

	vao.create();
	// Position
	vao.attachArrayBufferVbo(&vbo, 0, 3, GL_FLOAT, GL_FALSE, vertSize, 0);

	// Scale
	vao.attachArrayBufferVbo(&vbo, 6, 1, GL_FLOAT, GL_FALSE, vertSize, 
		sizeof(F32) * 3);

	// Alpha
	vao.attachArrayBufferVbo(&vbo, 7, 1, GL_FLOAT, GL_FALSE, vertSize, 
		sizeof(F32) * 4);
}

//==============================================================================
ParticleEmitter::~ParticleEmitter()
{
	// Delete simple particles
	if(simulationType == SIMPLE_SIMULATION)
	{
		for(ParticleBase* part : particles)
		{
			getSceneAllocator().deleteInstance(part);
		}
	}
}

//==============================================================================
void ParticleEmitter::getRenderingData(
	const PassLodKey& key, 
	const U8* subMeshIndicesArray, U subMeshIndicesCount,
	const Vao*& vao_, const ShaderProgram*& prog,
	Drawcall& dc)
{
	ANKI_ASSERT(subMeshIndicesCount <= transforms.size() + 1);
	vao_ = &vao;
	prog = &getMaterial().findShaderProgram(key);

	dc.primitiveType = GL_POINTS;
	dc.indicesType = 0;

	dc.instancesCount = subMeshIndicesCount;
	dc.drawCount = 1;

	dc.count = aliveParticlesCountDraw;
	dc.offset = 0;
}

//==============================================================================
const Material& ParticleEmitter::getMaterial()
{
	return particleEmitterResource->getMaterial();
}

//==============================================================================
void ParticleEmitter::componentUpdated(SceneComponent& comp, 
	SceneComponent::UpdateType)
{
	if(comp.getType() == MoveComponent::getGlobType())
	{
		identityRotation =
			getWorldTransform().getRotation() == Mat3::getIdentity();
	}
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

		Particle* part;
		getSceneGraph().newSceneNode(part,
			nullptr, &scene->getPhysics(), binit);

		part->size = getRandom(particle.size, particle.sizeDeviation);
		part->alpha = getRandom(particle.alpha, particle.alphaDeviation);
		part->getRigidBody()->forceActivationState(DISABLE_SIMULATION);

		particles.push_back(part);
	}
#endif
}

//==============================================================================
void ParticleEmitter::createParticlesSimpleSimulation(SceneGraph* scene)
{
	for(U i = 0; i < maxNumOfParticles; i++)
	{
		ParticleSimple* part = 
			getSceneAllocator().newInstance<ParticleSimple>();

		part->size = getRandom(particle.size, particle.sizeDeviation);
		part->alpha = getRandom(particle.alpha, particle.alphaDeviation);

		particles.push_back(part);
	}
}

//==============================================================================
void ParticleEmitter::frameUpdate(F32 prevUpdateTime, F32 crntTime, 
	SceneNode::UpdateType uptype)
{
	if(uptype == SceneNode::SYNC_UPDATE)
	{
		// In main thread update the client buffer

		vbo.write(&clientBuffer[0]);
		aliveParticlesCountDraw = aliveParticlesCount;

		return;
	}

	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff
	//
	Vec3 aabbmin(std::numeric_limits<F32>::max());
	Vec3 aabbmax(-std::numeric_limits<F32>::max());
	aliveParticlesCount = 0;

	F32* verts = &clientBuffer[0];
	F32* verts_base = verts;

	for(ParticleBase* p : particles)
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
			// This will calculate a new world transformation
			p->simulate(*this, prevUpdateTime, crntTime);

			// An alive
			const Vec3& origin = p->getPosition();

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
			verts[3] = p->size + (lifePercent * particle.sizeAnimation);

			// Set alpha
			verts[4] = sin((lifePercent) * getPi<F32>()) * p->alpha;

			++aliveParticlesCount;
			verts += 5;

			// Do checks
			ANKI_ASSERT(
				((PtrSize)verts - (PtrSize)verts_base) <= vbo.getSizeInBytes());
		}
	}

	if(aliveParticlesCount != 0)
	{
		Vec3 min = aabbmin - particle.size;
		Vec3 max = aabbmax + particle.size;
		Vec3 center = (min + max) / 2.0;

		obb = Obb(center, Mat3::getIdentity(), max - center);
	}
	else
	{
		obb = Obb(Vec3(0.0), Mat3::getIdentity(), Vec3(0.001));
	}
	SpatialComponent::markForUpdate();

	//
	// Emit new particles
	//
	if(timeLeftForNextEmission <= 0.0)
	{
		U particlesCount = 0; // How many particles I am allowed to emmit
		for(ParticleBase* pp : particles)
		{
			ParticleBase& p = *pp;
			if(!p.isDead())
			{
				// its alive so skip it
				continue;
			}

			p.revive(*this, prevUpdateTime, crntTime);

			// do the rest
			++particlesCount;
			if(particlesCount >= particlesPerEmittion)
			{
				break;
			}
		} // end for all particles

		timeLeftForNextEmission = emissionPeriod;
	} // end if can emit
	else
	{
		timeLeftForNextEmission -= crntTime - prevUpdateTime;
	}

	// Do something more
	doInstancingCalcs();
}

//==============================================================================
void ParticleEmitter::doInstancingCalcs()
{
	// Gather the move components of the instances
	SceneFrameVector<MoveComponent*> instanceMoves(getSceneFrameAllocator());
	Timestamp instancesTimestamp = 0;
	SceneNode::visitThisAndChildren([&](SceneObject& so)
	{
		SceneNode* sn = staticCast<SceneNode*>(&so);
		
		if(sn->tryGetComponent<InstanceComponent>())
		{
			MoveComponent& move = sn->getComponent<MoveComponent>();

			instanceMoves.push_back(&move);

			instancesTimestamp = 
				std::max(instancesTimestamp, move.getTimestamp());
		}
	});

	// If instancing
	if(instanceMoves.size() != 0)
	{
		Bool transformsNeedUpdate = false;

		// Check if an instance was added or removed and reset the spatials and
		// the transforms
		if(instanceMoves.size() != transforms.size())
		{
			transformsNeedUpdate = true;

			// Check if instances added or removed
			if(transforms.size() < instanceMoves.size())
			{
				// Instances added

				U diff = instanceMoves.size() - transforms.size();

				while(diff-- != 0)
				{
					ObbSpatialComponent* newSpatial = getSceneAllocator().
						newInstance<ObbSpatialComponent>(this);

					addComponent(newSpatial);
				}
			}
			else
			{
				// Instances removed

				// TODO
				ANKI_ASSERT(0 && "TODO");
			}

			transforms.resize(instanceMoves.size());
		}

		if(transformsNeedUpdate || transformsTimestamp < instancesTimestamp)
		{
			transformsTimestamp = instancesTimestamp;

			// Update the transforms
			for(U i = 0; i < instanceMoves.size(); i++)
			{
				transforms[i] = instanceMoves[i]->getWorldTransform();
			}
		}

		// Update the spatials anyway
		U count = 0;
		iterateComponentsOfType<SpatialComponent>([&](SpatialComponent& sp)
		{
			// Skip the first
			if(count != 0)	
			{
				ObbSpatialComponent* msp = 
					staticCast<ObbSpatialComponent*>(&sp);
		
				Obb aobb = obb;
				aobb.setCenter(Vec3(0.0));
				msp->obb = aobb.getTransformed(transforms[count - 1]);

				msp->markForUpdate();
			}

			++count;
		});

		ANKI_ASSERT(count - 1 == transforms.size());
	} // end if instancing
}

//==============================================================================
Bool ParticleEmitter::getHasWorldTransforms()
{
	if(transforms.size() == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//==============================================================================
void ParticleEmitter::getRenderWorldTransform(U index, Transform& trf)
{
	ANKI_ASSERT(transforms.size() > 0);

	if(index == 0)
	{
		// Don't transform the particle positions. They are already in world 
		// space
		trf = Transform::getIdentity();
	}
	else
	{
		--index;
		ANKI_ASSERT(index < transforms.size());

		// The particle positions are already in word space. Move them back to
		// local space
		Transform invTrf = getWorldTransform().getInverse();
		trf = Transform::combineTransformations(transforms[index], invTrf);
	}
}

} // end namespace anki
