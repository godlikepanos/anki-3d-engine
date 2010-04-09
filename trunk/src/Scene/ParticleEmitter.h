#ifndef _PARTICLEEMITTER_H_
#define _PARTICLEEMITTER_H_

#include "Common.h"
#include "Node.h"
#include "MeshNode.h"
#include "GhostNode.h"
#include "PhyCommon.h"


/**
 * @brief The particle emitter scene node
 *
 * This scene node emitts @ref ParticleEmitter:Particle particle nodes in space.
 */
class ParticleEmitter: public Node
{
	public:

		/**
		 * @brief The scene node particle class
		 */
		class Particle: public GhostNode
		{
			public:
				int lifeTillDeath; ///< Life till death. If < 0 then dead. In ms
				btRigidBody* body;

				Particle(): lifeTillDeath(-1) {}
				void render();
				void renderDepth() {};
		};

		// the particle properties
		uint minParticleLife;
		uint maxParticleLife;
		Vec3 minDirection;
		Vec3 maxDirection;
		float minForceMagnitude;
		float maxForceMagnitude;
		float minParticleMass;
		float maxParticleMass;
		Vec3 minGravity;
		Vec3 maxGravity;
		Vec3 minStartingPos;
		Vec3 maxStartingPos;

		// the emittion properties
		uint maxNumOfParticles; ///< The size of the particles vector
		uint emittionPeriod; ///< How often the emitter emits new particles. In ms
		uint particlesPerEmittion; ///< How many particles are emitted every emittion


		// the changeable vars
		Vec<Particle*> particles;
		uint timeOfPrevUpdate;
		uint timeOfPrevEmittion;

		// funcs
		ParticleEmitter(): Node( NT_PARTICLE_EMITTER ) {}
		void render();
		void renderDepth() {}
		void init( const char* filename );
		void deinit() {}
		void update();
};

#endif
