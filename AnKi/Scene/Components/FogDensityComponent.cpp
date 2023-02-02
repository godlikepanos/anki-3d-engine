// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

void FogDensityComponent::onDestroy(SceneNode& node)
{
	m_spatial.removeFromOctree(node.getSceneGraph().getOctree());
}

Error FogDensityComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_dirty || info.m_node->movedThisFrame();

	if(updated)
	{
		m_dirty = false;

		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();

		if(m_isBox)
		{
			const Aabb aabb(m_aabbMin + m_worldPos, m_aabbMax + m_worldPos);
			m_spatial.setBoundingShape(aabb);
		}
		else
		{
			const Sphere sphere(m_worldPos, m_sphereRadius);
			m_spatial.setBoundingShape(sphere);
		}

		m_spatial.update(info.m_node->getSceneGraph().getOctree());
	}

	return Error::kNone;
}

} // end namespace anki
