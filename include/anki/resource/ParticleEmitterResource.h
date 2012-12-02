#ifndef ANKI_RESOURCE_PARTICLE_EMITTER_RSRC_H
#define ANKI_RESOURCE_PARTICLE_EMITTER_RSRC_H

#include "anki/math/Math.h"
#include "anki/resource/Resource.h"

namespace anki {

/// XXX
struct ParticleEmitterProperties
{
public:
	ParticleEmitterProperties& operator=(const ParticleEmitterProperties& b);

protected:
	/// @name Particle specific properties
	/// @{
	F32 particleLife = 0.0; ///< Required and > 0.0. In seconds
	F32 particleLifeDeviation = 0.0;

	/// Not-required, any value, Default 0.0, If not set only the gravity
	/// applies
	Vec3 forceDirection = Vec3(0.0, 1.0, 0.0);
	Vec3 forceDirectionDeviation = Vec3(0.0);
	F32 forceMagnitude = 0; ///< Default 0.0
	F32 forceMagnitudeDeviation = 0.0;

	F32 particleMass = 1.0; ///< Required and > 0.0
	F32 particleMassDeviation = 0.0;

	/// Not-required, any value. If not set then it uses the world's default
	Vec3 gravity = Vec3(0.0);
	Vec3 gravityDeviation = Vec3(0.0);

	Vec3 startingPos = Vec3(0.0); ///< If not set the default is zero
	Vec3 startingPosDeviation = Vec3(0.0);

	F32 size = 0.0; ///< The size of the collision shape. Required and > 0.0
	/// @}

	/// @name Emitter specific properties
	/// @{

	/// The size of the particles vector. Required
	U32 maxNumOfParticles;
	/// How often the emitter emits new particles. In secs. Required
	F32 emissionPeriod;
	/// How many particles are emitted every emission. Required
	U32 particlesPerEmittion;
	/// @}
};

/// This is the properties of the particle emitter resource
class ParticleEmitterResource: public ParticleEmitterProperties
{
public:
	ParticleEmitterResource();
	~ParticleEmitterResource();

	Bool hasForce() const
	{
		return forceEnabled;
	}

	///< @return True if gravity is derived
	Bool usingWorldGravity() const
	{
		return wordGravityEnabled;
	}

	const Model& getModel() const
	{
		return *model;
	}

	/// Load it
	void load(const char* filename);

private:
	ModelResourcePointer model;
	Bool forceEnabled;
	Bool wordGravityEnabled;
};

} // end namespace anki

#endif
