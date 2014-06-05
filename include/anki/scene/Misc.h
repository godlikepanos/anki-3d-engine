// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_MISC_H
#define ANKI_SCENE_MISC_H

#include "anki/scene/SpatialComponent.h"

namespace anki {

/// A class that holds spatial information and implements the SpatialComponent
/// virtuals. You just need to update the OBB manually
struct ObbSpatialComponent: public SpatialComponent
{
	Obb obb;

	ObbSpatialComponent(SceneNode* node)
		: SpatialComponent(node)
	{}

	/// Implement SpatialComponent::getSpatialCollisionShape
	const CollisionShape& getSpatialCollisionShape()
	{
		return obb;
	}

	/// Implement SpatialComponent::getSpatialOrigin
	Vec3 getSpatialOrigin()
	{
		return obb.getCenter();
	}
};

} // end namespace anki

#endif
