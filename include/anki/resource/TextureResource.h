#ifndef ANKI_RESOURCE_TEXTURE_RESOURCE_H
#define ANKI_RESOURCE_TEXTURE_RESOURCE_H

#include "anki/gl/Texture.h"

namespace anki {

class Image;

/// Texture resource class
///
/// It loads or creates an image and then loads it in the GPU. It supports 
/// compressed and uncompressed TGAs and some of the formats of PNG 
/// (PNG loading uses libpng)
class TextureResource: public Texture
{
public:
	/// Load a texture
	void load(const char* filename);

	/// Load a texture
	void load(const Image& img);
};

} // end namespace

#endif
