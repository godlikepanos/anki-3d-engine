#ifndef ANKI_SCENE_OCTREE_H
#define ANKI_SCENE_OCTREE_H

#include "anki/collision/Aabb.h"
#include "anki/util/Vector.h"
#include "anki/util/Array.h"
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
	typedef Array<std::unique_ptr<OctreeNode>, 8> ChildrenContainer;

	OctreeNode(const Aabb& aabb_, OctreeNode* parent_)
		: parent(parent_), aabb(aabb_)
	{}

	/// @name Accessors
	/// @{
	const OctreeNode* getChild(U id) const
	{
		return children[id].get();
	}

	const OctreeNode* getParent() const
	{
		return parent;
	}

	const Aabb& getAabb() const
	{
		return aabb;
	}

	Vector<SceneNode*>::iterator getSceneNodesBegin()
	{
		return sceneNodes.begin();
	}
	Vector<SceneNode*>::iterator getSceneNodesEnd()
	{
		return sceneNodes.end();
	}
	U getSceneNodesCount() const
	{
		return sceneNodes.size();
	}
	/// @}

	Bool isRoot() const
	{
		return parent == nullptr;
	}

	void addChild(U pos, OctreeNode* child)
	{
		child->parent = this;
		children[pos].reset(child);
	}

	void addSceneNode(SceneNode* sn)
	{
		ANKI_ASSERT(sn != nullptr);
		sceneNodes.push_back(sn);
	}

	void removeSceneNode(SceneNode* sn);

private:
	ChildrenContainer children;
	OctreeNode* parent;
	Aabb aabb; ///< Including AABB
	Vector<SceneNode*> sceneNodes;
};

/// Octree
class Octree
{
public:
	Octree(const Aabb& aabb, U8 maxDepth, F32 looseness = 1.5);

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

	/// Fill the fr with visible data
	void doVisibilityTests(Frustumable& fr);

private:
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
	void doVisibilityTestsRec(Frustumable& fr, OctreeNode& node);

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
