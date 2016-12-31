// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

LensFlareComponent::LensFlareComponent(SceneNode* node)
	: SceneComponent(CLASS_TYPE, node)
{
}

LensFlareComponent::~LensFlareComponent()
{
}

Error LensFlareComponent::init(const CString& textureFilename)
{
	// Texture
	ANKI_CHECK(getSceneGraph().getResourceManager().loadResource(textureFilename, m_tex));

	return ErrorCode::NONE;
}

} // end namespace anki
