#ifndef PARTICLE_H
#define PARTICLE_H

#include <boost/scoped_ptr.hpp>
#include "ModelNode.h"


namespace phys {
class RigidBody;
}


/// The scene node particle class
class Particle: public ModelNode
{
	public:
		Particle(float timeOfDeath_, SceneNode* parent);
		virtual ~Particle();

		GETTER_SETTER(float, timeOfDeath, getTimeOfDeath, setTimeOfDeath)
		bool isDead() const {return timeOfDeath < 0.0;}
		void setNewRigidBody(phys::RigidBody* body_);
		phys::RigidBody& getRigidBody() {return *body;}
		const phys::RigidBody& getRigidBody() const {return *body;}

	private:
		float timeOfDeath; ///< Life of death. If < 0.0 then dead. In seconds
		boost::scoped_ptr<phys::RigidBody> body; ///< For garbage collection
};


#endif
