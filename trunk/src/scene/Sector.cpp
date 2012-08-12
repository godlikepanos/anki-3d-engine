#include "anki/scene/Sector.h"
#include "anki/scene/Spatial.h"
#include "anki/collision/CollisionAlgorithmsMatrix.h"

namespace anki {

//==============================================================================
bool Sector::placeSpatial(Spatial* sp)
{
	// XXX Optimize

	if(!CollisionAlgorithmsMatrix::collide(sp->getAabb(),
		octree.getRoot().getAabb()))
	{
		return false;
	}

	octree.placeSpatial(sp);
	return true;
}

} // end namespace anki
