// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

LensFlareComponent::LensFlareComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_spatial(this)
{
}

LensFlareComponent::~LensFlareComponent()
{
	m_spatial.removeFromOctree(SceneGraph::getSingleton().getOctree());
}

void LensFlareComponent::loadImageResource(CString filename)
{
	ImageResourcePtr image;
	const Error err = ResourceManager::getSingleton().loadResource(filename, image);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load lens flare image");
		return;
	}

	m_image = std::move(image);
}

Error LensFlareComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_dirty || info.m_node->movedThisFrame();

	if(updated)
	{
		m_dirty = false;

		m_worldPosition = info.m_node->getWorldTransform().getOrigin().xyz();

		const Aabb aabb(m_worldPosition - (kAabbSize / 2.0f), m_worldPosition + (kAabbSize / 2.0f));
		m_spatial.setBoundingShape(aabb);
	}

	const Bool spatialUpdated = m_spatial.update(SceneGraph::getSingleton().getOctree());
	updated = updated || spatialUpdated;

	return Error::kNone;
}

void LensFlareComponent::onDestroy([[maybe_unused]] SceneNode& node)
{
	m_spatial.removeFromOctree(SceneGraph::getSingleton().getOctree());
}

} // end namespace anki
