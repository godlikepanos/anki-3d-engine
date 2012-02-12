#ifndef ANKI_SCENE_PARTICLE_EMITTER_NODE_H
#define ANKI_SCENE_PARTICLE_EMITTER_NODE_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <btBulletCollisionCommon.h>
#include "anki/scene/SceneNode.h"
#include "anki/resource/ParticleEmitterRsrc.h"
#include "anki/resource/Resource.h"


class btCollisionShape;


namespace anki {

/*class Particle;


/// The particle emitter scene node. This scene node emitts
/// @ref ParticleEmitter:Particle particle nodes in space.
class ParticleEmitterNode: public SceneNode, public ParticleEmitterRsrc
{
	public:
		ParticleEmitterNode(Scene& scene, ulong flags, SceneNode* parent);
		virtual ~ParticleEmitterNode();

		void init(const char* filename);

		void frameUpdate(float prevUpdateTime, float crntTime);

	private:
		boost::scoped_ptr<btCollisionShape> collShape;
		std::vector<Particle*> particles;
		float timeLeftForNextEmission;
		/// The resource
		ParticleEmitterRsrcResourcePointer particleEmitterProps;
		static btTransform startingTrf;

		static float getRandom(float initial, float deviation);
		static Vec3 getRandom(const Vec3& initial, const Vec3& deviation);
};


inline ParticleEmitterNode::ParticleEmitterNode(Scene& scene, ulong flags,
	SceneNode* parent)
:	SceneNode(SNT_PARTICLE_EMITTER_NODE, scene, flags, parent)
{}*/


} // end namespace


#endif
