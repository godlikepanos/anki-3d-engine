// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup resource
/// @{

ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageSize, 1024u * 1024u, 4u, kMaxU32, "Max image size to load")

/// Image resource class. It loads or creates an image and then loads it in the GPU. It supports compressed and uncompressed TGAs, PNGs, JPEG and
/// AnKi's image format.
class ImageResource : public ResourceObject
{
public:
	ImageResource(CString fname, U32 uuid)
		: ResourceObject(fname, uuid)
	{
	}

	~ImageResource();

	/// Load an image.
	Error load(const ResourceFilename& filename, Bool async);

	/// Get the texture.
	Texture& getTexture() const
	{
		return *m_tex;
	}

	Vec4 getAverageColor() const
	{
		return m_avgColor;
	}

	Bool isLoaded() const
	{
		return m_loadedMipCount.load() == m_tex->getMipmapCount();
	}

private:
	static constexpr U32 kMaxCopiesBeforeFlush = 4;

	class TexUploadTask;
	class LoadingContext;

	TexturePtr m_tex;

	Vec4 m_avgColor = Vec4(0.0f);

	mutable Atomic<U32> m_loadedMipCount = {0};

	Error loadAsync(LoadingContext& ctx) const;
};
/// @}

} // end namespace anki
