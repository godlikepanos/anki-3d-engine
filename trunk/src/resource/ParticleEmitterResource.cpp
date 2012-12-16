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
	particle.life = 7.0;
	particle.lifeDeviation = 2.0;

	particle.size = 1.0;
	particle.sizeAnimation = 2.0;

	particle.forceDirection = Vec3(0.0, 1.0, 0.0);
	particle.forceDirectionDeviation = Vec3(0.2);
	particle.forceMagnitude = 100.0;
	particle.forceMagnitudeDeviation = 0.0;

	particle.mass = 1.0;
	particle.massDeviation = 0.0;

	particle.gravity = Vec3(0.0, 1.0, 0.0);
	particle.gravityDeviation = Vec3(0.0, 0.0, 0.0);

	particle.alpha = 0.25;

	/*gravity = Vec3(0.0, 10.0, 0.0);
	gravityDeviation = Vec3(0.0, 0.0, 0.0);*/

	particle.startingPos = Vec3(0.0, -0.5, 0.0);
	particle.startingPosDeviation = Vec3(0.3, 0.0, 0.3);

	maxNumOfParticles = 16;
	emissionPeriod = 0.5;
	particlesPerEmittion = 1;
	model.load("data/particles/fire/fire.mdl");

	// sanity checks
	//

	if(particle.life <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "life");
	}

	if(particle.life - particle.lifeDeviation <= 0.0)
	{
		throw PE_EXCEPTION(errMsg + "lifeDeviation");
	}

	if(particle.size <= 0.0)
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

	forceEnabled = !isZero(particle.forceDirection.getLengthSquared());
	forceEnabled = forceEnabled
		|| !isZero(particle.forceDirectionDeviation.getLengthSquared());
	forceEnabled = forceEnabled
		&& (particle.forceMagnitude != 0.0
		|| particle.forceMagnitudeDeviation != 0.0);

	wordGravityEnabled = isZero(particle.gravity.getLengthSquared());
}

} // end namespace anki
