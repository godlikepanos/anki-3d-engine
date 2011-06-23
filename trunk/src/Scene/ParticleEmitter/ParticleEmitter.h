#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <btBulletCollisionCommon.h>
#include "SceneNode.h"
#include "GhostNode.h"
#include "ParticleEmitterProps.h"
#include "RsrcPtr.h"
#include "PhysRigidBody.h"


class btCollisionShape;


/// The particle emitter scene node. This scene node emitts @ref ParticleEmitter:Particle particle nodes in space.
class ParticleEmitter: public SceneNode, public ParticleEmitterPropsStruct
{
	public:
		/// The scene node particle class
		class Particle: public GhostNode
		{
			public:
				float timeOfDeath; ///< Life of death. If < 0.0 then dead. In seconds
				boost::scoped_ptr<Phys::RigidBody> body;

				Particle();
				void render();
		};

	public:
		ParticleEmitter();
		void init(const char* filename);

	private:
		std::auto_ptr<btCollisionShape> collShape;
		boost::ptr_vector<Particle> particles;
		float timeOfPrevUpdate;
		float timeOfPrevEmittion;
		RsrcPtr<ParticleEmitterProps> particleEmitterProps; ///< The resource
		static btTransform startingTrf;

		void frameUpdate();
		void moveUpdate() {}
};


inline ParticleEmitter::Particle::Particle():
	timeOfDeath(-1.0)
{}


inline ParticleEmitter::ParticleEmitter():
	SceneNode(SNT_PARTICLE_EMITTER, false, NULL)
{}


#endif
