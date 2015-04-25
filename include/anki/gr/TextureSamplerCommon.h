// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_TEXTURE_SAMPLER_COMMON_H
#define ANKI_GR_TEXTURE_SAMPLER_COMMON_H

#include "anki/gr/Common.h"

namespace anki {

/// @addtogroup graphics
/// @{

class SamplerInitializer
{
public:
	SamplingFilter m_minMagFilter = SamplingFilter::NEAREST;
	SamplingFilter m_mipmapFilter = SamplingFilter::BASE;
	CompareOperation m_compareOperation = CompareOperation::ALWAYS;
	F32 m_minLod = -1000.0;
	F32 m_maxLod = 1000.0;
	I8 m_anisotropyLevel = 0;
	Bool8 m_repeat = true;
};

class SurfaceData
{
public:
	const void* m_ptr = nullptr;
	PtrSize m_size = 0;
};

/// Texture initializer.
class TextureInitializer
{
public:
	TextureType m_type = TextureType::_2D;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0; ///< Relevant only for 3D and 2DArray textures
	PixelFormat m_format;
	U8 m_mipmapsCount = 0;
	U8 m_samples = 1;

	SamplerInitializer m_sampling;

	/// In some backends it may copy the m_data to a temp buffer for async 
	/// operations.
	Bool8 m_copyDataBeforeReturn = true;

	/// [level][slice]
	Array2d<SurfaceData, MAX_MIPMAPS, MAX_TEXTURE_LAYERS> m_data;
};
/// @}

} // end namespace anki

#endif

