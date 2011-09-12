#include "Particle.h"
#include "phys/RigidBody.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Particle::Particle(float timeOfDeath_, Scene& scene, ulong flags,
	SceneNode* parent):
	ModelNode(CID_PARTICLE, scene, flags, parent),
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
