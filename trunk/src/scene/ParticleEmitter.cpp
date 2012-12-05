#include "anki/scene/ParticleEmitter.h"
#include "anki/scene/Scene.h"
#include "anki/resource/Model.h"
#include "anki/util/Functions.h"
#include "anki/physics/PhysWorld.h"
#include <limits>

namespace anki {

//==============================================================================
// Particle                                                                    =
//==============================================================================

//==============================================================================
Particle::Particle(
	F32 timeOfDeath_,
	// Scene
	const char* name, Scene* scene,
	// Movable
	U32 movableFlags, Movable* movParent, 
	// RigidBody
	PhysWorld* masterContainer, const Initializer& init)
	: SceneNode(name, scene), Movable(movableFlags, movParent, *this),
		 RigidBody(masterContainer, init, this), timeOfDeath(timeOfDeath_)
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
	collShape.reset(new btSphereShape(size));

	RigidBody::Initializer binit;
	binit.shape = collShape.get();
	binit.group = PhysWorld::CG_PARTICLE;
	binit.mask = PhysWorld::CG_MAP;

	for(U i = 0; i < maxNumOfParticles; i++)
	{
		binit.mass = getRandom(particleMass, particleMassDeviation);

		Particle* particle = new Particle(
			-1.0, 
			(getName() + std::to_string(i)).c_str(), scene,
			Movable::MF_NONE, nullptr,
			&scene->getPhysics(), binit);

		particles.push_back(particle);

		particle->forceActivationState(DISABLE_SIMULATION);
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
	for(Particle* p : particles)
	{
		// if its already dead so dont deactivate it again
		if(!p->isDead() && p->getTimeOfDeath() < crntTime)
		{
			p->setActivationState(DISABLE_SIMULATION);
			p->setTimeOfDeath(-1.0);
		}
		else
		{
			const Vec3& origin = p->Movable::getWorldTransform().getOrigin();

			for(U i = 0; i < 3; i++)
			{
				aabbmin[i] = std::min(aabbmin[i], origin[i]);
				aabbmax[i] = std::max(aabbmax[i], origin[i]);
			}

			instancingTransformations.push_back(
				p->Movable::getWorldTransform());
		}
	}

	instancesCount = instancingTransformations.size();
	if(instancesCount != 0)
	{
		aabb = Aabb(aabbmin - size, aabbmax + size);
	}
	else
	{
		aabb = Aabb(Vec3(0.0), Vec3(0.01));
	}
	spatialMarkUpdated();

	// pre calculate
	Bool forceFlag = particleEmitterResource->hasForce();
	Bool worldGravFlag = particleEmitterResource->usingWorldGravity();

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

			RigidBody& body = p;

			// life
			p.setTimeOfDeath(
				getRandom(crntTime + particleLife, particleLifeDeviation));

			//cout << "Time of death " << p.timeOfDeath << endl;
			//cout << "Particle life " << p.timeOfDeath - crntTime << endl;

			// activate it (Bullet stuff)
			body.forceActivationState(ACTIVE_TAG);
			body.activate();
			body.clearForces();
			body.setLinearVelocity(btVector3(0.0, 0.0, 0.0));
			body.setAngularVelocity(btVector3(0.0, 0.0, 0.0));

			//cout << p.body->internalGetDeltaAngularVelocity() << endl;

			// force
			if(forceFlag)
			{
				Vec3 forceDir =
					getRandom(forceDirection, forceDirectionDeviation);
				forceDir.normalize();

				if(!identityRotation)
				{
					// the forceDir depends on the particle emitter rotation
					forceDir = getWorldTransform().getRotation() * forceDir;
				}

				F32 forceMag =
					getRandom(forceMagnitude, forceMagnitudeDeviation);

				body.applyCentralForce(toBt(forceDir * forceMag));
			}

			// gravity
			if(!worldGravFlag)
			{
				body.setGravity(toBt(getRandom(gravity, gravityDeviation)));
			}

			// Starting pos. In local space
			Vec3 pos = getRandom(startingPos, startingPosDeviation);

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
