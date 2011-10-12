#include "anki/scene/Particle.h"
#include "anki/physics/RigidBody.h"


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Particle::Particle(float timeOfDeath_, Scene& scene, ulong flags,
	SceneNode* parent):
	ModelNode(scene, flags, parent),
	timeOfDeath(timeOfDeath_)
{}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Particle::~Particle()
{}


//==============================================================================
// setNewRigidBody                                                             =
//==============================================================================
void Particle::setNewRigidBody(RigidBody* body_)
{
	body.reset(body_);
}


} // end namespace
