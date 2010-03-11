#ifndef _PARTICLEEMITTER_H_
#define _PARTICLEEMITTER_H_

#include "Common.h"
#include "Node.h"
#include "MeshNode.h"


/**
 *
 */
class ParticleEmitter: public Node
{
	public:

		class Particle: public MeshNode
		{
			public:
				int lifeTillDeath; ///< Life till death. If 0 then dead. In ms

				Particle() {}
				void render();
				void renderDepth() {};
		};

		// the properties
		int maxParticleLife;
		int minParticleLife;
		Euler maxAngle;
		Euler minAngle;
		float minParticleMass;
		float maxParticleMass;
		int maxNumOfParticles; ///< The size of the particles vector
		int emittionPeriod; ///< How often the emitter emits new particles. In ms


		// the changeable vars
		Vec<Particle*> particles;
		int life;

		// funcs
		ParticleEmitter(): Node( NT_PARTICLE_EMITTER ) {}
		void render() {}
		void renderDepth() {}
		void init( const char* filename );
		void deinit() {}
};

#endif
