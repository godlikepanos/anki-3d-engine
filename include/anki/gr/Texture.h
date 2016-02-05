// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Sampler initializer.
class SamplerInitInfo
{
public:
	SamplingFilter m_minMagFilter = SamplingFilter::NEAREST;
	SamplingFilter m_mipmapFilter = SamplingFilter::BASE;
	CompareOperation m_compareOperation = CompareOperation::ALWAYS;
	F32 m_minLod = -1000.0;
	F32 m_maxLod = 1000.0;
	I8 m_anisotropyLevel = 0;
	Bool8 m_repeat = true; ///< Repeat or clamp.
};

/// Texture initializer.
class TextureInitInfo
{
public:
	TextureType m_type = TextureType::_2D;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0; ///< Relevant only for 3D, 2DArray and CubeArray textures
	PixelFormat m_format;
	U8 m_mipmapsCount = 0;
	U8 m_samples = 1;

	SamplerInitInfo m_sampling;
};

/// GPU texture
class Texture : public GrObject
{
public:
	/// Construct.
	Texture(GrManager* manager);

	/// Destroy.
	~Texture();

	/// Access the implementation.
	TextureImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create it.
	void create(const TextureInitInfo& init);

private:
	UniquePtr<TextureImpl> m_impl;
};
/// @}

} // end namespace anki
