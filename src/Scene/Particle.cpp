#include "Particle.h"
#include "Physics/RigidBody.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Particle::Particle(float timeOfDeath_, SceneNode* parent):
	ModelNode(false, parent),
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
void Particle::setNewRigidBody(Phys::RigidBody* body_)
{
	body.reset(body_);
}
