#include "anki/scene/Spatial.h"
#include "anki/scene/Octree.h"

namespace anki {

//==============================================================================
Spatial::~Spatial()
{
	if(octreeNode)
	{
		octreeNode->removeSceneNode(sceneNode);
	}
}

} // end namespace anki
