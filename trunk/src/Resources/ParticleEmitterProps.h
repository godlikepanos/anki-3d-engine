#ifndef PARTICLE_EMITTER_PROPS_H
#define PARTICLE_EMITTER_PROPS_H

#include "Math.h"


/// This is the properties of the particle emitter resource. Its a separate class from ParticleEmitterProps cause
/// ParticleEmitter SceneNode subclass it.
class ParticleEmitterPropsStruct
{
	public:
		/// @name Particle specific properties
		/// @{
		float particleLife; ///< Required and > 0.0
		float particleLifeMargin;

		Vec3 forceDirection; ///< Not-required, any value, Default 0.0, If not set only the gravity applies
		Vec3 forceDirectionMargin;
		float forceMagnitude; ///< Default 0.0
		float forceMagnitudeMargin;

		float particleMass; ///< Required and > 0.0
		float particleMassMargin;

		Vec3 gravity; ///< Not-required, any value. If not set then it uses the world's default
		Vec3 gravityMargin;

		Vec3 startingPos; ///< If not set the default is zero
		Vec3 startingPosMargin;

		float size; ///< The size of the collision shape. Required and > 0.0
		/// @}

		/// @name Emitter specific properties
		/// @{
		uint maxNumOfParticles; ///< The size of the particles vector. Required
		float emittionPeriod; ///< How often the emitter emits new particles. In secs. Required
		uint particlesPerEmittion; ///< How many particles are emitted every emittion. Required
		/// @}

		ParticleEmitterPropsStruct();
		ParticleEmitterPropsStruct(const ParticleEmitterPropsStruct& a);
		ParticleEmitterPropsStruct& operator=(const ParticleEmitterPropsStruct& a);
		~ParticleEmitterPropsStruct() {}
		bool hasForce() const; ///< Dont call it offten. Its slow
		bool usingWorldGrav() const; ///< @return True if gravity is derived
};


/// The actual particle emitter resource
class ParticleEmitterProps: public ParticleEmitterPropsStruct
{
	public:
		void load(const char* filename);
};


#endif
