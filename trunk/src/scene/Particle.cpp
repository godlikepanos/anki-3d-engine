#include "Particle.h"
#include "phys/RigidBody.h"


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
void Particle::setNewRigidBody(phys::RigidBody* body_)
{
	body.reset(body_);
}
