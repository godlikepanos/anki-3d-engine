#ifndef ANKI_SCENE_SECTOR_H
#define ANKI_SCENE_SECTOR_H

#include "anki/scene/Octree.h"

namespace anki {

class SceneNode;
class Sector;

/// Portal
class Portal
{
private:
	Array<Sector*, 2> sectors;
};

/// A sector
class Sector
{
public:
	Sector(const Aabb& box)
		: octree(box, 3)
	{}

	const Octree& getOctree() const
	{
		return octree;
	}
	Octree& getOctree()
	{
		return octree;
	}

	bool placeSceneNode(SceneNode* sp);

private:
	Octree octree;
};

} // end namespace anki

#endif
