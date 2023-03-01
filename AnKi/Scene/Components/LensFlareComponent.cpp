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
	, m_node(node)
	, m_spatial(this)
{
}

LensFlareComponent::~LensFlareComponent()
{
	m_spatial.removeFromOctree(m_node->getSceneGraph().getOctree());
}

void LensFlareComponent::loadImageResource(CString filename)
{
	ImageResourcePtr image;
	const Error err = getExternalSubsystems(*m_node).m_resourceManager->loadResource(filename, image);
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

	const Bool spatialUpdated = m_spatial.update(info.m_node->getSceneGraph().getOctree());
	updated = updated || spatialUpdated;

	return Error::kNone;
}

void LensFlareComponent::onDestroy(SceneNode& node)
{
	m_spatial.removeFromOctree(node.getSceneGraph().getOctree());
}

} // end namespace anki
