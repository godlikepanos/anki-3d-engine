// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/ImageBinary.h>

namespace anki {

/// An image surface
/// @memberof ImageLoader
class ImageLoaderSurface
{
public:
	U32 m_width;
	U32 m_height;
	DynamicArray<U8, PtrSize> m_data;
};

/// An image volume
/// @memberof ImageLoader
class ImageLoaderVolume
{
public:
	U32 m_width;
	U32 m_height;
	U32 m_depth;
	DynamicArray<U8, PtrSize> m_data;
};

/// Loads bitmaps from regular system files or resource files. Supported formats are .tga and .ankitex.
class ImageLoader
{
public:
	ImageLoader(BaseMemoryPool* pool)
		: m_pool(pool)
	{
		ANKI_ASSERT(pool);
	}

	~ImageLoader()
	{
		destroy();
	}

	ImageBinaryColorFormat getColorFormat() const
	{
		ANKI_ASSERT(m_colorFormat != ImageBinaryColorFormat::kNone);
		return m_colorFormat;
	}

	ImageBinaryDataCompression getCompression() const
	{
		ANKI_ASSERT(m_compression != ImageBinaryDataCompression::kNone);
		return m_compression;
	}

	U32 getMipmapCount() const
	{
		ANKI_ASSERT(m_mipmapCount != 0);
		return m_mipmapCount;
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
		ANKI_ASSERT(m_imageType == ImageBinaryType::k3D);
		return m_depth;
	}

	U32 getLayerCount() const
	{
		ANKI_ASSERT(m_imageType == ImageBinaryType::k2DArray);
		return m_layerCount;
	}

	ImageBinaryType getImageType() const
	{
		ANKI_ASSERT(m_imageType != ImageBinaryType::kNone);
		return m_imageType;
	}

	UVec2 getAstcBlockSize() const
	{
		ANKI_ASSERT(!!(m_compression & ImageBinaryDataCompression::kAstc));
		ANKI_ASSERT(m_astcBlockSize != UVec2(0u));
		return m_astcBlockSize;
	}

	const ImageLoaderSurface& getSurface(U32 level, U32 face, U32 layer) const;

	const ImageLoaderVolume& getVolume(U32 level) const;

	/// Load a resource image file.
	Error load(ResourceFilePtr file, const CString& filename, U32 maxImageSize = kMaxU32);

	/// Load a system image file.
	Error load(const CString& filename, U32 maxImageSize = kMaxU32);

private:
	class FileInterface;
	class RsrcFile;
	class SystemFile;

	BaseMemoryPool* m_pool = nullptr;

	/// [mip][depth or face or layer]. Loader doesn't support cube arrays ATM so face and layer won't be used at the
	/// same time.
	DynamicArray<ImageLoaderSurface> m_surfaces;

	DynamicArray<ImageLoaderVolume> m_volumes;

	U32 m_mipmapCount = 0;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_layerCount = 0;
	UVec2 m_astcBlockSize = UVec2(0u);
	ImageBinaryDataCompression m_compression = ImageBinaryDataCompression::kNone;
	ImageBinaryColorFormat m_colorFormat = ImageBinaryColorFormat::kNone;
	ImageBinaryType m_imageType = ImageBinaryType::kNone;

	void destroy();

	static Error loadUncompressedTga(FileInterface& fs, U32& width, U32& height, U32& bpp,
									 DynamicArray<U8, PtrSize>& data, BaseMemoryPool& pool);

	static Error loadCompressedTga(FileInterface& fs, U32& width, U32& height, U32& bpp,
								   DynamicArray<U8, PtrSize>& data, BaseMemoryPool& pool);

	static Error loadTga(FileInterface& fs, U32& width, U32& height, U32& bpp, DynamicArray<U8, PtrSize>& data,
						 BaseMemoryPool& pool);

	static Error loadStb(Bool isFloat, FileInterface& fs, U32& width, U32& height, DynamicArray<U8, PtrSize>& data,
						 BaseMemoryPool& pool);

	static Error loadAnkiImage(FileInterface& file, U32 maxImageSize, ImageBinaryDataCompression& preferredCompression,
							   DynamicArray<ImageLoaderSurface>& surfaces, DynamicArray<ImageLoaderVolume>& volumes,
							   BaseMemoryPool& pool, U32& width, U32& height, U32& depth, U32& layerCount,
							   U32& mipCount, ImageBinaryType& imageType, ImageBinaryColorFormat& colorFormat,
							   UVec2& astcBlockSize);

	Error loadInternal(FileInterface& file, const CString& filename, U32 maxImageSize);
};

} // end namespace anki
