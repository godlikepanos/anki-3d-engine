#ifndef ANKI_SCENE_OCTREE_H
#define ANKI_SCENE_OCTREE_H

#include "anki/collision/Aabb.h"
#include "anki/util/Vector.h"
#include <array>
#include <memory>

namespace anki {

class Spatial;

/// Octree node
class OctreeNode
{
	friend class Octree;

public:
	typedef std::array<std::unique_ptr<OctreeNode>, 8> ChildrenContainer;

	OctreeNode(const Aabb& aabb_, OctreeNode* parent_)
		: parent(parent_), aabb(aabb_)
	{}

	/// @name Accessors
	/// @{
	const OctreeNode* getChild(uint32_t id) const
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

	Vector<Spatial*>::iterator getSpatialsBegin()
	{
		return spatials.begin();
	}
	Vector<Spatial*>::iterator getSpatialsEnd()
	{
		return spatials.end();
	}
	uint32_t getSpatialsCount() const
	{
		return spatials.size();
	}
	/// @}

	bool isRoot() const
	{
		return parent == nullptr;
	}

	void addChild(uint32_t pos, OctreeNode* child)
	{
		child->parent = this;
		children[pos].reset(child);
	}

private:
	ChildrenContainer children;
	OctreeNode* parent;
	Aabb aabb; ///< Including AABB
	Vector<Spatial*> spatials;
};

/// Octree
class Octree
{
public:
	Octree(const Aabb& aabb, uint8_t maxDepth, float looseness = 1.5);

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

	uint32_t getMaxDepth() const
	{
		return maxDepth;
	}
	/// @}

	void placeSpatial(Spatial* sp);

	/// Place an AABB inside the tree. The algorithm is pretty simple, find the
	/// node that completely includes the aabb. If found then go deeper into
	/// the node's children. The aabb will not be in more than one nodes. Also,
	/// if it is ourside the tree then return nullptr
	OctreeNode* place(const Aabb& aabb);

private:
	uint32_t maxDepth;
	float looseness;
	OctreeNode root;

	OctreeNode* place(const Aabb& aabb, uint depth, OctreeNode& node);

	void createChildren(OctreeNode& parent);

	/// Calculate the AABB of a child given the parent's AABB and its
	/// position
	/// @param[in] i 0: left, 1: right
	/// @param[in] j 0: down, 1: up
	/// @param[in] k 0: back, 1: front
	/// @param[in] parentAabb The parent's AABB
	/// @param[out] out The out AABB
	void calcAabb(uint i, uint j, uint k, const Aabb& parentAabb,
		Aabb& out) const;
};

} // end namespace anki

#endif
