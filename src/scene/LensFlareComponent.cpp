// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ResourceManager.h"

namespace anki {

//==============================================================================
LensFlareComponent::LensFlareComponent(SceneNode* node)
	: SceneComponent(Type::LENS_FLARE, node)
{}

//==============================================================================
LensFlareComponent::~LensFlareComponent()
{}

//==============================================================================
Error LensFlareComponent::create(const CString& textureFilename)
{
	// Texture
	ANKI_CHECK(getSceneGraph()._getResourceManager().loadResource(
		textureFilename, m_tex));

	// Queries
	GrManager& gr = getSceneGraph().getGrManager();
	for(auto it = m_queries.getBegin(); it != m_queries.getEnd(); ++it)
	{
		(*it) = gr.newInstance<OcclusionQuery>(
			OcclusionQueryResultBit::VISIBLE);
	}

	// Resource group
	ResourceGroupInitializer rcInit;
	rcInit.m_textures[0].m_texture = m_tex->getGrTexture();
	rcInit.m_uniformBuffers[0].m_dynamic = true;
	m_rcGroup = gr.newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

//==============================================================================
OcclusionQueryPtr& LensFlareComponent::getOcclusionQueryToTest()
{
	// Move the query counter
	m_crntQueryIndex = (m_crntQueryIndex + 1) % m_queries.getSize();

	// Save the timestamp
	m_queryTestTimestamp[m_crntQueryIndex] = getGlobalTimestamp();

	return m_queries[m_crntQueryIndex];
}

//==============================================================================
void LensFlareComponent::getOcclusionQueryToCheck(
	OcclusionQueryPtr& q, Bool& queryInvalid)
{
	U idx = (m_crntQueryIndex + 1) % m_queries.getSize();

	if(m_queryTestTimestamp[idx] == MAX_U32
		|| m_queryTestTimestamp[idx] != getGlobalTimestamp() - 2)
	{
		queryInvalid = true;
	}
	else
	{
		queryInvalid = false;
		q = m_queries[idx];
	}
}

} // end namespace anki

