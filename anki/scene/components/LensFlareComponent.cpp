// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/LensFlareComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

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

Error LensFlareComponent::loadTextureResource(CString textureFilename)
{
	return m_node->getSceneGraph().getResourceManager().loadResource(textureFilename, m_tex);
}

} // end namespace anki
