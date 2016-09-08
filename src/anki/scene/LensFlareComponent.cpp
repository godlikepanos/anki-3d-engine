// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	// Resource group
	ResourceGroupInitInfo rcInit;
	rcInit.m_textures[0].m_texture = m_tex->getGrTexture();
	rcInit.m_textures[0].m_usage = TextureUsageBit::SAMPLED_FRAGMENT;
	rcInit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;
	m_rcGroup = getSceneGraph().getGrManager().newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

} // end namespace anki
