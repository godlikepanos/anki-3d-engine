#ifndef ANKI_RESOURCE_TEXTURE_RESOURCE_H
#define ANKI_RESOURCE_TEXTURE_RESOURCE_H

#include "anki/Gl.h"

namespace anki {

// Forward
class Image;

/// @addtogroup Resource
/// @{

/// Texture resource class
///
/// It loads or creates an image and then loads it in the GPU. It supports 
/// compressed and uncompressed TGAs and some of the formats of PNG 
/// (PNG loading uses libpng)
class TextureResource
{
public:
	/// Load a texture
	void load(const char* filename);

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
	void loadInternal(const char* filename);
};
/// @}

} // end namespace anki

#endif
