// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/LensFlareComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

LensFlareComponent::LensFlareComponent(SceneNode* node)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
{
	ANKI_ASSERT(node);
}

LensFlareComponent::~LensFlareComponent()
{
}

Error LensFlareComponent::init(const CString& textureFilename)
{
	// Texture
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(textureFilename, m_tex));

	return Error::NONE;
}

} // end namespace anki
