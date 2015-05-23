// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_TEXTURE_RESOURCE_H
#define ANKI_RESOURCE_TEXTURE_RESOURCE_H

#include "anki/resource/ResourceObject.h"
#include "anki/resource/ResourcePointer.h"
#include "anki/Gr.h"

namespace anki {

// Forward
class Image;

/// @addtogroup resource
/// @{

/// Texture resource class.
///
/// It loads or creates an image and then loads it in the GPU. It supports
/// compressed and uncompressed TGAs and AnKi's texture format.
class TextureResource: public ResourceObject
{
public:
	TextureResource(ResourceManager* manager)
	:	ResourceObject(manager)
	{}

	~TextureResource();

	/// Load a texture
	ANKI_USE_RESULT Error load(const CString& filename);

	/// Get the GL texture
	const TexturePtr& getGlTexture() const
	{
		return m_tex;
	}

	/// Get the GL texture
	TexturePtr& getGlTexture()
	{
		return m_tex;
	}

	U32 getWidth() const
	{
		return m_size.x();
	}

	U32 getHeight() const
	{
		return m_size.y();
	}

	U32 getDepth() const
	{
		return m_size.z();
	}

private:
	TexturePtr m_tex;
	UVec3 m_size;
};
/// @}

} // end namespace anki

#endif
