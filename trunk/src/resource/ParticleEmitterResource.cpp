#include "anki/resource/ParticleEmitterResource.h"
#include "anki/resource/Model.h"
#include "anki/util/Exception.h"
#include <cstring>

namespace anki {

static const char* errMsg = "Incorrect or missing value ";

#define PE_EXCEPTION(x) ANKI_EXCEPTION("Particle emmiter: " + x)

//==============================================================================
// ParticleEmitterProperties                                                   =
//==============================================================================

//==============================================================================
ParticleEmitterProperties& ParticleEmitterProperties::operator=(
	const ParticleEmitterProperties& b)
{
	memcpy(this, &b, sizeof(ParticleEmitterProperties));
	return *this;
}

//==============================================================================
// ParticleEmitterResource                                                     =
//==============================================================================

//==============================================================================
ParticleEmitterResource::ParticleEmitterResource()
{}

//==============================================================================
ParticleEmitterResource::~ParticleEmitterResource()
{}

//==============================================================================
void ParticleEmitterResource::load(const char* /*filename*/)
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
	maxNumOfParticles = 16;
	emissionPeriod = 1.0;
	particlesPerEmittion = 1;
	model.load("data/particles/fire/fire.mdl");

	// sanity checks
	//

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

	// Calc some stuff
	//

	forceEnabled =
		(!isZero(forceDirection.getLengthSquared())
		|| !isZero(forceDirectionDeviation.getLengthSquared()))
		&& (forceMagnitude != 0.0 || forceMagnitudeDeviation != 0.0);

	wordGravityEnabled = isZero(gravity.getLengthSquared());
}

} // end namespace anki
