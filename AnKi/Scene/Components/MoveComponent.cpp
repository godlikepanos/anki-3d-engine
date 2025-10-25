// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

void MoveComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = info.m_node->updateTransform();

	if(updated) [[unlikely]]
	{
		Array<Mat3x4, 2> trfs;
		trfs[0] = Mat3x4(info.m_node->getWorldTransform());
		trfs[1] = Mat3x4(info.m_node->getPreviousWorldTransform());
		m_gpuSceneTransforms.uploadToGpuScene(trfs);
	}
}

} // end namespace anki
