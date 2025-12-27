// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSEJ

#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

SkyboxComponent& SkyboxComponent::setSkyImageFilename(CString filename)
{
	ImageResourcePtr img;
	if(ResourceManager::getSingleton().loadResource(filename, img))
	{
		ANKI_SCENE_LOGE("Setting skybox image failed: %s", filename.cstr());
	}
	else
	{
		m_sky.m_image = std::move(img);
	}

	return *this;
}

void SkyboxComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = false;
}

Error SkyboxComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_type, 1);
	ANKI_SERIALIZE(m_sky.m_color, 1);
	ANKI_SERIALIZE(m_sky.m_image, 1);
	ANKI_SERIALIZE(m_sky.m_imageColorScale, 1);
	ANKI_SERIALIZE(m_sky.m_imageColorBias, 1);
	ANKI_SERIALIZE(m_fog.m_scatteringCoeff, 1);
	ANKI_SERIALIZE(m_fog.m_minDensity, 1);
	ANKI_SERIALIZE(m_fog.m_maxDensity, 1);
	ANKI_SERIALIZE(m_fog.m_heightOfMinDensity, 1);
	ANKI_SERIALIZE(m_fog.m_heightOfMaxDensity, 1);
	ANKI_SERIALIZE(m_fog.m_diffuseColor, 1);
	ANKI_SERIALIZE(m_fog.m_absorptionCoeff, 1);

	return Error::kNone;
}

} // end namespace anki
