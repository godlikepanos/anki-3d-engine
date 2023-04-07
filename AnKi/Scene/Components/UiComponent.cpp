// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

UiComponent ::~UiComponent()
{
	m_spatial.removeFromOctree(SceneGraph::getSingleton().getOctree());
}

Error UiComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_spatial.update(SceneGraph::getSingleton().getOctree());
	return Error::kNone;
}

} // end namespace anki
