// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

SceneComponent::SceneComponent([[maybe_unused]] SceneNode* node, SceneComponentType type, U32 uuid)
	: m_uuid(uuid)
	, m_type(U8(type))
{
	ANKI_ASSERT(uuid);
	ANKI_ASSERT(SceneComponentType(m_type) < SceneComponentType::kCount);
}

} // namespace anki
