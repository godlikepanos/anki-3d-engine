#ifndef ANKI_SCENE_SECTOR_H
#define ANKI_SCENE_SECTOR_H

#include "anki/scene/Octree.h"
#include "anki/collision/Collision.h"

namespace anki {

// Forward
class SceneNode;
class Scene;
class Sector;

/// @addtogroup Scene
/// @{

/// 2 way Portal
struct Portal
{
	Array<Sector*, 2> sectors;
	Obb shape;

	Portal();
};

/// A sector. It consists of an octree and some portals
struct Sector
{
	Octree octree;
	SceneVector<Portal*> portals;

	/// Default constructor
	Sector(const SceneAllocator<U8>& alloc, const Aabb& box);

	/// Called when a node was moved or a change in shape happened
	Bool placeSceneNode(SceneNode* sp);

	const Aabb& getAabb() const
	{
		return octree.getRoot().getAabb();
	}
};

/// Sector group. This is supposed to represent the whole scene
class SectorGroup
{
public:
	/// Default constructor
	SectorGroup(Scene* scene);

	/// Destructor
	~SectorGroup();

	/// Called when a node was moved or a change in shape happened. The node 
	/// must be Spatial
	void placeSceneNode(SceneNode* sp);

private:
	Scene* scene; ///< Keep it here to access various allocators
	SceneVector<Sector*> sectors;
	SceneVector<Portal*> portals;
};

/// @}

} // end namespace anki

#endif
