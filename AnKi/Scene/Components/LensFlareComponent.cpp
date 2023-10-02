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
	: SceneComponent(node, kClassType)
{
}

LensFlareComponent::~LensFlareComponent()
{
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
	}

	return Error::kNone;
}

} // end namespace anki
