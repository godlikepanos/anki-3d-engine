// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/resource/ResourceFilesystem.h>

namespace anki
{

/// Texture type
/// @memberof ImageLoader
enum class ImageLoaderTextureType : U32
{
	NONE,
	_2D,
	CUBE,
	_3D,
	_2D_ARRAY
};

/// The acceptable color types of AnKi
/// @memberof ImageLoader
enum class ImageLoaderColorFormat : U32
{
	NONE,
	RGB8, ///< RGB
	RGBA8 ///< RGB plus alpha
};

/// The data compression
/// @memberof ImageLoader
enum class ImageLoaderDataCompression : U32
{
	NONE,
	RAW = 1 << 0,
	S3TC = 1 << 1,
	ETC = 1 << 2
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ImageLoaderDataCompression)

/// An image surface
/// @memberof ImageLoader
class ImageLoaderSurface
{
public:
	U32 m_width;
	U32 m_height;
	DynamicArray<U8> m_data;
};

/// An image volume
/// @memberof ImageLoader
class ImageLoaderVolume
{
public:
	U32 m_width;
	U32 m_height;
	U32 m_depth;
	DynamicArray<U8> m_data;
};

/// Loads bitmaps from regular system files or resource files. Supported formats are .tga and .ankitex.
class ImageLoader
{
public:
	ImageLoader(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~ImageLoader()
	{
		destroy();
	}

	ImageLoaderColorFormat getColorFormat() const
	{
		ANKI_ASSERT(m_colorFormat != ImageLoaderColorFormat::NONE);
		return m_colorFormat;
	}

	ImageLoaderDataCompression getCompression() const
	{
		ANKI_ASSERT(m_compression != ImageLoaderDataCompression::NONE);
		return m_compression;
	}

	U32 getMipmapCount() const
	{
		ANKI_ASSERT(m_mipCount != 0);
		return m_mipCount;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	U32 getDepth() const
	{
		ANKI_ASSERT(m_textureType == ImageLoaderTextureType::_3D);
		return m_depth;
	}

	U32 getLayerCount() const
	{
		ANKI_ASSERT(m_textureType == ImageLoaderTextureType::_2D_ARRAY);
		return m_layerCount;
	}

	ImageLoaderTextureType getTextureType() const
	{
		ANKI_ASSERT(m_textureType != ImageLoaderTextureType::NONE);
		return m_textureType;
	}

	const ImageLoaderSurface& getSurface(U32 level, U32 face, U32 layer) const;

	const ImageLoaderVolume& getVolume(U32 level) const;

	/// Load a resource image file.
	ANKI_USE_RESULT Error load(ResourceFilePtr file, const CString& filename, U32 maxTextureSize = MAX_U32);

	/// Load a system image file.
	ANKI_USE_RESULT Error load(const CString& filename, U32 maxTextureSize = MAX_U32);

private:
	class FileInterface;
	class RsrcFile;
	class SystemFile;

	GenericMemoryPoolAllocator<U8> m_alloc;

	/// [mip][depth or face or layer]. Loader doesn't support cube arrays ATM so face and layer won't be used at the
	/// same time.
	DynamicArray<ImageLoaderSurface> m_surfaces;

	DynamicArray<ImageLoaderVolume> m_volumes;

	U32 m_mipCount = 0;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_layerCount = 0;
	ImageLoaderDataCompression m_compression = ImageLoaderDataCompression::NONE;
	ImageLoaderColorFormat m_colorFormat = ImageLoaderColorFormat::NONE;
	ImageLoaderTextureType m_textureType = ImageLoaderTextureType::NONE;

	void destroy();

	static ANKI_USE_RESULT Error loadUncompressedTga(FileInterface& fs, U32& width, U32& height, U32& bpp,
													 DynamicArray<U8>& data, GenericMemoryPoolAllocator<U8>& alloc);

	static ANKI_USE_RESULT Error loadCompressedTga(FileInterface& fs, U32& width, U32& height, U32& bpp,
												   DynamicArray<U8>& data, GenericMemoryPoolAllocator<U8>& alloc);

	static ANKI_USE_RESULT Error loadTga(FileInterface& fs, U32& width, U32& height, U32& bpp, DynamicArray<U8>& data,
										 GenericMemoryPoolAllocator<U8>& alloc);

	static ANKI_USE_RESULT Error loadStb(FileInterface& fs, U32& width, U32& height, DynamicArray<U8>& data,
										 GenericMemoryPoolAllocator<U8>& alloc);

	static ANKI_USE_RESULT Error
	loadAnkiTexture(FileInterface& file, U32 maxTextureSize, ImageLoaderDataCompression& preferredCompression,
					DynamicArray<ImageLoaderSurface>& surfaces, DynamicArray<ImageLoaderVolume>& volumes,
					GenericMemoryPoolAllocator<U8>& alloc, U32& width, U32& height, U32& depth, U32& layerCount,
					U32& mipCount, ImageLoaderTextureType& textureType, ImageLoaderColorFormat& colorFormat);

	ANKI_USE_RESULT Error loadInternal(FileInterface& file, const CString& filename, U32 maxTextureSize);
};

} // end namespace anki
