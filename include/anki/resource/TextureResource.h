// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_TEXTURE_RESOURCE_H
#define ANKI_RESOURCE_TEXTURE_RESOURCE_H

#include "anki/resource/Common.h"
#include "anki/Gl.h"

namespace anki {

// Forward
class Image;

/// @addtogroup resource
/// @{

/// Texture resource class.
///
/// It loads or creates an image and then loads it in the GPU. It supports 
/// compressed and uncompressed TGAs and AnKi's texture format.
class TextureResource
{
public:
	/// Load a texture
	void load(const CString& filename, ResourceInitializer& init);

	/// Get the GL texture
	const GlTextureHandle& getGlTexture() const
	{
		return m_tex;
	}

	/// Get the GL texture
	GlTextureHandle& getGlTexture()
	{
		return m_tex;
	}

private:
	GlTextureHandle m_tex;

	/// Load a texture
	void loadInternal(const CString& filename, ResourceInitializer& init);
};
/// @}

} // end namespace anki

#endif
