#ifndef ANKI_SCENE_OCTREE_H
#define ANKI_SCENE_OCTREE_H

#include "anki/collision/Aabb.h"
#include "anki/util/StdTypes.h"
#include <boost/array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


namespace anki {


/// XXX
class OctreeNode
{
	public:
		OctreeNode(const Aabb& aabb_, OctreeNode* parent_)
		:	parent(parent_),
		 	aabb(aabb_)
		{
			initArr();
		}

		/// @name Accessors
		/// @{
		const boost::array<OctreeNode*, 8>& getChildren() const
		{
			return children;
		}

		const OctreeNode* getParent() const
		{
			return parent;
		}

		const Aabb& getAabb() const
		{
			return aabb;
		}
		/// @}

		bool isRoot() const
		{
			return parent == NULL;
		}

		void addChild(uint pos, OctreeNode& child)
		{
			child.parent = this;
			children[pos] = &child;
		}

	private:
		boost::array<OctreeNode*, 8> children;
		OctreeNode* parent;
		Aabb aabb; ///< Including AABB

		void initArr()
		{
			for(size_t i = 0; i < children.size(); ++i)
			{
				children[i] = NULL;
			}
		}
};


/// XXX
class Octree
{
	public:
		Octree(const Aabb& aabb, uchar maxDepth, float looseness = 1.5);

		/// @name Accessors
		/// @{
		const OctreeNode& getRoot() const
		{
			return *root;
		}
		OctreeNode& getRoot()
		{
			return *root;
		}

		uint getMaxDepth() const
		{
			return maxDepth;
		}
		/// @}

		OctreeNode* place(const Aabb& aabb);

	private:
		OctreeNode* root;
		boost::ptr_vector<OctreeNode> nodes; ///< For garbage collection
		uint maxDepth;
		float looseness;

		//void createSubTree(uint rdepth, OctreeNode& parent);

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


} // end namespace


#endif
