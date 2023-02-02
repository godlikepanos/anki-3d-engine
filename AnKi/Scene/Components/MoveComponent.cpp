// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

Error MoveComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = info.m_node->updateTransform();
	return Error::kNone;
}

} // end namespace anki
