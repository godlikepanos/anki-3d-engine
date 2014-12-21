// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
	Error err = m_tex.load(
		textureFilename, &m_node->getSceneGraph()._getResourceManager());

	// Queries
	GlDevice& gl = m_node->getSceneGraph()._getGlDevice();
	for(auto it = m_queries.getBegin(); it != m_queries.getEnd() && !err; ++it)
	{
		err = (*it).create(&gl);
	}

	return err;
}

} // end namespace anki

