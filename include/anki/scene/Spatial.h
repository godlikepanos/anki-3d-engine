#ifndef ANKI_SCENE_SPATIAL_H
#define ANKI_SCENE_SPATIAL_H

#include "anki/scene/Common.h"
#include "anki/collision/Collision.h"
#include "anki/util/Flags.h"
#include "anki/core/Timestamp.h"

namespace anki {

class OctreeNode;
class Grid;
class SceneNode;

/// @addtogroup Scene
/// @{

/// Spatial "interface" for scene nodes. It indicates scene nodes that need to 
/// be placed in the scene's octree and they participate in the visibility 
/// tests
class Spatial: public Flags<U8>
{
	friend class OctreeNode;
	friend class Grid;
	friend class SceneNode;

public:
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
		SF_FULLY_TRANSPARENT = 1 << 3
	};

	/// Pass the collision shape here so we can avoid the virtuals
	Spatial(const CollisionShape* cs, const SceneAllocator<U8>& alloc,
		U32 flags = SF_NONE)
		: spatialProtected(cs, alloc)
	{
		enableFlags(flags);
	}

	// Remove from current OctreeNode
	virtual ~Spatial();

	/// @name Accessors
	/// @{
	const CollisionShape& getSpatialCollisionShape() const
	{
		return *spatialProtected.spatialCs;
	}

	const Aabb& getAabb() const
	{
		return aabb;
	}

	/// Get optimal collision shape for visibility tests
	const CollisionShape& getOptimalCollisionShape() const
	{
		if(spatialProtected.spatialCs->getCollisionShapeType() 
			== CollisionShape::CST_SPHERE)
		{
			return *spatialProtected.spatialCs;
		}
		else
		{
			return aabb;
		}
	}

	U32 getSpatialTimestamp() const
	{
		return timestamp;
	}

	/// Used for sorting spatials. In most object the origin is the center of
	/// the bounding volume but for cameras the origin is the eye point
	virtual const Vec3& getSpatialOrigin() const
	{
		return origin;
	}

	OctreeNode* getOctreeNode()
	{
		return octreeNode;
	}
	const OctreeNode* getOctreeNode() const
	{
		return octreeNode;
	}

	SceneVector<Spatial*>::iterator getSubSpatialsBegin()
	{
		return spatialProtected.subSpatials.begin();
	}
	SceneVector<Spatial*>::const_iterator getSubSpatialsBegin() const
	{
		return spatialProtected.subSpatials.begin();
	}
	SceneVector<Spatial*>::iterator getSubSpatialsEnd()
	{
		return spatialProtected.subSpatials.end();
	}
	SceneVector<Spatial*>::const_iterator getSubSpatialsEnd() const
	{
		return spatialProtected.subSpatials.end();
	}
	PtrSize getSubSpatialsCount() const
	{
		return spatialProtected.subSpatials.size();
	}
	/// @}

	/// The derived class has to manually set when the collision shape got
	/// updated
	void spatialMarkForUpdate()
	{
		timestamp = Timestamp::getTimestamp();

		for(Spatial* subsp : spatialProtected.subSpatials)
		{
			subsp->spatialMarkForUpdate();
		}
	}

	void update()
	{
		spatialProtected.spatialCs->toAabb(aabb);
		origin = (aabb.getMax() + aabb.getMin()) * 0.5;

		for(Spatial* subsp : spatialProtected.subSpatials)
		{
			subsp->update();
		}
	}

	void resetFrame()
	{
		disableFlags(SF_VISIBLE_ANY);

		for(Spatial* subsp : spatialProtected.subSpatials)
		{
			subsp->disableFlags(SF_VISIBLE_ANY);
		}
	}

protected:
	struct SpatialProtected
	{
		SpatialProtected(const CollisionShape* spatialCs_, 
			const SceneAllocator<U8>& alloc)
			: spatialCs(spatialCs_), subSpatials(alloc)
		{}

		const CollisionShape* spatialCs = nullptr;
		SceneVector<Spatial*> subSpatials;
	} spatialProtected;

private:
	U32 timestamp = Timestamp::getTimestamp();
	OctreeNode* octreeNode = nullptr; ///< What octree node includes this
	Aabb aabb; ///< A faster shape
	Vec3 origin; ///< Cached value
};
/// @}

} // end namespace anki

#endif
