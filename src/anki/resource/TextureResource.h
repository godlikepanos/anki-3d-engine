// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Texture resource class.
///
/// It loads or creates an image and then loads it in the GPU. It supports compressed and uncompressed TGAs and AnKi's
/// texture format.
class TextureResource : public ResourceObject
{
public:
	TextureResource(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~TextureResource();

	/// Load a texture
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	/// Get the texture
	const TexturePtr& getGrTexture() const
	{
		return m_tex;
	}

	/// Get the texture
	TexturePtr& getGrTexture()
	{
		return m_tex;
	}

	U getWidth() const
	{
		ANKI_ASSERT(m_size.x());
		return m_size.x();
	}

	U getHeight() const
	{
		ANKI_ASSERT(m_size.y());
		return m_size.y();
	}

	U getDepth() const
	{
		ANKI_ASSERT(m_size.z());
		return m_size.z();
	}

	U getLayerCount() const
	{
		ANKI_ASSERT(m_layerCount);
		return m_layerCount;
	}

private:
	TexturePtr m_tex;
	UVec3 m_size = UVec3(0u);
	U32 m_layerCount = 0;
};
/// @}

} // end namespace anki
