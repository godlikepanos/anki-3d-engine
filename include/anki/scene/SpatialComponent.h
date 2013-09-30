#ifndef ANKI_SCENE_SPATIAL_COMPONENT_H
#define ANKI_SCENE_SPATIAL_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/Collision.h"
#include "anki/util/Bitset.h"
#include "anki/core/Timestamp.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Spatial component for scene nodes. It indicates scene nodes that need to 
/// be placed in the scene's octree and they participate in the visibility 
/// tests
class SpatialComponent: 
	public SceneHierarchicalObject<SpatialComponent>, 
	public Bitset<U8>
{
public:
	typedef SceneHierarchicalObject<SpatialComponent> Base;

	/// Spatial flags
	enum SpatialFlag
	{
		SF_NONE = 0,
		SF_VISIBLE_CAMERA = 1 << 1,
		SF_VISIBLE_LIGHT = 1 << 2,
		/// Visible or not. The visibility tester sets it
		SF_VISIBLE_ANY = SF_VISIBLE_CAMERA | SF_VISIBLE_LIGHT,

		/// This is used for example in lights. If the light does not collide 
		/// with any surface then it shouldn't be visible and be processed 
		/// further. This flag is being used to check if we should test agains
		/// near plane when using the tiler for visibility tests.
		SF_FULLY_TRANSPARENT = 1 << 3,

		SF_MARKED_FOR_UPDATE = 1 << 4
	};

	/// Pass the collision shape here so we can avoid the virtuals
	SpatialComponent(const CollisionShape* cs, const SceneAllocator<U8>& alloc,
		U32 flags = SF_NONE)
		: Base(nullptr, alloc), spatialCs(cs)
	{
		enableBits(flags | SF_MARKED_FOR_UPDATE);
	}

	// Remove from current OctreeNode
	virtual ~SpatialComponent();

	/// @name Accessors
	/// @{
	const CollisionShape& getSpatialCollisionShape() const
	{
		return *spatialCs;
	}

	const Aabb& getAabb() const
	{
		return aabb;
	}

	/// Get optimal collision shape for visibility tests
	const CollisionShape& getOptimalCollisionShape() const
	{
		if(spatialCs->getCollisionShapeType() == CollisionShape::CST_SPHERE)
		{
			return *spatialCs;
		}
		else
		{
			return aabb;
		}
	}

	Timestamp getSpatialTimestamp() const
	{
		return timestamp;
	}

	/// Used for sorting spatials. In most object the origin is the center of
	/// the bounding volume but for cameras the origin is the eye point
	virtual const Vec3& getSpatialOrigin() const
	{
		return origin;
	}

	PtrSize getSubSpatialsCount() const
	{
		return getChildrenSize();
	}

	SpatialComponent& getSubSpatial(PtrSize i)
	{
		return getChild(i);
	}
	const SpatialComponent& getSubSpatial(PtrSize i) const
	{
		return getChild(i);
	}
	/// @}

	/// Visit this an the rest of the tree spatials
	template<typename VisitorFunc>
	void visitSubSpatials(VisitorFunc vis)
	{
		SceneHierarchicalObject<SpatialComponent>::visitChildren(vis);
	}

	/// The derived class has to manually set when the collision shape got
	/// updated
	void spatialMarkForUpdate()
	{
		visitThisAndChildren([](SpatialComponent& sp)
		{
			sp.enableBits(SF_MARKED_FOR_UPDATE);
		});
	}

	void update()
	{
		visitThisAndChildren([](SpatialComponent& sp)
		{
			sp.updateInternal();
		});
	}

	void resetFrame()
	{
		visitThisAndChildren([](SpatialComponent& sp)
		{
			sp.disableBits(SF_VISIBLE_ANY);
		});
	}

protected:
	const CollisionShape* spatialCs = nullptr;

private:
	Aabb aabb; ///< A faster shape
	Vec3 origin; ///< Cached value
	Timestamp timestamp = getGlobTimestamp();

	void updateInternal()
	{
		spatialCs->toAabb(aabb);
		origin = (aabb.getMax() + aabb.getMin()) * 0.5;
		timestamp = getGlobTimestamp();
		disableBits(SF_MARKED_FOR_UPDATE);
	}
};
/// @}

} // end namespace anki

#endif
