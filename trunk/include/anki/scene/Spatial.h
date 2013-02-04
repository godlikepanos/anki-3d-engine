#ifndef ANKI_SCENE_SPATIAL_H
#define ANKI_SCENE_SPATIAL_H

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
		SF_VISIBLE_ANY = SF_VISIBLE_CAMERA | SF_VISIBLE_LIGHT
	};

	/// Pass the collision shape here so we can avoid the virtuals
	Spatial(CollisionShape* cs)
		: spatialCs(cs)
	{}

	// Remove from current OctreeNode
	~Spatial();

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

	OctreeNode& getOctreeNode()
	{
		ANKI_ASSERT(octreeNode != nullptr && "Not placed yet");
		return *octreeNode;
	}
	const OctreeNode& getOctreeNode() const
	{
		ANKI_ASSERT(octreeNode != nullptr && "Not placed yet");
		return *octreeNode;
	}
	/// @}

	/// The derived class has to manually set when the collision shape got
	/// updated
	void spatialMarkUpdated()
	{
		timestamp = Timestamp::getTimestamp();
		spatialCs->toAabb(aabb);
		origin = (aabb.getMax() + aabb.getMin()) * 0.5;
	}

protected:
	CollisionShape* spatialCs = nullptr;

private:
	U32 timestamp = Timestamp::getTimestamp();
	OctreeNode* octreeNode = nullptr; ///< What octree node includes this
	Aabb aabb; ///< A faster shape
	Vec3 origin; ///< Cached value
};
/// @}

} // end namespace anki

#endif
