#ifndef PARTICLEEMITTERPROPS_H
#define PARTICLEEMITTERPROPS_H

#include "Common.h"
#include "Math.h"
#include "Resource.h"


/**
 * This is the properties of the particle emitter resource. Its a separate class from ParticleEmitterProps cause
 * ParticleEmitter SceneNode subclass it.
 */
class ParticleEmitterPropsStruct
{
	public:
		/**
		 * @name Particle properties
		 */
		/**@{*/
		float particleLife; ///< Required
		float particleLifeMargin;

		Vec3 forceDirection; ///< Required
		Vec3 forceDirectionMargin;
		float forceMagnitude; ///< Required
		float forceMagnitudeMargin;

		float particleMass; ///< Required
		float particleMassMargin;

		Vec3 gravity; ///< If not set then it uses the world's default
		Vec3 gravityMargin;

		Vec3 startingPos; ///< If not set the default is zero
		Vec3 startingPosMargin;
		/**@}*/

		/**
		 * @name Emitter properties
		 */
		/**@{*/
		uint maxNumOfParticles; ///< The size of the particles vector. Required
		float emittionPeriod; ///< How often the emitter emits new particles. In ms. Required
		uint particlesPerEmittion; ///< How many particles are emitted every emittion. Required
		/**@}*/

		ParticleEmitterPropsStruct();
		ParticleEmitterPropsStruct(const ParticleEmitterPropsStruct& a);

	protected:
		bool usingWorldGrav; ///< This flag indicates if we should use the default world gravity or to override it
};



/**
 * The actual particle emitter resource
 */
class ParticleEmitterProps: public ParticleEmitterPropsStruct, public Resource
{
	public:
		ParticleEmitterProps();

		bool load(const char* filename);
		void unload();
};


inline ParticleEmitterProps::ParticleEmitterProps():
	Resource(RT_PARTICLE_EMITTER_PROPS)
{}


inline void ParticleEmitterProps::unload()
{}

#endif
