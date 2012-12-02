#include "anki/scene/ParticleEmitter.h"
#include "anki/resource/Model.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
// Particle                                                                    =
//==============================================================================

//==============================================================================
Particle::Particle(F32 timeOfDeath_,
	const char* name, Scene* scene, // Scene
	U32 movableFlags, Movable* movParent, // Movable
	PhysWorld* masterContainer, const Initializer& init)
	: SceneNode(name, scene), Movable(movableFlags, movParent, *this),
		 RigidBody(masterContainer, init), timeOfDeath(timeOfDeath_)
{}

//==============================================================================
Particle::~Particle()
{}

//==============================================================================
// ParticleEmitter                                                             =
//==============================================================================

//==============================================================================
ParticleEmitter::ParticleEmitter(const char* filename,
	const char* name, Scene* scene, // Scene
	U32 movableFlags, Movable* movParent) // Movable
	: SceneNode(name, scene), Spatial(this, &aabb),
		Movable(movableFlags, movParent, *this)
{
	init(filename);
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
void ParticleEmitter::init(const char* filename)
{
	particleEmitterResource.load(filename);

	// copy the resource to me
	ParticleEmitterProperties& me = *this;
	const ParticleEmitterProperties& other = *particleEmitterResource;
	me = other;

	// create the particles
	collShape.reset(new btSphereShape(size));

	for(U i = 0; i < maxNumOfParticles; i++)
	{
		Particle* particle = new Particle(-1.0, getScene(), SNF_NONE, NULL);
		particle->init(modelName.c_str());
		particle->disableFlag(SceneNode::SNF_ACTIVE);
		particle->setWorldTransform(Transform(Vec3(1000000.0),
			Mat3::getIdentity(), 1.0));

		particles.push_back(particle);

		float mass = particleMass +
			Util::randFloat(particleMassDeviation) * 2.0 -
			particleMassDeviation;

		RigidBody::Initializer init;
		init.mass = mass;
		init.startTrf = toAnki(startingTrf);
		init.shape = collShape.get();
		init.sceneNode = particle;
		init.group = PhysWorld::CG_PARTICLE;
		init.mask = PhysWorld::CG_ALL ^
			PhysWorld::CG_PARTICLE;

		RigidBody* body = new RigidBody(
			getScene().getPhysWorld(), init);

		body->forceActivationState(DISABLE_SIMULATION);

		particle->setNewRigidBody(body);
	}

	timeLeftForNextEmission = 0.0;
}

//==============================================================================
void ParticleEmitter::frameUpdate(F32 prevUpdateTime, F32 crntTime, I frame)
{
	SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);

	// Opt: We dont have to make extra calculations if the ParticleEmitterNode's
	// rotation is the identity
	Bool identRot = getWorldTransform().getRotation() == Mat3::getIdentity();

	// deactivate the dead particles
	for(Particle* p : particles)
	{
		if(p->isDead()) // its already dead so dont deactivate it again
		{
			continue;
		}

		if(p->getTimeOfDeath() < crntTime)
		{
			//cout << "Killing " << i << " " << p.timeOfDeath << endl;
			p->setActivationState(DISABLE_SIMULATION);
			p->setTimeOfDeath(-1.0);
		}
	}

	// pre calculate
	Bool forceFlag = particleEmitterResource->hasForce();
	Bool worldGravFlag = particleEmitterResource->usingWorldGravity();

	if(timeLeftForNextEmission <= 0.0)
	{
		U partNum = 0;
		for(Particle* pp : particles)
		{
			Particle& p = *pp;
			if(!p.isDead())
			{
				// its alive so skip it
				continue;
			}

			RigidBody& body = p.getRigidBody();

			// life
			p.setTimeOfDeath(getRandom(crntTime + particleLife,
				particleLifeDeviation));

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
				Vec3 forceDir = getRandom(
					forceDirection, forceDirectionDeviation);
				forceDir.normalize();

				if(!identRot)
				{
					// the forceDir depends on the particle emitter rotation
					forceDir = getWorldTransform().getRotation() * forceDir;
				}

				float forceMag = getRandom(forceMagnitude,
					forceMagnitudeDeviation);

				body.applyCentralForce(toBt(forceDir * forceMag));
			}

			// gravity
			if(!worldGravFlag)
			{
				body.setGravity(toBt(getRandom(gravity, gravityDeviation)));
			}

			// Starting pos. In local space
			Vec3 pos = getRandom(startingPos, startingPosDeviation);

			if(identRot)
			{
				pos += getWorldTransform().getOrigin();
			}
			else
			{
				pos.transform(getWorldTransform());
			}

			btTransform trf(toBt(Transform(pos,
				getWorldTransform().getRotation(), 1.0)));
			body.setWorldTransform(trf);

			// do the rest
			++partNum;
			if(partNum >= particlesPerEmittion)
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
