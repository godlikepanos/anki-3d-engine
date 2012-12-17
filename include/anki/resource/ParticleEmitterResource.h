#ifndef ANKI_RESOURCE_PARTICLE_EMITTER_RSRC_H
#define ANKI_RESOURCE_PARTICLE_EMITTER_RSRC_H

#include "anki/math/Math.h"
#include "anki/resource/Resource.h"

namespace anki {

class XmlElement;

/// The particle emitter properties. Different class from
/// ParticleEmitterResource so it can be inherited
struct ParticleEmitterProperties
{
public:
	ParticleEmitterProperties& operator=(const ParticleEmitterProperties& b);

protected:
	/// @name Particle specific properties
	/// @{
	struct
	{
		/// Particle life
		F32 life = 10.0;
		F32 lifeDeviation = 0.0;

		/// Particle mass
		F32 mass = 1.0;
		F32 massDeviation = 0.0;

		/// Particle size. It is the size of the collision shape
		F32 size = 1.0;
		F32 sizeAnimation = 1.0;

		/// Alpha factor. If the material supports alpha then multiply with 
		/// this
		F32 alpha = 1.0;

		/// Initial force. If not set only the gravity applies
		Vec3 forceDirection = Vec3(0.0, 1.0, 0.0);
		Vec3 forceDirectionDeviation = Vec3(0.0);
		F32 forceMagnitude = 0.0; ///< Default 0.0
		F32 forceMagnitudeDeviation = 0.0;

		/// If not set then it uses the world's default
		Vec3 gravity = Vec3(0.0);
		Vec3 gravityDeviation = Vec3(0.0);

		/// This position is relevant to the particle emitter pos
		Vec3 startingPos = Vec3(0.0);
		Vec3 startingPosDeviation = Vec3(0.0);
	} particle;
	/// @}

	/// @name Emitter specific properties
	/// @{

	/// The size of the particles vector. Required
	U32 maxNumOfParticles = 16;
	/// How often the emitter emits new particles. In secs. Required
	F32 emissionPeriod = 1.0;
	/// How many particles are emitted every emission. Required
	U32 particlesPerEmittion = 1;
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

	void loadInternal(const XmlElement& el);
};

} // end namespace anki

#endif
