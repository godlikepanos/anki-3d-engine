#ifndef ANKI_SCENE_OCTREE_H
#define ANKI_SCENE_OCTREE_H

#include "anki/collision/Aabb.h"
#include "anki/util/Vector.h"
#include "anki/util/Array.h"
#include "anki/scene/Common.h"
#include <memory>

namespace anki {

class Spatial;
class Frustumable;
class SceneNode;

/// Octree node
class OctreeNode
{
	friend class Octree;

public:
	typedef Array<OctreeNode*, 8> ChildrenContainer;

	OctreeNode(const Aabb& aabb, OctreeNode* parent,
		const SceneAllocator<U8>& alloc);

	~OctreeNode();

	/// @name Accessors
	/// @{
	const OctreeNode* getChild(U id) const
	{
		return children[id];
	}

	const OctreeNode* getParent() const
	{
		return parent;
	}

	const Aabb& getAabb() const
	{
		return aabb;
	}

	SceneVector<SceneNode*>::iterator getSceneNodesBegin()
	{
		return sceneNodes.begin();
	}
	SceneVector<SceneNode*>::const_iterator getSceneNodesBegin() const
	{
		return sceneNodes.begin();
	}
	SceneVector<SceneNode*>::iterator getSceneNodesEnd()
	{
		return sceneNodes.end();
	}
	SceneVector<SceneNode*>::const_iterator getSceneNodesEnd() const
	{
		return sceneNodes.end();
	}
	/// @}

	Bool isRoot() const
	{
		return parent == nullptr;
	}

	void removeSceneNode(SceneNode* sn);

private:
	ChildrenContainer children;
	OctreeNode* parent;
	Aabb aabb; ///< Including AABB
	SceneVector<SceneNode*> sceneNodes;

	void addSceneNode(SceneNode* sn);

	void addChild(U pos, OctreeNode* child);
};

/// Octree
class Octree
{
public:
	Octree(const SceneAllocator<U8>& alloc, const Aabb& aabb, U8 maxDepth, 
		F32 looseness = 1.5);

	~Octree();

	/// @name Accessors
	/// @{
	const OctreeNode& getRoot() const
	{
		return root;
	}
	OctreeNode& getRoot()
	{
		return root;
	}

	U getMaxDepth() const
	{
		return maxDepth;
	}
	/// @}

	/// Place a spatial in the tree
	void placeSceneNode(SceneNode* sn);

	/// Do the visibility tests
	void getVisible(const Frustumable& fr,
		SceneVector<SceneNode*>* renderableNodes,
		SceneVector<SceneNode*>* lightNodes);

private:
	SceneAllocator<U8> alloc;
	U maxDepth;
	F32 looseness;
	OctreeNode root;

	OctreeNode* placeInternal(const Aabb& aabb, U depth, OctreeNode& node);

	/// Place an AABB inside the tree. The algorithm is pretty simple, find the
	/// node that completely includes the aabb. If found then go deeper into
	/// the node's children. The aabb will not be in more than one nodes. Also,
	/// if it is ourside the tree then return nullptr
	OctreeNode* place(const Aabb& aabb);

	/// Recursive method
	void doVisibilityTestsInternal(const Frustumable& fr,
		SceneVector<SceneNode*>* renderableNodes,
		SceneVector<SceneNode*>* lightNodes,
		OctreeNode& node);

	void createChildren(OctreeNode& parent);

	/// Calculate the AABB of a child given the parent's AABB and its
	/// position
	/// @param[in] i 0: left, 1: right
	/// @param[in] j 0: down, 1: up
	/// @param[in] k 0: back, 1: front
	/// @param[in] parentAabb The parent's AABB
	/// @param[out] out The out AABB
	void calcAabb(U i, U j, U k, const Aabb& parentAabb, Aabb& out) const;
};

} // end namespace anki

#endif
