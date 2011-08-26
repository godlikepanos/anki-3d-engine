#ifndef PARTICLE_EMITTER_NODE_H
#define PARTICLE_EMITTER_NODE_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <btBulletCollisionCommon.h>
#include "SceneNode.h"
#include "rsrc/ParticleEmitterRsrc.h"
#include "rsrc/RsrcPtr.h"


class btCollisionShape;
class Particle;


/// The particle emitter scene node. This scene node emitts
/// @ref ParticleEmitter:Particle particle nodes in space.
class ParticleEmitterNode: public SceneNode, public ParticleEmitterRsrc
{
	public:
		ParticleEmitterNode(bool inheritParentTrfFlag, SceneNode* parent);
		virtual ~ParticleEmitterNode();

		void init(const char* filename);

		void frameUpdate(float prevUpdateTime, float crntTime);

	private:
		boost::scoped_ptr<btCollisionShape> collShape;
		Vec<Particle*> particles;
		float timeLeftForNextEmission;
		RsrcPtr<ParticleEmitterRsrc> particleEmitterProps; ///< The resource
		static btTransform startingTrf;

		static float getRandom(float initial, float deviation);
		static Vec3 getRandom(const Vec3& initial, const Vec3& deviation);
};


inline ParticleEmitterNode::ParticleEmitterNode(bool inheritParentTrfFlag,
	SceneNode* parent)
:	SceneNode(SNT_PARTICLE_EMITTER, inheritParentTrfFlag, parent)
{}


#endif
