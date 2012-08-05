#ifndef ANKI_SCENE_SPATIAL_H
#define ANKI_SCENE_SPATIAL_H

#include "anki/collision/Collision.h"
#include "anki/util/Flags.h"
#include "anki/scene/Scene.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Spatial "interface" for scene nodes. It indicates scene nodes that need to 
/// be placed in the scene's octree and they participate in the visibility 
/// tests
class Spatial: public Flags<uint32_t>
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
		: spatialCs(cs)
	{}

	/// @name Accessors
	/// @{
	const CollisionShape& getSpatialCollisionShape() const
	{
		return *spatialCs;
	}

	uint32_t getSpatialLastUpdateFrame() const
	{
		return lastUpdateFrame;
	}
	/// @}

	/// The derived class has to manually set when the collision shape got
	/// updated
	void spatialMarkUpdated()
	{
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}

protected:
	CollisionShape* spatialCs;

private:
	uint32_t lastUpdateFrame = SceneSingleton::get().getFramesCount();
};
/// @}

} // namespace anki

#endif
