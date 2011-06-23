#ifndef PARTICLE_H
#define PARTICLE_H

#include "ModelNode.h"


namespace Phys {
class RigidBody;
}


/// The scene node particle class
class Particle: public ModelNode
{
	public:
		Particle(uint timeOfDeath_): timeOfDeath(timeOfDeath_) {}
		~Particle();

		GETTER_SETTER(uint, timeOfDeath, getTimeOfDeath, setTimeOfDeath)
		void setNewRigidBody(Phys::RigidBody* body_) {body.reset(body_);}

	private:
		uint timeOfDeath; ///< Life of death. If < 0.0 then dead. In seconds
		boost::scoped_ptr<Phys::RigidBody> body;
};


#endif
