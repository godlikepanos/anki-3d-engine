// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(LensFlareComponent)

LensFlareComponent::LensFlareComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
	ANKI_ASSERT(node);
}

LensFlareComponent::~LensFlareComponent()
{
}

Error LensFlareComponent::loadImageResource(CString filename)
{
	return m_node->getSceneGraph().getResourceManager().loadResource(filename, m_image);
}

} // end namespace anki
