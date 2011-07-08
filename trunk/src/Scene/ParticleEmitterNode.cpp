#include <boost/foreach.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "ParticleEmitterNode.h"
#include "Particle.h"
#include "Physics/RigidBody.h"
#include "Core/App.h"
#include "Scene.h"
#include "Util/Util.h"


btTransform ParticleEmitterNode::startingTrf(toBt(Mat3::getIdentity()),
	btVector3(10000000.0, 10000000.0, 10000000.0));


//==============================================================================
// Destructor                                                                  =
//==============================================================================
ParticleEmitterNode::~ParticleEmitterNode()
{}


//==============================================================================
// getRandom                                                                   =
//==============================================================================
float ParticleEmitterNode::getRandom(float initial, float deviation)
{
	return (deviation == 0.0) ?  initial :
		initial + Util::randFloat(deviation) * 2.0 - deviation;
}


//==============================================================================
// getRandom                                                                   =
//==============================================================================
Vec3 ParticleEmitterNode::getRandom(const Vec3& initial, const Vec3& deviation)
{
	if(deviation == Vec3(0.0))
	{
		return initial;
	}
	else
	{
		Vec3 out;
		for(int i = 0; i < 3; i++)
		{
			out[i] = getRandom(initial[i], deviation[i]);
		}
		return out;
	}
}


//==============================================================================
// init                                                                        =
//==============================================================================
void ParticleEmitterNode::init(const char* filename)
{
	particleEmitterProps.loadRsrc(filename);

	// copy the resource to me
	ParticleEmitterRsrc& me = *this;
	ParticleEmitterRsrc& other = *particleEmitterProps.get();
	me = other;

	// create the particles
	collShape.reset(new btSphereShape(size));

	for(uint i = 0; i < maxNumOfParticles; i++)
	{
		ModelNode* md = new ModelNode(false, this);
		md->init(modelName.c_str());
		md->setLocalTransform(Transform(Vec3(Util::randFloat(i * 1.0)),
			Mat3::getIdentity(), 1.0));

		continue;

		Particle* particle = new Particle(-1.0, this);
		particle->init(modelName.c_str());
		particle->setLocalTransform(Transform(Vec3(Util::randFloat(i * 1.0)),
			Mat3::getIdentity(), 1.0));

		continue;

		particles.push_back(particle);

		float mass = particleMass +
			Util::randFloat(particleMassDeviation) * 2.0 -
			particleMassDeviation;

		Phys::RigidBody::Initializer init;
		init.mass = mass;
		init.startTrf = toAnki(startingTrf);
		init.shape = collShape.get();
		init.sceneNode = NULL;
		init.group = Phys::MasterContainer::CG_PARTICLE;
		init.mask = Phys::MasterContainer::CG_ALL ^
			Phys::MasterContainer::CG_PARTICLE;

		Phys::RigidBody* body = new Phys::RigidBody(
			SceneSingleton::getInstance().getPhysMasterContainer(), init);

		body->forceActivationState(DISABLE_SIMULATION);

		particle->setNewRigidBody(body);
	}
}


//==============================================================================
// frameUpdate                                                                 =
//==============================================================================
void ParticleEmitterNode::frameUpdate(float prevUpdateTime, float crntTime)
{
	// Opt: We dont have to make extra calculations if the ParticleEmitterNode's
	// rotation is the identity
	bool identRot = getWorldTransform().getRotation() == Mat3::getIdentity();

	// deactivate the dead particles
	BOOST_FOREACH(Particle* p, particles)
	{
		if(p->isDead()) // its already dead so dont deactivate it again
		{
			continue;
		}

		if(p->getTimeOfDeath() < crntTime)
		{
			//cout << "Killing " << i << " " << p.timeOfDeath << endl;
			p->getRigidBody().setActivationState(DISABLE_SIMULATION);
			p->getRigidBody().setWorldTransform(startingTrf);
			p->setTimeOfDeath(-1.0);
		}
	}

	// pre calculate
	bool forceFlag = hasForce();
	bool worldGravFlag = usingWorldGrav();

	if(timeLeftForNextEmission <= 0.0)
	{
		uint partNum = 0;
		BOOST_FOREACH(Particle* pp, particles)
		{
			Particle& p = *pp;
			if(!p.isDead())
			{
				// its alive so skip it
				continue;
			}

			//INFO("Reiniting " << i);

			// life
			p.setTimeOfDeath(getRandom(crntTime + particleLife,
				particleLifeDeviation));

			//cout << "Time of death " << p.timeOfDeath << endl;
			//cout << "Particle life " << p.timeOfDeath - crntTime << endl;

			// activate it (Bullet stuff)
			p.getRigidBody().forceActivationState(ACTIVE_TAG);
			p.getRigidBody().activate();
			p.getRigidBody().clearForces();
			p.getRigidBody().setLinearVelocity(btVector3(0.0, 0.0, 0.0));
			p.getRigidBody().setAngularVelocity(btVector3(0.0, 0.0, 0.0));

			//cout << p.body->internalGetDeltaAngularVelocity() << endl;

			// force
			if(forceFlag)
			{
				Vec3 forceDir = getRandom(forceDirection,
					forceDirectionDeviation);
				forceDir.normalize();

				if(!identRot)
				{
					// the forceDir depends on the particle emitter rotation
					forceDir = getWorldTransform().getRotation() * forceDir;
				}

				float forceMag = getRandom(forceMagnitude,
					forceMagnitudeDeviation);

				p.getRigidBody().applyCentralForce(toBt(forceDir * forceMag));
			}

			// gravity
			if(!worldGravFlag)
			{
				p.getRigidBody().setGravity(toBt(getRandom(gravity,
					gravityDeviation)));
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

			btTransform trf;
			trf.setIdentity();
			trf.setOrigin(toBt(pos));
			p.getRigidBody().setWorldTransform(trf);

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
