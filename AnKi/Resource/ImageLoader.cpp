// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ImageLoader.h>
#include <AnKi/Resource/Stb.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

static PtrSize calcRawTexelSize(const ImageBinaryColorFormat cf)
{
	PtrSize out;
	switch(cf)
	{
	case ImageBinaryColorFormat::kRgb8:
	case ImageBinaryColorFormat::kSrgb8:
		out = 3;
		break;
	case ImageBinaryColorFormat::kRgba8:
	case ImageBinaryColorFormat::kSrgba8:
		out = 4;
		break;
	case ImageBinaryColorFormat::kRgbFloat:
		out = 3 * sizeof(F32);
		break;
	case ImageBinaryColorFormat::kRgbaFloat:
		out = 4 * sizeof(F32);
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

static PtrSize calcS3tcBlockSize(const ImageBinaryColorFormat cf)
{
	PtrSize out;
	switch(cf)
	{
	case ImageBinaryColorFormat::kRgb8:
	case ImageBinaryColorFormat::kSrgb8:
		out = 8;
		break;
	case ImageBinaryColorFormat::kRgba8:
	case ImageBinaryColorFormat::kSrgba8:
		out = 16;
		break;
	case ImageBinaryColorFormat::kRgbFloat:
		out = 16;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

// Get the size in bytes of a single surface
static PtrSize calcSurfaceSize(const U32 width32, const U32 height32, const ImageBinaryDataCompression comp, const ImageBinaryColorFormat cf,
							   UVec2 astcBlockSize)
{
	const PtrSize width = width32;
	const PtrSize height = height32;
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4);

	switch(comp)
	{
	case ImageBinaryDataCompression::kRaw:
		out = width * height * calcRawTexelSize(cf);
		break;
	case ImageBinaryDataCompression::kS3tc:
		out = (width / 4) * (height / 4) * calcS3tcBlockSize(cf);
		break;
	case ImageBinaryDataCompression::kEtc:
		out = (width / 4) * (height / 4) * 8;
		break;
	case ImageBinaryDataCompression::kAstc:
		out = (width / astcBlockSize.x) * (height / astcBlockSize.y) * 16;
		break;
	default:
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out > 0);

	return out;
}

// Get the size in bytes of a single volume
static PtrSize calcVolumeSize(const U width, const U height, const U depth, const ImageBinaryDataCompression comp, const ImageBinaryColorFormat cf)
{
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4 || depth >= 4);

	switch(comp)
	{
	case ImageBinaryDataCompression::kRaw:
		out = width * height * depth * calcRawTexelSize(cf);
		break;
	default:
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out > 0);

	return out;
}

// Calculate the size of a compressed or uncompressed color data
static PtrSize calcSizeOfSegment(const ImageBinaryHeader& header, ImageBinaryDataCompression comp)
{
	PtrSize out = 0;
	U32 width = header.m_width;
	U32 height = header.m_height;
	U32 mips = header.m_mipmapCount;
	ANKI_ASSERT(mips > 0);

	if(header.m_type != ImageBinaryType::k3D)
	{
		U32 surfCountPerMip = 0;

		switch(header.m_type)
		{
		case ImageBinaryType::k2D:
			surfCountPerMip = 1;
			break;
		case ImageBinaryType::kCube:
			surfCountPerMip = 6;
			break;
		case ImageBinaryType::k2DArray:
			surfCountPerMip = header.m_depthOrLayerCount;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}

		while(mips-- != 0)
		{
			out +=
				calcSurfaceSize(width, height, comp, header.m_colorFormat, UVec2(header.m_astcBlockSizeX, header.m_astcBlockSizeY)) * surfCountPerMip;

			width /= 2;
			height /= 2;
		}
	}
	else
	{
		U depth = header.m_depthOrLayerCount;

		while(mips-- != 0)
		{
			out += calcVolumeSize(width, height, depth, comp, header.m_colorFormat);

			width /= 2;
			height /= 2;
			depth /= 2;
		}
	}

	return out;
}

class ImageLoader::FileInterface
{
public:
	virtual Error read(void* buff, PtrSize size) = 0;

	virtual Error seek(PtrSize offset, FileSeekOrigin origin) = 0;

	virtual PtrSize getSize() const
	{
		ANKI_ASSERT(!"Not Implemented");
		return kMaxPtrSize;
	}
};

class ImageLoader::RsrcFile : public FileInterface
{
public:
	ResourceFile* m_rfile;

	RsrcFile(ResourceFile* file)
		: m_rfile(file)
	{
	}

	Error read(void* buff, PtrSize size) final
	{
		return m_rfile->read(buff, size);
	}

	Error seek(PtrSize offset, FileSeekOrigin origin) final
	{
		return m_rfile->seek(offset, origin);
	}

	PtrSize getSize() const final
	{
		return m_rfile->getSize();
	}
};

class ImageLoader::SystemFile : public FileInterface
{
public:
	File& m_file;

	SystemFile(File& file)
		: m_file(file)
	{
	}

	Error read(void* buff, PtrSize size) final
	{
		return m_file.read(buff, size);
	}

	Error seek(PtrSize offset, FileSeekOrigin origin) final
	{
		return m_file.seek(offset, origin);
	}

	PtrSize getSize() const final
	{
		return m_file.getSize();
	}
};

Error ImageLoader::loadAnkiImageHeader(FileInterface& file)
{
	//
	// Read and check the header
	//
	ANKI_CHECK(file.read(&m_ankiHeader, sizeof(ImageBinaryHeader)));
	const ImageBinaryHeader& header = m_ankiHeader;

	if(std::memcmp(&header.m_magic[0], kImageMagic, CString(kImageMagic).getLength()) != 0)
	{
		ANKI_RESOURCE_LOGE("Wrong magic word");
		return Error::kUserData;
	}

	if(header.m_width == 0 || !isPowerOfTwo(header.m_width) || header.m_height == 0 || !isPowerOfTwo(header.m_height))
	{
		ANKI_RESOURCE_LOGE("Incorrect width/height value");
		return Error::kUserData;
	}

	if(header.m_depthOrLayerCount < 1 || header.m_depthOrLayerCount > 4096)
	{
		ANKI_RESOURCE_LOGE("Zero or too big depth or layerCount");
		return Error::kUserData;
	}

	if(header.m_type < ImageBinaryType::k2D || header.m_type > ImageBinaryType::k2DArray)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: image type");
		return Error::kUserData;
	}

	if(header.m_colorFormat < ImageBinaryColorFormat::kFirst || header.m_colorFormat > ImageBinaryColorFormat::kLast)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: color format");
		return Error::kUserData;
	}

	if(!!(header.m_compressionMask & ImageBinaryDataCompression::kAstc))
	{
		if((header.m_astcBlockSizeX != 8 && header.m_astcBlockSizeX != 4) || (header.m_astcBlockSizeY != 8 && header.m_astcBlockSizeY != 4))
		{
			ANKI_RESOURCE_LOGE("Incorrect header: ASTC block size");
			return Error::kUserData;
		}
	}

	if(header.m_type != ImageBinaryType::k3D
	   && (header.m_mipmapCount > computeMaxMipmapCount2d(header.m_width, header.m_height, 4) || header.m_mipmapCount == 0))
	{
		ANKI_RESOURCE_LOGE("Incorrect mipmap count: %u", header.m_mipmapCount);
		return Error::kUserData;
	}

	if(header.m_type == ImageBinaryType::k3D
	   && (header.m_mipmapCount > computeMaxMipmapCount3d(header.m_width, header.m_height, header.m_depthOrLayerCount, 4)
		   || header.m_mipmapCount == 0))
	{
		ANKI_RESOURCE_LOGE("Incorrect mipmap count: %u", header.m_mipmapCount);
		return Error::kUserData;
	}

#if ANKI_PLATFORM_MOBILE
	m_compression = ImageBinaryDataCompression::kAstc;
#else
	m_compression = ImageBinaryDataCompression::kS3tc;
#endif

	if(!(header.m_compressionMask & m_compression))
	{
		// Fallback
		m_compression = ImageBinaryDataCompression::kRaw;

		if(!(header.m_compressionMask & m_compression))
		{
			ANKI_RESOURCE_LOGE("File does not contain raw compression");
			return Error::kUserData;
		}
	}

	if(header.m_isNormal != 0 && header.m_isNormal != 1)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: normal");
		return Error::kUserData;
	}

	// Compute the actual width, height, depth and mipmap
	if(header.m_type != ImageBinaryType::k3D)
	{
		U32 mipWidth = header.m_width;
		U32 mipHeight = header.m_height;
		for(U32 mip = 0; mip < header.m_mipmapCount; mip++)
		{
			if(max(mipWidth, mipHeight) <= m_maxSurfaceOrVolumeDimension || mip == header.m_mipmapCount - 1)
			{
				m_width = mipWidth;
				m_height = mipHeight;
				m_mipmapCount = header.m_mipmapCount - mip;
				break;
			}

			mipWidth /= 2;
			mipHeight /= 2;
		}
	}
	else
	{
		U32 mipWidth = header.m_width;
		U32 mipHeight = header.m_height;
		U32 mipDepth = header.m_depthOrLayerCount;

		for(U32 mip = 0; mip < header.m_mipmapCount; mip++)
		{
			if(max(max(mipWidth, mipHeight), mipDepth) <= m_maxSurfaceOrVolumeDimension || mip == header.m_mipmapCount - 1)
			{
				m_width = mipWidth;
				m_height = mipHeight;
				m_depth = mipDepth;
				m_mipmapCount = header.m_mipmapCount - mip;
				break;
			}

			mipWidth /= 2;
			mipHeight /= 2;
			mipDepth /= 2;
		}
	}

	m_avgColor = Vec4(header.m_averageColor);

	// Set a few things
	m_colorFormat = header.m_colorFormat;
	m_imageType = header.m_type;
	m_astcBlockSize = UVec2(header.m_astcBlockSizeX, header.m_astcBlockSizeY);

	switch(header.m_type)
	{
	case ImageBinaryType::k2D:
		m_depth = 1;
		m_layerCount = 1;
		break;
	case ImageBinaryType::kCube:
		m_depth = 1;
		m_layerCount = 1;
		break;
	case ImageBinaryType::k3D:
		m_layerCount = 1;
		break;
	case ImageBinaryType::k2DArray:
		m_depth = 1;
		m_layerCount = header.m_depthOrLayerCount;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return Error::kNone;
}

Error ImageLoader::loadStb(Bool isFloat, FileInterface& fs, U32& width, U32& height,
						   DynamicArray<U8, MemoryPoolPtrWrapper<BaseMemoryPool>, PtrSize>& data)
{
	// Read the file
	DynamicArray<U8, MemoryPoolPtrWrapper<BaseMemoryPool>, PtrSize> fileData(data.getMemoryPool());
	const PtrSize fileSize = fs.getSize();
	fileData.resize(fileSize);
	ANKI_CHECK(fs.read(&fileData[0], fileSize));

	// Use STB to read the image
	int stbw, stbh, comp;
	// stbi_set_flip_vertically_on_load_thread(true);
	U8* stbdata;
	if(isFloat)
	{
		stbdata = reinterpret_cast<U8*>(stbi_loadf_from_memory(&fileData[0], I32(fileSize), &stbw, &stbh, &comp, 4));
	}
	else
	{
		stbdata = reinterpret_cast<U8*>(stbi_load_from_memory(&fileData[0], I32(fileSize), &stbw, &stbh, &comp, 4));
	}

	if(!stbdata)
	{
		ANKI_RESOURCE_LOGE("STB failed to read image");
		return Error::kFunctionFailed;
	}

	// Store it
	width = U32(stbw);
	height = U32(stbh);
	const U32 componentSize = (isFloat) ? sizeof(F32) : sizeof(U8);
	data.resize(width * height * 4 * componentSize);
	memcpy(&data[0], stbdata, data.getSize());

	// Cleanup
	stbi_image_free(stbdata);

	return Error::kNone;
}

Error ImageLoader::loadHeaderFromResourceFile(CString filename, U32 maxSurfaceOrVolumeDimension)
{
	ANKI_CHECK(ResourceFilesystem::getSingleton().openFile(filename, m_rsrcFile));
	RsrcFile file(m_rsrcFile.get());
	m_maxSurfaceOrVolumeDimension = maxSurfaceOrVolumeDimension;

	const Error err = loadHeaderInternal(file, filename);
	if(err)
	{
		ANKI_RESOURCE_LOGE("Failed to read image: %s", filename.cstr());
	}

	return err;
}

Error ImageLoader::loadHeaderFromSystemFile(CString filename, U32 maxSurfaceOrVolumeDimension)
{
	ANKI_CHECK(m_systemFile.open(filename, FileOpenFlag::kRead | FileOpenFlag::kBinary));
	SystemFile file(m_systemFile);
	m_maxSurfaceOrVolumeDimension = maxSurfaceOrVolumeDimension;

	const Error err = loadHeaderInternal(file, filename);
	if(err)
	{
		ANKI_RESOURCE_LOGE("Failed to read image: %s", filename.cstr());
	}

	return err;
}

Error ImageLoader::loadHeaderInternal(FileInterface& file, const CString& filename)
{
	// get the extension
	const String ext = getFileExtension(filename);

	if(ext.isEmpty())
	{
		ANKI_RESOURCE_LOGE("Failed to get filename extension");
		return Error::kUserData;
	}

	// load from this extension
	m_imageType = ImageBinaryType::k2D;
	m_compression = ImageBinaryDataCompression::kRaw;

	if(ext == "ankitex")
	{
		ANKI_CHECK(loadAnkiImageHeader(file));
		createSurfaceOrVolumeFileOffsets();
	}
	else if(ext == "png" || ext == "jpg" || ext == "tga")
	{
		m_mipmapCount = 1;
		m_depth = 1;
		m_layerCount = 1;
		m_colorFormat = ImageBinaryColorFormat::kRgba8;

		ANKI_CHECK(loadStb(false, file, m_width, m_height, m_stbImageData));
	}
	else if(ext == "hdr")
	{
		m_mipmapCount = 1;
		m_depth = 1;
		m_layerCount = 1;
		m_colorFormat = ImageBinaryColorFormat::kRgbaFloat;

		ANKI_CHECK(loadStb(true, file, m_width, m_height, m_stbImageData));
	}
	else
	{
		ANKI_RESOURCE_LOGE("Unsupported extension: %s", &ext[0]);
		return Error::kUserData;
	}

	return Error::kNone;
}

void ImageLoader::createSurfaceOrVolumeFileOffsets()
{
	const ImageBinaryHeader& header = m_ankiHeader;
	const U32 faceCount = (header.m_type == ImageBinaryType::kCube) ? 6 : 1;

	PtrSize seekSize = sizeof(ImageBinaryHeader); // It's the bytes to skip reading from the beginning of the file
	if(m_compression == ImageBinaryDataCompression::kRaw)
	{
		// Do nothing
	}
	else if(m_compression == ImageBinaryDataCompression::kS3tc)
	{
		if(!!(header.m_compressionMask & ImageBinaryDataCompression::kRaw))
		{
			// If raw compression is present then skip it
			seekSize += calcSizeOfSegment(header, ImageBinaryDataCompression::kRaw);
		}
	}
	else if(m_compression == ImageBinaryDataCompression::kEtc)
	{
		if(!!(header.m_compressionMask & ImageBinaryDataCompression::kRaw))
		{
			// If raw compression is present then skip it
			seekSize += calcSizeOfSegment(header, ImageBinaryDataCompression::kRaw);
		}

		if(!!(header.m_compressionMask & ImageBinaryDataCompression::kS3tc))
		{
			// If s3tc compression is present then skip it
			seekSize += calcSizeOfSegment(header, ImageBinaryDataCompression::kS3tc);
		}
	}
	else if(m_compression == ImageBinaryDataCompression::kAstc)
	{
		if(!!(header.m_compressionMask & ImageBinaryDataCompression::kRaw))
		{
			// If raw compression is present then skip it
			seekSize += calcSizeOfSegment(header, ImageBinaryDataCompression::kRaw);
		}

		if(!!(header.m_compressionMask & ImageBinaryDataCompression::kS3tc))
		{
			// If s3tc compression is present then skip it
			seekSize += calcSizeOfSegment(header, ImageBinaryDataCompression::kS3tc);
		}

		if(!!(header.m_compressionMask & ImageBinaryDataCompression::kEtc))
		{
			// If ETC compression is present then skip it
			seekSize += calcSizeOfSegment(header, ImageBinaryDataCompression::kEtc);
		}
	}

	if(header.m_type != ImageBinaryType::k3D)
	{
		U32 mipWidth = header.m_width;
		U32 mipHeight = header.m_height;
		for(U32 mip = 0; mip < header.m_mipmapCount; mip++)
		{
			for(U32 l = 0; l < m_layerCount; l++)
			{
				for(U32 f = 0; f < faceCount; ++f)
				{
					const PtrSize dataSize = calcSurfaceSize(mipWidth, mipHeight, m_compression, header.m_colorFormat,
															 UVec2(header.m_astcBlockSizeX, header.m_astcBlockSizeY));

					if(max(mipWidth, mipHeight) <= m_maxSurfaceOrVolumeDimension || mip == header.m_mipmapCount - 1)
					{
						m_surfaceOrVolumeFileOffsets.emplaceBack(FileOffsetAndSize{seekSize, dataSize});
					}

					seekSize += dataSize;
				}
			}

			mipWidth /= 2;
			mipHeight /= 2;
		}
	}
	else
	{
		U32 mipWidth = header.m_width;
		U32 mipHeight = header.m_height;
		U32 mipDepth = header.m_depthOrLayerCount;

		for(U32 mip = 0; mip < header.m_mipmapCount; mip++)
		{
			const PtrSize dataSize = calcVolumeSize(mipWidth, mipHeight, mipDepth, m_compression, header.m_colorFormat);

			if(max(max(mipWidth, mipHeight), mipDepth) <= m_maxSurfaceOrVolumeDimension || mip == header.m_mipmapCount - 1)
			{
				m_surfaceOrVolumeFileOffsets.emplaceBack(FileOffsetAndSize{seekSize, dataSize});
			}

			seekSize += dataSize;

			mipWidth /= 2;
			mipHeight /= 2;
			mipDepth /= 2;
		}
	}

	[[maybe_unused]] const U32 expectedCount = (header.m_type == ImageBinaryType::k3D) ? m_mipmapCount : m_mipmapCount * m_layerCount * faceCount;
	ANKI_ASSERT(m_surfaceOrVolumeFileOffsets.getSize() == expectedCount);
}

Error ImageLoader::loadSurfaceOrVolume(U32 level, U32 face, U32 layer, WeakArray<U8> data)
{
	ANKI_ASSERT(m_imageType != ImageBinaryType::kNone);
	ANKI_ASSERT(level < m_mipmapCount);
	[[maybe_unused]] const U32 faceCount = (m_imageType == ImageBinaryType::kCube) ? 6 : 1;
	ANKI_ASSERT(face < faceCount);
	ANKI_ASSERT(layer < m_layerCount);

	if(m_stbImageData.getSize())
	{
		// STB already loaded the surface, early exit
		ANKI_ASSERT(m_stbImageData.getSizeInBytes() == data.getSizeInBytes());
		memcpy(data.getBegin(), m_stbImageData.getBegin(), data.getSizeInBytes());
		return Error::kNone;
	}

	// It's an ankitex

	RsrcFile file1(m_rsrcFile.tryGet());
	SystemFile file2(m_systemFile);
	FileInterface& file = (m_rsrcFile) ? static_cast<FileInterface&>(file1) : static_cast<FileInterface&>(file2);

	U32 idx = 0;
	switch(m_imageType)
	{
	case ImageBinaryType::k2D:
		idx = level;
		break;
	case ImageBinaryType::kCube:
		idx = level * 6 + face;
		break;
	case ImageBinaryType::k3D:
		ANKI_ASSERT(face == 0 && layer == 0);
		idx = level;
		break;
	case ImageBinaryType::k2DArray:
		idx = level * m_layerCount + layer;
		break;
	default:
		ANKI_ASSERT(0);
	}

	const FileOffsetAndSize& fileOffset = m_surfaceOrVolumeFileOffsets[idx];

	ANKI_ASSERT(fileOffset.m_dataSize == data.getSizeInBytes());
	ANKI_CHECK(file.seek(fileOffset.m_offset, FileSeekOrigin::kBeginning));
	ANKI_CHECK(file.read(data.getBegin(), fileOffset.m_dataSize));

	return Error::kNone;
}

} // end namespace anki
