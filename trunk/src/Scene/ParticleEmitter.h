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
				int life; ///< Life in ms

				void render();
				void renderDepth() {};
		};

		Vec<Particle*> particles;
		int maxParticlesLife;
		int minParticlesLife;
		Euler maxAngle;
		Euler minAngle;
		int maxNumOfParticlesInScreen;

		ParticleEmitter(): Node( NT_PARTICLE_EMITTER ) {}
		void render() {}
		void renderDepth() {}
		void init( const char* filename );
		void deinit() {}
};

#endif
