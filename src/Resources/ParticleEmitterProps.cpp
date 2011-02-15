#include <cstring>
#include "ParticleEmitterProps.h"
#include "Exception.h"


static const char* errMsg = "Incorrect or missing value ";


#define PE_EXCEPTION(x) EXCEPTION("Particle emmiter: " + x)


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
ParticleEmitterPropsStruct::ParticleEmitterPropsStruct():
	particleLife(0.0),
	particleLifeMargin(0.0),
	forceDirection(0.0),
	forceDirectionMargin(0.0),
	forceMagnitude(0.0),
	forceMagnitudeMargin(0.0),
	particleMass(0.0),
	particleMassMargin(0.0),
	gravity(0.0),
	gravityMargin(0.0),
	startingPos(0.0),
	startingPosMargin(0.0),
	size(0.0)
{}


//======================================================================================================================
// Constructor [copy]                                                                                                  =
//======================================================================================================================
ParticleEmitterPropsStruct::ParticleEmitterPropsStruct(const ParticleEmitterPropsStruct& a)
{
	(*this) = a;
}


//======================================================================================================================
//                                                                                                                     =
//======================================================================================================================
ParticleEmitterPropsStruct& ParticleEmitterPropsStruct::operator=(const ParticleEmitterPropsStruct& a)
{
	memcpy(this, &(const_cast<ParticleEmitterPropsStruct&>(a)), sizeof(ParticleEmitterPropsStruct));
	return *this;
}


//======================================================================================================================
// hasForce                                                                                                            =
//======================================================================================================================
bool ParticleEmitterPropsStruct::hasForce() const
{
	return (!M::isZero(forceDirection.getLengthSquared()) || !M::isZero(forceDirectionMargin.getLengthSquared())) &&
	       (forceMagnitude != 0.0 || forceMagnitudeMargin != 0.0);
}


//======================================================================================================================
// usingWorldGrav                                                                                                      =
//======================================================================================================================
bool ParticleEmitterPropsStruct::usingWorldGrav() const
{
	return M::isZero(gravity.getLengthSquared());
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void ParticleEmitterProps::load(const char* /*filename*/)
{
	// dummy load
	particleLife = 7.0;
	particleLifeMargin = 2.0;

	forceDirection = Vec3(0.0, 0.1, 0.0);
	forceDirectionMargin = Vec3(0.0, 0.0, 0.0);
	forceMagnitude = 900.0;
	forceMagnitudeMargin = 100.0;

	particleMass = 1.0;
	particleMassMargin = 0.0;

	/*gravity = Vec3(0.0, 10.0, 0.0);
	gravityMargin = Vec3(0.0, 0.0, 0.0);*/

	startingPos = Vec3(0.0, 1.0, 0.0);
	startingPosMargin = Vec3(0.0, 0.0, 0.0);
	size = 0.5;
	maxNumOfParticles = 50;
	emittionPeriod = 1.5;
	particlesPerEmittion = 2;

	// sanity checks
	if(particleLife <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "particleLife");
	}

	if(particleLife - particleLifeMargin <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "particleLifeMargin");
	}

	if(size <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "size");
	}

	if(maxNumOfParticles < 1)
	{
		throw PE_EXCEPTION(errMsg + "maxNumOfParticles");
	}

	if(emittionPeriod <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "emittionPeriod");
	}

	if(particlesPerEmittion < 1)
	{
		throw PE_EXCEPTION(errMsg + "particlesPerEmittion");
	}
}
