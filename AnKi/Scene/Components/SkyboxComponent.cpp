// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSEJ

#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

SkyboxComponent::SkyboxComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	SceneGraph::getSingleton().addSkybox(this);
}

SkyboxComponent::~SkyboxComponent()
{
	SceneGraph::getSingleton().removeSkybox(this);
}

void SkyboxComponent::loadImageResource(CString filename)
{
	ImageResourcePtr img;
	const Error err = ResourceManager::getSingleton().loadResource(filename, img);
	if(err)
	{
		ANKI_SCENE_LOGE("Setting skybox image failed. Ignoring error");
		return;
	}

	m_image = std::move(img);
	m_type = SkyboxType::kImage2D;
}

Error SkyboxComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = false;
	return Error::kNone;
}

} // end namespace anki
