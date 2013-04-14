#ifndef ANKI_SCENE_GRID_H
#define ANKI_SCENE_GRID_H

#include "anki/Collision.h"
#include "anki/scene/Common.h"

namespace anki {

class SceneNode;
class Frustumable;

/// Space partitioning container
class Grid
{
public:
	/// Default constructor
	Grid(const SceneAllocator<U8>& alloc, F32 cubeSize, const Aabb& aabb);

	/// @name Accessors
	/// @{
	const Aabb& getAabb() const
	{
		return aabb;
	}

	SceneVector<SceneNode*>::iterator getSceneNodesBegin()
	{
		return sceneNodes.begin();
	}
	SceneVector<SceneNode*>::iterator getSceneNodesEnd()
	{
		return sceneNodes.end();
	}
	/// @}

	/// Place a scene node in the grid
	Bool placeSceneNode(SceneNode* sn);

	/// XXX
	void getVisible(const Frustumable& cam, 
		SceneVector<SceneNode*>& nodes);

private:
	Aabb aabb;
	Array<U32, 3> cubesCount; ///< Cubes count in the 3 dimentions
	F32 cubeSize;
	SceneVector<SceneNode*> sceneNodes;

	void removeSceneNode(SceneNode* sn);
};

} // end namespace anki

#endif
