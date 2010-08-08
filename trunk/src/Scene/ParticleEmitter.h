#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "Common.h"
#include "SceneNode.h"
#include "MeshNode.h"
#include "GhostNode.h"
#include "PhyCommon.h"
#include "ParticleEmitterProps.h"


/**
 * The particle emitter scene node. This scene node emitts @ref ParticleEmitter:Particle particle nodes in space.
 */
class ParticleEmitter: public SceneNode, public ParticleEmitterPropsStruct
{
	public:
		/**
		 * The scene node particle class
		 */
		class Particle: public GhostNode
		{
			public:
				float timeOfDeath; ///< Life of death. If < 0.0 then dead. In seconds
				btRigidBody* body;

				Particle();
				void render();
		};

		// the changeable vars
		ptr_vector<Particle> particles;
		float timeOfPrevUpdate;
		float timeOfPrevEmittion;
		RsrcPtr<ParticleEmitterProps> particleEmitterProps;

		// funcs
		ParticleEmitter();
		void render();
		void init(const char* filename);
		void deinit() {}
		void update();
};


inline ParticleEmitter::Particle::Particle():
	timeOfDeath(-1.0)
{}


inline ParticleEmitter::ParticleEmitter():
	SceneNode(NT_PARTICLE_EMITTER)
{}


#endif
