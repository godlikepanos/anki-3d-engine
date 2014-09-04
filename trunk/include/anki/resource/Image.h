// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_IMAGE_H
#define ANKI_RESOURCE_IMAGE_H

#include "anki/resource/Common.h"
#include "anki/util/Functions.h"
#include "anki/util/Vector.h"
#include "anki/util/Enum.h"

namespace anki {

/// Image class.
/// Used in Texture::load. Supported types: TGA and PNG
class Image
{
public:
	/// Texture type
	enum class TextureType: U32
	{
		NONE,
		_2D,
		CUBE,
		_3D,
		_2D_ARRAY
	};

	/// The acceptable color types of AnKi
	enum class ColorFormat: U32
	{
		NONE,
		RGB8, ///< RGB
		RGBA8 ///< RGB plus alpha
	};

	/// The data compression
	enum class DataCompression: U32
	{
		NONE,
		RAW = 1 << 0,
		S3TC = 1 << 1,
		ETC = 1 << 2
	};

	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DataCompression, friend)

	/// An image surface
	class Surface
	{
	public:
		Surface(ResourceAllocator<U8>& alloc)
		:	m_data(alloc)
		{}

		U32 m_width;
		U32 m_height;
		U32 m_mipLevel;
		ResourceVector<U8> m_data;
	};

	Image(ResourceAllocator<U8>& alloc)
	:	m_surfaces(alloc)
	{}

	ColorFormat getColorFormat() const
	{
		ANKI_ASSERT(m_colorFormat != ColorFormat::NONE);
		return m_colorFormat;
	}

	DataCompression getCompression() const
	{
		ANKI_ASSERT(m_compression != DataCompression::NONE);
		return m_compression;
	}

	U getMipLevelsCount() const
	{
		ANKI_ASSERT(m_mipLevels != 0);
		return m_mipLevels;
	}

	U getDepth() const
	{
		ANKI_ASSERT(m_depth != 0);
		return m_depth;
	}

	TextureType getTextureType() const
	{
		ANKI_ASSERT(m_textureType != TextureType::NONE);
		return m_textureType;
	}

	const Surface& getSurface(U mipLevel, U layer) const;

	ResourceAllocator<U8> getAllocator() const
	{
		return m_surfaces.get_allocator();
	}

	/// Load an image file
	/// @param[in] filename The file to load
	/// @param[in] maxTextureSize Only load mipmaps less or equal to that. Used
	///                           with AnKi textures
	void load(const CString& filename, U32 maxTextureSize = MAX_U32);

private:
	/// [mip][depthFace]
	ResourceVector<Surface> m_surfaces;
	U8 m_mipLevels = 0;
	U8 m_depth = 0;
	DataCompression m_compression = DataCompression::NONE;
	ColorFormat m_colorFormat = ColorFormat::NONE;
	TextureType m_textureType = TextureType::NONE;
};

} // end namespace anki

#endif
