#include "anki/scene/ParticleEmitter.h"
#include "anki/scene/Scene.h"
#include "anki/resource/Model.h"
#include "anki/util/Functions.h"
#include "anki/physics/PhysWorld.h"
#include <limits>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
F32 getRandom(F32 initial, F32 deviation)
{
	return (deviation == 0.0) ? initial
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
ParticleBase::ParticleBase(
	ParticleType type_,
	// SceneNode
	const char* name, Scene* scene, 
	// Movable
	U32 movableFlags, Movable* movParent)
	: SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		type(type_)
{}

//==============================================================================
ParticleBase::~ParticleBase()
{}

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
ParticleSimple::ParticleSimple(
	// SceneNode
	const char* name, Scene* scene, 
	// Movable
	U32 movableFlags, Movable* movParent)
	: ParticleBase(PT_SIMPLE, name, scene, movableFlags, movParent)
{}

//==============================================================================
void ParticleSimple::simulate(const ParticleEmitter& pe,
	F32 prevUpdateTime, F32 crntTime)
{
	F32 dt = crntTime - prevUpdateTime;

	ANKI_ASSERT(
		static_cast<const ParticleEmitterProperties&>(pe).
		particle.gravity.getLength() > 0.0);

	Transform trf = getWorldTransform();
	Vec3 xp = trf.getOrigin();
	Vec3 xc = acceleration * (dt * dt) + velocity * dt + xp;

	trf.setOrigin(xc);

	setLocalTransform(trf);

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
	Vec3 pos = getRandom(props.particle.startingPos,
		props.particle.startingPosDeviation);

	pos += pe.getWorldTransform().getOrigin();

	Transform trf;
	trf.setIdentity();
	trf.setOrigin(pos);
	setLocalTransform(trf);
}

//==============================================================================
// Particle                                                                    =
//==============================================================================

//==============================================================================
Particle::Particle(
	// Scene
	const char* name, Scene* scene,
	// Movable
	U32 movableFlags, Movable* movParent, 
	// RigidBody
	PhysWorld* masterContainer, const Initializer& init)
	: ParticleBase(PT_PHYSICS, name, scene, movableFlags, movParent),
		RigidBody(masterContainer, init, this)
{}

//==============================================================================
Particle::~Particle()
{}

//==============================================================================
void Particle::revive(const ParticleEmitter& pe,
	F32 prevUpdateTime, F32 crntTime)
{
	ParticleBase::revive(pe, prevUpdateTime, crntTime);

	const ParticleEmitterProperties& props = pe;

	RigidBody& body = *this;

	// pre calculate
	Bool forceFlag = props.forceEnabled;
	Bool worldGravFlag = props.wordGravityEnabled;

	// activate it (Bullet stuff)
	body.forceActivationState(ACTIVE_TAG);
	body.activate();
	body.clearForces();
	body.setLinearVelocity(btVector3(0.0, 0.0, 0.0));
	body.setAngularVelocity(btVector3(0.0, 0.0, 0.0));

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

		body.applyCentralForce(toBt(forceDir * forceMag));
	}

	// gravity
	if(!worldGravFlag)
	{
		body.setGravity(toBt(getRandom(props.particle.gravity,
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
	body.setWorldTransform(trf);
}

//==============================================================================
// ParticleEmitter                                                             =
//==============================================================================

//==============================================================================
ParticleEmitter::ParticleEmitter(
	const char* filename,
	// SceneNode
	const char* name, Scene* scene, 
	// Movable
	U32 movableFlags, Movable* movParent)
	:	SceneNode(name, scene),
		Spatial(this, &aabb),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		Renderable(getSceneAllocator())
{
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
	}
	else
	{
		createParticlesSimpleSimulation(scene);
	}

	timeLeftForNextEmission = 0.0;
	instancesCount = particles.size();
	Renderable::init(*this);

	// Find the "alpha" material variable
	for(auto it = Renderable::getVariablesBegin();
		it != Renderable::getVariablesEnd(); ++it)
	{
		if((*it)->getName() == "alpha")
		{
			alphaRenderableVar = *it;
		}
	}
}

//==============================================================================
ParticleEmitter::~ParticleEmitter()
{
	for(ParticleBase* part : particles)
	{
		delete part;
	}
}

//==============================================================================
const ModelPatchBase& ParticleEmitter::getRenderableModelPatchBase() const
{
	return *particleEmitterResource->getModel().getModelPatches()[0];
}

//==============================================================================
const Material& ParticleEmitter::getRenderableMaterial() const
{
	return
		particleEmitterResource->getModel().getModelPatches()[0]->getMaterial();
}

//==============================================================================
void ParticleEmitter::movableUpdate()
{
	identityRotation =
		getWorldTransform().getRotation() == Mat3::getIdentity();
}

//==============================================================================
void ParticleEmitter::createParticlesSimulation(Scene* scene)
{
	collShape.reset(new btSphereShape(particle.size));

	RigidBody::Initializer binit;
	binit.shape = collShape.get();
	binit.group = PhysWorld::CG_PARTICLE;
	binit.mask = PhysWorld::CG_MAP;

	for(U i = 0; i < maxNumOfParticles; i++)
	{
		binit.mass = getRandom(particle.mass, particle.massDeviation);

		Particle* part = new Particle(
			(getName() + std::to_string(i)).c_str(), scene,
			Movable::MF_NONE, nullptr,
			&scene->getPhysics(), binit);

		part->size = getRandom(particle.size, particle.sizeDeviation);
		part->alpha = getRandom(particle.alpha, particle.alphaDeviation);
		part->forceActivationState(DISABLE_SIMULATION);

		particles.push_back(part);
	}
}

//==============================================================================
void ParticleEmitter::createParticlesSimpleSimulation(Scene* scene)
{
	for(U i = 0; i < maxNumOfParticles; i++)
	{
		ParticleSimple* part = new ParticleSimple(
			(getName() + std::to_string(i)).c_str(),
			scene, Movable::MF_NONE, nullptr);

		part->size = getRandom(particle.size, particle.sizeDeviation);
		part->alpha = getRandom(particle.alpha, particle.alphaDeviation);

		particles.push_back(part);
	}
}

//==============================================================================
void ParticleEmitter::frameUpdate(F32 prevUpdateTime, F32 crntTime, I frame)
{
	SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);

	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff
	//
	Vec3 aabbmin(std::numeric_limits<F32>::max());
	Vec3 aabbmax(std::numeric_limits<F32>::min());
	instancingTransformations.clear();
	Vector<F32> alpha;
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
			const Vec3& origin = p->Movable::getWorldTransform().getOrigin();

			for(U i = 0; i < 3; i++)
			{
				aabbmin[i] = std::min(aabbmin[i], origin[i]);
				aabbmax[i] = std::max(aabbmax[i], origin[i]);
			}

			F32 lifePercent = (crntTime - p->getTimeOfBirth())
				/ (p->getTimeOfDeath() - p->getTimeOfBirth());

			Transform trf = p->Movable::getWorldTransform();
			// XXX set a flag for scale
			trf.setScale(
				p->size + (lifePercent * particle.sizeAnimation));

			instancingTransformations.push_back(trf);

			// Set alpha
			if(alphaRenderableVar)
			{
				alpha.push_back(
					sin((lifePercent) * getPi<F32>())
					* p->alpha);
			}
		}
	}

	instancesCount = instancingTransformations.size();
	if(instancesCount != 0)
	{
		aabb = Aabb(aabbmin - particle.size, aabbmax + particle.size);
	}
	else
	{
		aabb = Aabb(Vec3(0.0), Vec3(0.01));
	}
	spatialMarkUpdated();

	if(alphaRenderableVar)
	{
		alphaRenderableVar->setValues(&alpha[0], alpha.size());
	}

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
}

} // end namespace anki
