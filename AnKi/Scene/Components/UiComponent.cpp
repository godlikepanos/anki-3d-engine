// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

Error UiComponent::updateReal(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_spatial.update(info.m_node->getSceneGraph().getOctree());
	return Error::kNone;
}

void UiComponent::onDestroy(SceneNode& node)
{
	m_spatial.removeFromOctree(node.getSceneGraph().getOctree());
}

} // end namespace anki
