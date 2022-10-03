// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSEJ

#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(SkyboxComponent)

SkyboxComponent::SkyboxComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
}

SkyboxComponent::~SkyboxComponent()
{
}

void SkyboxComponent::setImage(CString filename)
{
	const Error err = m_node->getSceneGraph().getResourceManager().loadResource(filename, m_image);
	if(err)
	{
		ANKI_SCENE_LOGE("Setting skybox image failed. Ignoring error");
	}
	else
	{
		m_type = SkyboxType::kImage2D;
	}
}

void SkyboxComponent::setupSkyboxQueueElement(SkyboxQueueElement& queueElement) const
{
	if(m_type == SkyboxType::kImage2D)
	{
		queueElement.m_skyboxTexture = m_image->getTextureView().get();
	}
	else
	{
		queueElement.m_skyboxTexture = nullptr;
		queueElement.m_solidColor = m_color;
	}

	queueElement.m_fog.m_minDensity = m_fog.m_minDensity;
	queueElement.m_fog.m_maxDensity = m_fog.m_maxDensity;
	queueElement.m_fog.m_heightOfMinDensity = m_fog.m_heightOfMinDensity;
	queueElement.m_fog.m_heightOfMaxDensity = m_fog.m_heightOfMaxDensity;
	queueElement.m_fog.m_scatteringCoeff = m_fog.m_scatteringCoeff;
	queueElement.m_fog.m_absorptionCoeff = m_fog.m_absorptionCoeff;
	queueElement.m_fog.m_diffuseColor = m_fog.m_diffuseColor;
}

} // end namespace anki
