#include "anki/scene/Grid.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
Grid::Grid(const SceneAllocator<U8> &alloc, F32 cubeSize_, const Aabb& aabb_)
	: cubeSize(cubeSize_), sceneNodes(alloc)
{
	// Set the aabb by snaping it
	for(U i = 0; i < 3; i++)
	{
		F32 d = aabb_.getMax()[i] - aabb_.getMin()[i];
		ANKI_ASSERT(d > cubeSize);
		F32 snap = ceil(d / cubeSize);

		cubesCount[i] = snap;

		snap *= cubeSize;
		snap -= d;
		snap *= 0.5;

		aabb.getMin()[i] = aabb_.getMin()[i] - snap;
		aabb.getMax()[i] = aabb_.getMax()[i] + snap;
	}
}

//==============================================================================
void Grid::removeSceneNode(SceneNode* sn)
{
	SceneVector<SceneNode*>::iterator it;

	it = std::find(sceneNodes.begin(), sceneNodes.end(), sn);

	ANKI_ASSERT(it != sceneNodes.end());
	sceneNodes.erase(it);

	//(*it)->getSpatial()->grid = nullptr;
}

//==============================================================================
Bool Grid::placeSceneNode(SceneNode* sn)
{
	/*Spatial* sp = sn->getSpatial();
	ANKI_ASSERT(sp);

	if(sp->grid == this)
	{
		// do nothing
	}
	else
	{
		sp->grid->removeSceneNode(sn);
		sp->grid = this;
		sceneNodes.push_back(sn);
	}*/

	return true;
}

//==============================================================================
void Grid::getVisible(const Frustumable& cam,
	SceneVector<SceneNode*>& nodes)
{
	nodes.insert(nodes.end(), sceneNodes.begin(), sceneNodes.end());
}

} // end namespace anki
