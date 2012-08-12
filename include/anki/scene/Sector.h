#ifndef ANKI_SCENE_SECTOR_H
#define ANKI_SCENE_SECTOR_H

#include "anki/scene/Octree.h"

namespace anki {

class Spatial;

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

	bool placeSpatial(Spatial* sp);

private:
	Octree octree;
};

} // end namespace anki

#endif
