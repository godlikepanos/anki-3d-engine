// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/ImageBinary.h>

namespace anki {

// Loads bitmaps from regular system files or resource files. Supported formats are .tga, .png, .jpg, .hdr and .ankitex.
class ImageLoader
{
public:
	ImageLoader(BaseMemoryPool* pool)
		: m_stbImageData(pool)
		, m_surfaceOrVolumeFileOffsets(pool)
	{
		ANKI_ASSERT(pool);
	}

	~ImageLoader() = default;

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
		ANKI_ASSERT(m_width > 0);
		return m_width;
	}

	U32 getHeight() const
	{
		ANKI_ASSERT(m_height > 0);
		return m_height;
	}

	U32 getDepth() const
	{
		ANKI_ASSERT(m_imageType == ImageBinaryType::k3D && m_depth > 0);
		return m_depth;
	}

	U32 getLayerCount() const
	{
		ANKI_ASSERT(m_imageType == ImageBinaryType::k2DArray && m_layerCount > 0);
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

	Vec4 getAverageColor() const
	{
		return m_avgColor;
	}

	// Step 1: Just load the header of the image file and use the ResourceFilesystem to do so
	// Not thread-safe
	Error loadHeaderFromResourceFile(CString filename, U32 maxSurfaceOrVolumeDimension = kMaxU32);

	// Step 1: See loadHeaderFromResourceFile(), same thing but opens a system file
	// Not thread-safe
	Error loadHeaderFromSystemFile(CString filename, U32 maxSurfaceOrVolumeDimension = kMaxU32);

	// Step 2: Fetch a surface or a volume from the file
	// Not thread-safe
	Error loadSurfaceOrVolume(U32 level, U32 face, U32 layer, WeakArray<U8> data);

private:
	class FileInterface;
	class RsrcFile;
	class SystemFile;

	class FileOffsetAndSize
	{
	public:
		PtrSize m_offset;
		PtrSize m_dataSize;
	};

	Vec4 m_avgColor = Vec4(0.0f);

	U32 m_mipmapCount = 0;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_layerCount = 0;
	UVec2 m_astcBlockSize = UVec2(0u);
	ImageBinaryDataCompression m_compression = ImageBinaryDataCompression::kNone;
	ImageBinaryColorFormat m_colorFormat = ImageBinaryColorFormat::kNone;
	ImageBinaryType m_imageType = ImageBinaryType::kNone;

	ImageBinaryHeader m_ankiHeader;

	DynamicArray<U8, MemoryPoolPtrWrapper<BaseMemoryPool>, PtrSize> m_stbImageData; // Populated when loading goes through STB

	// File offsets in the ankitex binary. It's [mip][depth or face or layer]. Loader doesn't support cube arrays ATM so face and layer won't be
	// used at the same time.
	DynamicArray<FileOffsetAndSize, MemoryPoolPtrWrapper<BaseMemoryPool>> m_surfaceOrVolumeFileOffsets;

	ResourceFilePtr m_rsrcFile;
	File m_systemFile;

	U32 m_maxSurfaceOrVolumeDimension = kMaxU32;

	static Error loadStb(Bool isFloat, FileInterface& fs, U32& width, U32& height,
						 DynamicArray<U8, MemoryPoolPtrWrapper<BaseMemoryPool>, PtrSize>& data);

	Error loadHeaderInternal(FileInterface& file, const CString& filename);

	Error loadAnkiImageHeader(FileInterface& file);

	void createSurfaceOrVolumeFileOffsets();
};

} // end namespace anki
