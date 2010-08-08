#include <cstring>
#include "ParticleEmitterProps.h"


static const char* errMsg = "Incorrect value ";


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
	usingWorldGrav(true),
	hasForce(false)
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
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool ParticleEmitterProps::load(const char* filename)
{
	// dummy load
	particleLife = 7.0;
	particleLifeMargin = 2.0;

	forceDirection = Vec3(0.0, 0.000001, 0.0);
	forceDirectionMargin = Vec3(1.0, 0.0, 1.0);
	forceMagnitude = 200.0;
	forceMagnitudeMargin = 100.0;

	particleMass = 1.0;
	particleMassMargin = 0.0;

	usingWorldGrav = false;
	gravity = Vec3(0.0, 10.0, 0.0);
	gravityMargin = Vec3(0.0, 0.0, 0.0);

	startingPos = Vec3(0.0, 1.0, 0.0);
	startingPosMargin = Vec3(0.0, 0.0, 0.0);
	maxNumOfParticles = 50;
	emittionPeriod = 0.05;
	particlesPerEmittion = 2;

	// set some values
	if((M::isZero(forceDirection.getLength()) && M::isZero(forceDirectionMargin.getLength())) ||
	   (forceMagnitude == 0.0 && forceMagnitudeMargin == 0.0))
	{
		hasForce = false;
	}
	else
		hasForce = true;

	usingWorldGrav = M::isZero(gravity.getLength()) ? true : false;


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


	return true;
}
