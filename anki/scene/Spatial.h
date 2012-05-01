#ifndef ANKI_SCENE_SPATIAL_H
#define ANKI_SCENE_SPATIAL_H

#include "anki/collision/Collision.h"


namespace anki {


/// @addtogroup Scene
/// @{

/// Spatial "interface" for scene nodes
///
/// It indicates scene nodes that need to be placed in the scene's octree and
/// they participate in the visibility tests
class Spatial
{
public:
	/// Spatial flags
	enum SpatialFlag
	{
		SF_NONE = 0,
		SF_VISIBLE = 1 ///< Visible or not. The visibility tester sets it
	};

	/// Pass the collision shape here so we can avoid the virtuals
	Spatial(CollisionShape* cs)
		: spatialCs(cs), flags(SF_NONE)
	{}

	/// @name Accessors
	/// @{
	const CollisionShape& getSpatialCollisionShape() const
	{
		return *spatialCs;
	}
	CollisionShape& getSpatialCollisionShape()
	{
		return *spatialCs;
	}
	/// @}

	/// @name Flag manipulation
	/// @{
	void enableFlag(SpatialFlag flag, bool enable = true)
	{
		flags = enable ? flags | flag : flags & ~flag;
	}
	void disableFlag(SpatialFlag flag)
	{
		enableFlag(flag, false);
	}
	bool isFlagEnabled(SpatialFlag flag) const
	{
		return flags & flag;
	}
	uint getFlagsBitmask() const
	{
		return flags;
	}
	/// @}

protected:
	CollisionShape* spatialCs;
	uint flags;
};
/// @}


} // namespace anki


#endif
