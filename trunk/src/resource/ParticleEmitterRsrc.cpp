#include <cstring>
#include "ParticleEmitterRsrc.h"
#include "util/Exception.h"


static const char* errMsg = "Incorrect or missing value ";


#define PE_EXCEPTION(x) EXCEPTION("Particle emmiter: " + x)


//==============================================================================
// Constructor                                                                 =
//==============================================================================
ParticleEmitterRsrc::ParticleEmitterRsrc():
	particleLife(0.0),
	particleLifeDeviation(0.0),
	forceDirection(0.0),
	forceDirectionDeviation(0.0),
	forceMagnitude(0.0),
	forceMagnitudeDeviation(0.0),
	particleMass(0.0),
	particleMassDeviation(0.0),
	gravity(0.0),
	gravityDeviation(0.0),
	startingPos(0.0),
	startingPosDeviation(0.0),
	size(0.0)
{}


//==============================================================================
// Constructor [copy]                                                          =
//==============================================================================
ParticleEmitterRsrc::ParticleEmitterRsrc(const ParticleEmitterRsrc& a)
{
	(*this) = a;
}


//==============================================================================
// operator=                                                                   =
//==============================================================================
ParticleEmitterRsrc& ParticleEmitterRsrc::operator=(
	const ParticleEmitterRsrc& b)
{
	particleLife = b.particleLife;
	particleLifeDeviation = b.particleLifeDeviation;
	forceDirection = b.forceDirection;
	forceDirectionDeviation = b.forceDirectionDeviation;
	forceMagnitude = b.forceMagnitude;
	forceMagnitudeDeviation = b.forceMagnitudeDeviation;
	particleMass = b.particleMass;
	particleMassDeviation = b.particleMassDeviation;
	gravity = b.gravity;
	gravityDeviation = b.gravityDeviation;
	startingPos = b.startingPos;
	startingPosDeviation = b.startingPosDeviation;
	size = b.size;
	maxNumOfParticles = b.maxNumOfParticles;
	emissionPeriod = b.emissionPeriod;
	particlesPerEmittion = b.particlesPerEmittion;
	modelName = b.modelName;

	return *this;
}


//==============================================================================
// hasForce                                                                    =
//==============================================================================
bool ParticleEmitterRsrc::hasForce() const
{
	return (!Math::isZero(forceDirection.getLengthSquared()) ||
		!Math::isZero(forceDirectionDeviation.getLengthSquared())) &&
			(forceMagnitude != 0.0 || forceMagnitudeDeviation != 0.0);
}


//==============================================================================
// usingWorldGrav                                                              =
//==============================================================================
bool ParticleEmitterRsrc::usingWorldGrav() const
{
	return Math::isZero(gravity.getLengthSquared());
}


//==============================================================================
// load                                                                        =
//==============================================================================
void ParticleEmitterRsrc::load(const char* /*filename*/)
{
	// dummy load
	particleLife = 7.0;
	particleLifeDeviation = 2.0;

	forceDirection = Vec3(0.0, 0.1, 0.0);
	forceDirectionDeviation = Vec3(0.0, 0.0, 0.0);
	forceMagnitude = 900.0;
	forceMagnitudeDeviation = 100.0;

	particleMass = 1.0;
	particleMassDeviation = 0.0;

	/*gravity = Vec3(0.0, 10.0, 0.0);
	gravityDeviation = Vec3(0.0, 0.0, 0.0);*/

	startingPos = Vec3(0.0, 1.0, 0.0);
	startingPosDeviation = Vec3(0.0, 0.0, 0.0);
	size = 0.5;
	maxNumOfParticles = 50;
	emissionPeriod = 1.0;
	particlesPerEmittion = 1;
	modelName = "meshes/horse/horse.mdl";

	// sanity checks
	if(particleLife <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "particleLife");
	}

	if(particleLife - particleLifeDeviation <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "particleLifeDeviation");
	}

	if(size <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "size");
	}

	if(maxNumOfParticles < 1)
	{
		throw PE_EXCEPTION(errMsg + "maxNumOfParticles");
	}

	if(emissionPeriod <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "emissionPeriod");
	}

	if(particlesPerEmittion < 1)
	{
		throw PE_EXCEPTION(errMsg + "particlesPerEmittion");
	}
}
