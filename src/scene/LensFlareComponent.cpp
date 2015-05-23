// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/resource/TextureResource.h"

namespace anki {

//==============================================================================
Error LensFlareComponent::create(const CString& textureFilename)
{
	// Texture
	ANKI_CHECK(m_tex.load(
		textureFilename, &m_node->getSceneGraph()._getResourceManager()));

	// Queries
	GrManager& gr = m_node->getSceneGraph().getGrManager();
	for(auto it = m_queries.getBegin(); it != m_queries.getEnd(); ++it)
	{
		(*it).create(&gr, OcclusionQueryPtr::ResultBit::VISIBLE);
	}

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

