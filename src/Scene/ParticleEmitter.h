#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <btBulletCollisionCommon.h>
#include "SceneNode.h"
#include "ParticleEmitterRsrc.h"
#include "Resources/RsrcPtr.h"


class btCollisionShape;
class Particle;


/// The particle emitter scene node. This scene node emitts @ref ParticleEmitter:Particle particle nodes in space.
class ParticleEmitter: public SceneNode, public ParticleEmitterRsrc
{
	public:
		ParticleEmitter();
		virtual ~ParticleEmitter();

		void init(const char* filename);

		void frameUpdate(float prevUpdateTime, float crntTime);
		void moveUpdate() {}

	private:
		boost::scoped_ptr<btCollisionShape> collShape;
		Vec<Particle*> particles;
		float timeLeftForNextEmission;
		RsrcPtr<ParticleEmitterRsrc> particleEmitterProps; ///< The resource
		static btTransform startingTrf;

		static float getRandom(float initial, float deviation);
		static Vec3 getRandom(const Vec3& initial, const Vec3& deviation);
};


inline ParticleEmitter::ParticleEmitter():
	SceneNode(SNT_PARTICLE_EMITTER, false, NULL)
{}


#endif
