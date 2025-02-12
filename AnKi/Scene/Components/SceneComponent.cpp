// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

SceneComponent::SceneComponent([[maybe_unused]] SceneNode* node, SceneComponentType type)
	: m_uuid(SceneGraph::getSingleton().getNewUuid())
	, m_type(U8(type))
{
}

} // namespace anki
