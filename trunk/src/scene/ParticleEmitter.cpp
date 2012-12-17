#include "anki/scene/ParticleEmitter.h"
#include "anki/scene/Scene.h"
#include "anki/resource/Model.h"
#include "anki/util/Functions.h"
#include "anki/physics/PhysWorld.h"
#include <limits>

namespace anki {

//==============================================================================
// ParticleSimple                                                              =
//==============================================================================

//==============================================================================
ParticleSimple::ParticleSimple(
	// SceneNode
	const char* name, Scene* scene, 
	// Movable
	U32 movableFlags, Movable* movParent)
	: SceneNode(name, scene), Movable(movableFlags, movParent, *this)
{}

//==============================================================================
ParticleSimple::~ParticleSimple()
{}

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
	: ParticleSimple(name, scene, movableFlags, movParent),
		RigidBody(masterContainer, init, this)
{}

//==============================================================================
Particle::~Particle()
{}

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
	: SceneNode(name, scene), Spatial(this, &aabb),
		Movable(movableFlags, movParent, *this)
{
	init(filename, scene);

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
{}

//==============================================================================
F32 ParticleEmitter::getRandom(F32 initial, F32 deviation)
{
	return (deviation == 0.0) ? initial
		: initial + randFloat(deviation) * 2.0 - deviation;
}

//==============================================================================
Vec3 ParticleEmitter::getRandom(const Vec3& initial, const Vec3& deviation)
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
void ParticleEmitter::init(const char* filename, Scene* scene)
{
	particleEmitterResource.load(filename);

	// copy the resource to me
	ParticleEmitterProperties& me = *this;
	const ParticleEmitterProperties& other = *particleEmitterResource;
	me = other;

	// create the particles
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

		particles.push_back(part);

		part->forceActivationState(DISABLE_SIMULATION);
	}

	timeLeftForNextEmission = 0.0;
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
	for(Particle* p : particles)
	{
		if(p->isDead())
		{
			// if its already dead so dont deactivate it again
			continue;
		}

		if(p->getTimeOfDeath() < crntTime)
		{
			// Just died
			p->setActivationState(DISABLE_SIMULATION);
			p->setTimeOfDeath(-1.0);
		}
		else
		{
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
				particle.size + (lifePercent * particle.sizeAnimation));

			instancingTransformations.push_back(trf);

			// Set alpha
			if(alphaRenderableVar)
			{
				alpha.push_back((1.0 - lifePercent) * particle.alpha);
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
		for(Particle* pp : particles)
		{
			Particle& p = *pp;
			if(!p.isDead())
			{
				// its alive so skip it
				continue;
			}

			reanimateParticle(p, crntTime);

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

//==============================================================================
void ParticleEmitter::reanimateParticle(ParticleSimple& p, F32 crntTime)
{
	ANKI_ASSERT(p.isDead());

	ANKI_ASSERT(p.getRigidBody() != nullptr);
	RigidBody& body = *p.getRigidBody();

	// pre calculate
	Bool forceFlag = particleEmitterResource->hasForce();
	Bool worldGravFlag = particleEmitterResource->usingWorldGravity();

	// life
	p.setTimeOfDeath(
		getRandom(crntTime + particle.life, particle.lifeDeviation));
	p.setTimeOfBirth(crntTime);

	// activate it (Bullet stuff)
	body.forceActivationState(ACTIVE_TAG);
	body.activate();
	body.clearForces();
	body.setLinearVelocity(btVector3(0.0, 0.0, 0.0));
	body.setAngularVelocity(btVector3(0.0, 0.0, 0.0));

	// force
	if(forceFlag)
	{
		Vec3 forceDir = getRandom(particle.forceDirection,
			particle.forceDirectionDeviation);
		forceDir.normalize();

		if(!identityRotation)
		{
			// the forceDir depends on the particle emitter rotation
			forceDir = getWorldTransform().getRotation() * forceDir;
		}

		F32 forceMag = getRandom(particle.forceMagnitude,
			particle.forceMagnitudeDeviation);

		body.applyCentralForce(toBt(forceDir * forceMag));
	}

	// gravity
	if(!worldGravFlag)
	{
		body.setGravity(
			toBt(getRandom(particle.gravity, particle.gravityDeviation)));
	}

	// Starting pos. In local space
	Vec3 pos = getRandom(particle.startingPos, particle.startingPosDeviation);

	if(identityRotation)
	{
		pos += getWorldTransform().getOrigin();
	}
	else
	{
		pos.transform(getWorldTransform());
	}

	btTransform trf(
		toBt(Transform(pos, getWorldTransform().getRotation(), 1.0)));
	body.setWorldTransform(trf);
}

} // end namespace anki
