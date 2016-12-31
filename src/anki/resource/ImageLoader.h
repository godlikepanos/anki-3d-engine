// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/util/Functions.h>
#include <anki/util/Enum.h>

namespace anki
{

/// Image loader.
class ImageLoader
{
public:
	/// Texture type
	enum class TextureType : U32
	{
		NONE,
		_2D,
		CUBE,
		_3D,
		_2D_ARRAY
	};

	/// The acceptable color types of AnKi
	enum class ColorFormat : U32
	{
		NONE,
		RGB8, ///< RGB
		RGBA8 ///< RGB plus alpha
	};

	/// The data compression
	enum class DataCompression : U32
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
		DynamicArray<U8> m_data;
	};

	class Volume
	{
	public:
		U32 m_width;
		U32 m_height;
		U32 m_depth;
		U32 m_mipLevel;
		DynamicArray<U8> m_data;
	};

	ImageLoader(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

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

	U getWidth() const
	{
		return m_width;
	}

	U getHeight() const
	{
		return m_height;
	}

	U getDepth() const
	{
		ANKI_ASSERT(m_textureType == TextureType::_3D);
		return m_depth;
	}

	U getLayerCount() const
	{
		ANKI_ASSERT(m_textureType == TextureType::_2D_ARRAY);
		return m_layerCount;
	}

	TextureType getTextureType() const
	{
		ANKI_ASSERT(m_textureType != TextureType::NONE);
		return m_textureType;
	}

	const Surface& getSurface(U level, U face, U layer) const;

	const Volume& getVolume(U level) const;

	GenericMemoryPoolAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	/// Load an image file.
	ANKI_USE_RESULT Error load(ResourceFilePtr file, const CString& filename, U32 maxTextureSize = MAX_U32);

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	Atomic<I32> m_refcount = {0};

	/// [mip][depth or face or layer]. Loader doesn't support cube arrays ATM so face and layer won't be used at the
	/// same time.
	DynamicArray<Surface> m_surfaces;

	DynamicArray<Volume> m_volumes;

	U8 m_mipLevels = 0;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_layerCount = 0;
	DataCompression m_compression = DataCompression::NONE;
	ColorFormat m_colorFormat = ColorFormat::NONE;
	TextureType m_textureType = TextureType::NONE;

	void destroy();
};

} // end namespace anki
