#ifndef ANKI_SCENE_SECTOR_H
#define ANKI_SCENE_SECTOR_H

#include "anki/scene/Octree.h"

namespace anki {

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

private:
	Octree octree;
};

} // end namespace anki

#endif
