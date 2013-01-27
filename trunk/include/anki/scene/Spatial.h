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
class Spatial: public Flags<U32>
{
	friend class OctreeNode;
	friend class Grid;

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
	Spatial(SceneNode* sceneNode_, CollisionShape* cs)
		: spatialCs(cs), sceneNode(sceneNode_)
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

	const Vec3& getSpatialOrigin() const
	{
		return origin;
	}
	/// @}

	/// The derived class has to manually set when the collision shape got
	/// updated
	void spatialMarkUpdated()
	{
		timestamp = Timestamp::getTimestamp();
		spatialCs->toAabb(aabb);
		origin = (aabb.getMin() + aabb.getMax()) * 0.5;
	}

protected:
	CollisionShape* spatialCs = nullptr;

private:
	U32 timestamp = Timestamp::getTimestamp();
	OctreeNode* octreeNode = nullptr; ///< What octree node includes this
	Grid* grid = nullptr;
	Aabb aabb; ///< A faster shape
	SceneNode* sceneNode; ///< Know your father
	Vec3 origin;
};
/// @}

} // end namespace anki

#endif
