#ifndef ANKI_SCENE_OCTREE_H
#define ANKI_SCENE_OCTREE_H

#include "anki/collision/Aabb.h"
#include <boost/array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


namespace anki {


/// XXX
class OctreeNode
{
	friend class Octree;

	public:
		OctreeNode()
		:	parent(NULL)
		{
			for(size_t i = 0; children.size(); ++i)
			{
				children[i] = NULL;
			}
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

		/// Means no more children
		bool isFinal() const
		{
			return children[0] == NULL;
		}

	private:
		boost::array<OctreeNode*, 8> children;
		OctreeNode* parent;
		Aabb aabb; ///< Including AABB
};


/// XXX
class Octree
{
	public:
		Octree(const Aabb& aabb, uint depth);

	private:
		OctreeNode* root;
		boost::ptr_vector<OctreeNode> nodes; ///< For garbage collection

		void createSubTree(uint rdepth, OctreeNode& parent);
};


} // end namespace


#endif
