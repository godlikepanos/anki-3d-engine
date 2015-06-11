// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/resource/Common.h"
#include "anki/resource/ResourceFilesystem.h"
#include "anki/util/Functions.h"
#include "anki/util/Enum.h"

namespace anki {

/// Image loader.
/// Used in Texture::load. Supported types: TGA and an AnKi specific format.
class ImageLoader
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
		U32 m_width;
		U32 m_height;
		U32 m_mipLevel;
		DArray<U8> m_data;
	};

	ImageLoader(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{}

	~ImageLoader()
	{
		destroy();
	}

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

	GenericMemoryPoolAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	/// Load an image file.
	ANKI_USE_RESULT Error load(
		ResourceFilePtr file,
		const CString& filename,
		U32 maxTextureSize = MAX_U32);

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	Atomic<I32> m_refcount = {0};
	/// [mip][depthFace]
	DArray<Surface> m_surfaces;
	U8 m_mipLevels = 0;
	U8 m_depth = 0;
	DataCompression m_compression = DataCompression::NONE;
	ColorFormat m_colorFormat = ColorFormat::NONE;
	TextureType m_textureType = TextureType::NONE;

	void destroy();
};

} // end namespace anki

