// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SkyboxNode.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>

namespace anki {

SkyboxNode::SkyboxNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<SkyboxComponent>();

	SpatialComponent* spatialc = newComponent<SpatialComponent>();
	spatialc->setAlwaysVisible(true);
	spatialc->setSpatialOrigin(Vec3(0.0f));
	spatialc->setUpdateOctreeBounds(false);

	newComponent<MoveComponent>();
}

} // end namespace anki
