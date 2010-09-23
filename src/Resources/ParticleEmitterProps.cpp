#include <cstring>
#include "ParticleEmitterProps.h"


static const char* errMsg = "Incorrect or missing value ";


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
bool ParticleEmitterProps::load(const char* /*filename*/)
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
		ERROR(errMsg << "particleLife");
		return false;
	}

	if(particleLife - particleLifeMargin <= 0.0)
	{
		ERROR(errMsg << "particleLifeMargin");
		return false;
	}

	if(size <= 0.0)
	{
		ERROR(errMsg << "size");
		return false;
	}

	if(maxNumOfParticles < 1)
	{
		ERROR(errMsg << "maxNumOfParticles");
		return false;
	}

	if(emittionPeriod <= 0.0)
	{
		ERROR(errMsg << "emittionPeriod");
		return false;
	}

	if(particlesPerEmittion < 1)
	{
		ERROR(errMsg << "particlesPerEmittion");
		return false;
	}

	return true;
}
