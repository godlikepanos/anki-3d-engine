#include "anki/physics/PhysicsObject.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
SceneAllocator<U8> PhysicsObject::getSceneAllocator() const
{
	return world->getSceneAllocator();
}

} // end namespace anki

