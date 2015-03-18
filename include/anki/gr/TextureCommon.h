// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_TEXTURE_COMMON_H
#define ANKI_GR_TEXTURE_COMMON_H

#include "anki/gr/Common.h"

namespace anki {

/// @addtogroup graphics
/// @{

struct SurfaceData
{
	void* m_ptr = nullptr;
	PtrSize m_size = 0;
};

/// Texture initializer.
struct TextureInitializer
{
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0; ///< Relevant only for 3D and 2DArray textures
	GLenum m_target = GL_TEXTURE_2D;
	GLenum m_internalFormat = GL_NONE;
	U32 m_mipmapsCount = 0;
	TextureFilter m_filterType = TextureFilter::NEAREST;
	Bool8 m_repeat = false;
	I32 m_anisotropyLevel = 0;
	U32 m_samples = 1;

	/// [level][slice]
	Array2d<SurfaceData, MAX_MIPMAPS, MAX_TEXTURE_LAYERS> m_data;
};
/// @}

} // end namespace anki

#endif


