#include "anki/scene/Sector.h"
#include "anki/scene/Spatial.h"
#include "anki/collision/CollisionAlgorithmsMatrix.h"

namespace anki {

//==============================================================================
bool Sector::placeSceneNode(SceneNode* sn)
{
	// XXX Optimize

	if(!CollisionAlgorithmsMatrix::collide(sn->getSpatial()->getAabb(),
		octree.getRoot().getAabb()))
	{
		return false;
	}

	octree.placeSceneNode(sn);
	return true;
}

} // end namespace anki
