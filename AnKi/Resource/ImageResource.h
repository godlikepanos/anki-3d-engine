// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup resource
/// @{

/// Image resource class.
///
/// It loads or creates an image and then loads it in the GPU. It supports compressed and uncompressed TGAs, PNGs, JPEG
/// and AnKi's image format.
class ImageResource : public ResourceObject
{
public:
	ImageResource(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~ImageResource();

	/// Load an image.
	Error load(const ResourceFilename& filename, Bool async);

	/// Get the texture.
	const TexturePtr& getTexture() const
	{
		return m_tex;
	}

	/// Get the texture view.
	const TextureViewPtr& getTextureView() const
	{
		return m_texView;
	}

	U32 getWidth() const
	{
		ANKI_ASSERT(m_size.x());
		return m_size.x();
	}

	U32 getHeight() const
	{
		ANKI_ASSERT(m_size.y());
		return m_size.y();
	}

	U32 getDepth() const
	{
		ANKI_ASSERT(m_size.z());
		return m_size.z();
	}

	U32 getLayerCount() const
	{
		ANKI_ASSERT(m_layerCount);
		return m_layerCount;
	}

private:
	static constexpr U32 kMaxCopiesBeforeFlush = 4;

	class TexUploadTask;
	class LoadingContext;

	TexturePtr m_tex;
	TextureViewPtr m_texView;
	UVec3 m_size = UVec3(0u);
	U32 m_layerCount = 0;

	[[nodiscard]] static Error load(LoadingContext& ctx);
};
/// @}

} // end namespace anki
