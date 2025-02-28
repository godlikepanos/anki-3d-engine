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
}

} // end namespace anki
