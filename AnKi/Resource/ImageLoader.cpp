// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ImageLoader.h>
#include <AnKi/Resource/Stb.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

static const U8 tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const U8 tgaHeaderCompressed[12] = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static PtrSize calcRawTexelSize(const ImageBinaryColorFormat cf)
{
	PtrSize out;
	switch(cf)
	{
	case ImageBinaryColorFormat::RGB8:
		out = 3;
		break;
	case ImageBinaryColorFormat::RGBA8:
		out = 4;
		break;
	case ImageBinaryColorFormat::SRGB8:
		out = 3;
		break;
	case ImageBinaryColorFormat::RGBF32:
		out = 3 * sizeof(F32);
		break;
	case ImageBinaryColorFormat::RGBAF32:
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
	case ImageBinaryColorFormat::RGB8:
		out = 8;
		break;
	case ImageBinaryColorFormat::RGBA8:
		out = 16;
		break;
	case ImageBinaryColorFormat::SRGB8:
		out = 8;
		break;
	case ImageBinaryColorFormat::RGBF32:
		out = 16;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

/// Get the size in bytes of a single surface
static PtrSize calcSurfaceSize(const U32 width32, const U32 height32, const ImageBinaryDataCompression comp,
							   const ImageBinaryColorFormat cf, UVec2 astcBlockSize)
{
	const PtrSize width = width32;
	const PtrSize height = height32;
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4);

	switch(comp)
	{
	case ImageBinaryDataCompression::RAW:
		out = width * height * calcRawTexelSize(cf);
		break;
	case ImageBinaryDataCompression::S3TC:
		out = (width / 4) * (height / 4) * calcS3tcBlockSize(cf);
		break;
	case ImageBinaryDataCompression::ETC:
		out = (width / 4) * (height / 4) * 8;
		break;
	case ImageBinaryDataCompression::ASTC:
		out = (width / astcBlockSize.x()) * (height / astcBlockSize.y()) * 16;
		break;
	default:
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out > 0);

	return out;
}

/// Get the size in bytes of a single volume
static PtrSize calcVolumeSize(const U width, const U height, const U depth, const ImageBinaryDataCompression comp,
							  const ImageBinaryColorFormat cf)
{
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4 || depth >= 4);

	switch(comp)
	{
	case ImageBinaryDataCompression::RAW:
		out = width * height * depth * calcRawTexelSize(cf);
		break;
	default:
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out > 0);

	return out;
}

/// Calculate the size of a compressed or uncomressed color data
static PtrSize calcSizeOfSegment(const ImageBinaryHeader& header, ImageBinaryDataCompression comp)
{
	PtrSize out = 0;
	U32 width = header.m_width;
	U32 height = header.m_height;
	U32 mips = header.m_mipmapCount;
	ANKI_ASSERT(mips > 0);

	if(header.m_type != ImageBinaryType::_3D)
	{
		U32 surfCountPerMip = 0;

		switch(header.m_type)
		{
		case ImageBinaryType::_2D:
			surfCountPerMip = 1;
			break;
		case ImageBinaryType::CUBE:
			surfCountPerMip = 6;
			break;
		case ImageBinaryType::_2D_ARRAY:
			surfCountPerMip = header.m_depthOrLayerCount;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}

		while(mips-- != 0)
		{
			out += calcSurfaceSize(width, height, comp, header.m_colorFormat,
								   UVec2(header.m_astcBlockSizeX, header.m_astcBlockSizeY))
				   * surfCountPerMip;

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
	virtual ANKI_USE_RESULT Error read(void* buff, PtrSize size) = 0;

	virtual ANKI_USE_RESULT Error seek(PtrSize offset, FileSeekOrigin origin) = 0;

	virtual PtrSize getSize() const
	{
		ANKI_ASSERT(!"Not Implemented");
		return MAX_PTR_SIZE;
	}
};

class ImageLoader::RsrcFile : public FileInterface
{
public:
	ResourceFilePtr m_rfile;

	ANKI_USE_RESULT Error read(void* buff, PtrSize size) final
	{
		return m_rfile->read(buff, size);
	}

	ANKI_USE_RESULT Error seek(PtrSize offset, FileSeekOrigin origin) final
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
	File m_file;

	ANKI_USE_RESULT Error read(void* buff, PtrSize size) final
	{
		return m_file.read(buff, size);
	}

	ANKI_USE_RESULT Error seek(PtrSize offset, FileSeekOrigin origin) final
	{
		return m_file.seek(offset, origin);
	}

	PtrSize getSize() const final
	{
		return m_file.getSize();
	}
};

Error ImageLoader::loadUncompressedTga(FileInterface& fs, U32& width, U32& height, U32& bpp,
									   DynamicArray<U8, PtrSize>& data, GenericMemoryPoolAllocator<U8>& alloc)
{
	Array<U8, 6> header6;

	// Read the info from header
	ANKI_CHECK(fs.read(&header6[0], sizeof(header6)));

	width = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width == 0) || (height == 0) || ((bpp != 24) && (bpp != 32)))
	{
		ANKI_RESOURCE_LOGE("Invalid image information");
		return Error::USER_DATA;
	}

	// Read the data
	const PtrSize bytesPerPxl = (bpp / 8);
	const PtrSize imageSize = bytesPerPxl * width * height;
	data.create(alloc, imageSize);

	ANKI_CHECK(fs.read(reinterpret_cast<char*>(&data[0]), imageSize));

	// Swap red with blue
	for(PtrSize i = 0; i < imageSize; i += bytesPerPxl)
	{
		const U8 temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	return Error::NONE;
}

Error ImageLoader::loadCompressedTga(FileInterface& fs, U32& width, U32& height, U32& bpp,
									 DynamicArray<U8, PtrSize>& data, GenericMemoryPoolAllocator<U8>& alloc)
{
	Array<U8, 6> header6;
	ANKI_CHECK(fs.read(&header6[0], sizeof(header6)));

	width = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32)))
	{
		ANKI_RESOURCE_LOGE("Invalid image information");
		return Error::USER_DATA;
	}

	const PtrSize bytesPerPxl = (bpp / 8);
	const PtrSize imageSize = bytesPerPxl * width * height;
	data.create(alloc, imageSize);

	const PtrSize pixelCount = height * width;
	PtrSize currentPixel = 0;
	PtrSize currentByte = 0;
	Array<U8, 4> colorbuffer;

	do
	{
		U8 chunkheader = 0;

		ANKI_CHECK(fs.read(&chunkheader, sizeof(U8)));

		if(chunkheader < 128)
		{
			chunkheader++;
			for(U8 counter = 0; counter < chunkheader; counter++)
			{
				ANKI_CHECK(fs.read(&colorbuffer[0], bytesPerPxl));

				data[currentByte] = colorbuffer[2];
				data[currentByte + 1] = colorbuffer[1];
				data[currentByte + 2] = colorbuffer[0];

				if(bytesPerPxl == 4)
				{
					data[currentByte + 3] = colorbuffer[3];
				}

				currentByte += bytesPerPxl;
				currentPixel++;

				if(currentPixel > pixelCount)
				{
					ANKI_RESOURCE_LOGE("Too many pixels read");
					return Error::USER_DATA;
				}
			}
		}
		else
		{
			chunkheader = U8(chunkheader - 127);
			ANKI_CHECK(fs.read(&colorbuffer[0], bytesPerPxl));

			for(U8 counter = 0; counter < chunkheader; counter++)
			{
				data[currentByte] = colorbuffer[2];
				data[currentByte + 1] = colorbuffer[1];
				data[currentByte + 2] = colorbuffer[0];

				if(bytesPerPxl == 4)
				{
					data[currentByte + 3] = colorbuffer[3];
				}

				currentByte += bytesPerPxl;
				currentPixel++;

				if(currentPixel > pixelCount)
				{
					ANKI_RESOURCE_LOGE("Too many pixels read");
					data.destroy(alloc);
					return Error::USER_DATA;
				}
			}
		}
	} while(currentPixel < pixelCount);

	return Error::NONE;
}

Error ImageLoader::loadTga(FileInterface& fs, U32& width, U32& height, U32& bpp, DynamicArray<U8, PtrSize>& data,
						   GenericMemoryPoolAllocator<U8>& alloc)
{
	Array<Char, 12> myTgaHeader;

	ANKI_CHECK(fs.read(&myTgaHeader[0], sizeof(myTgaHeader)));

	if(memcmp(tgaHeaderUncompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		ANKI_CHECK(loadUncompressedTga(fs, width, height, bpp, data, alloc));
	}
	else if(std::memcmp(tgaHeaderCompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		ANKI_CHECK(loadCompressedTga(fs, width, height, bpp, data, alloc));
	}
	else
	{
		ANKI_RESOURCE_LOGE("Invalid image header");
		return Error::USER_DATA;
	}

	if(bpp != 32 && bpp != 24)
	{
		ANKI_RESOURCE_LOGE("Invalid bpp");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error ImageLoader::loadAnkiImage(FileInterface& file, U32 maxImageSize,
								 ImageBinaryDataCompression& preferredCompression,
								 DynamicArray<ImageLoaderSurface>& surfaces, DynamicArray<ImageLoaderVolume>& volumes,
								 GenericMemoryPoolAllocator<U8>& alloc, U32& width, U32& height, U32& depth,
								 U32& layerCount, U32& mipCount, ImageBinaryType& imageType,
								 ImageBinaryColorFormat& colorFormat, UVec2& astcBlockSize)
{
	//
	// Read and check the header
	//
	ImageBinaryHeader header;
	ANKI_CHECK(file.read(&header, sizeof(ImageBinaryHeader)));

	if(std::memcmp(&header.m_magic[0], IMAGE_MAGIC, sizeof(IMAGE_MAGIC) - 1) != 0)
	{
		ANKI_RESOURCE_LOGE("Wrong magic word");
		return Error::USER_DATA;
	}

	if(header.m_width == 0 || !isPowerOfTwo(header.m_width) || header.m_width > 4096 || header.m_height == 0
	   || !isPowerOfTwo(header.m_height) || header.m_height > 4096)
	{
		ANKI_RESOURCE_LOGE("Incorrect width/height value");
		return Error::USER_DATA;
	}

	if(header.m_depthOrLayerCount < 1 || header.m_depthOrLayerCount > 4096)
	{
		ANKI_RESOURCE_LOGE("Zero or too big depth or layerCount");
		return Error::USER_DATA;
	}

	if(header.m_type < ImageBinaryType::_2D || header.m_type > ImageBinaryType::_2D_ARRAY)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: image type");
		return Error::USER_DATA;
	}

	if(header.m_colorFormat < ImageBinaryColorFormat::RGB8 || header.m_colorFormat > ImageBinaryColorFormat::RGBAF32)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: color format");
		return Error::USER_DATA;
	}

	if(!!(header.m_compressionMask & ImageBinaryDataCompression::ASTC))
	{
		if((header.m_astcBlockSizeX != 8 && header.m_astcBlockSizeX != 4)
		   || (header.m_astcBlockSizeY != 8 && header.m_astcBlockSizeY != 4))
		{
			ANKI_RESOURCE_LOGE("Incorrect header: ASTC block size");
			return Error::USER_DATA;
		}
	}

	if(!(header.m_compressionMask & preferredCompression))
	{
		// Fallback
		preferredCompression = ImageBinaryDataCompression::RAW;

		if(!(header.m_compressionMask & preferredCompression))
		{
			ANKI_RESOURCE_LOGE("File does not contain raw compression");
			return Error::USER_DATA;
		}
	}

	if(header.m_isNormal != 0 && header.m_isNormal != 1)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: normal");
		return Error::USER_DATA;
	}

	// Set a few things
	colorFormat = header.m_colorFormat;
	imageType = header.m_type;
	astcBlockSize = UVec2(header.m_astcBlockSizeX, header.m_astcBlockSizeY);

	U32 faceCount = 1;
	switch(header.m_type)
	{
	case ImageBinaryType::_2D:
		depth = 1;
		layerCount = 1;
		break;
	case ImageBinaryType::CUBE:
		depth = 1;
		layerCount = 1;
		faceCount = 6;
		break;
	case ImageBinaryType::_3D:
		depth = header.m_depthOrLayerCount;
		layerCount = 1;
		break;
	case ImageBinaryType::_2D_ARRAY:
		depth = 1;
		layerCount = header.m_depthOrLayerCount;
		break;
	default:
		ANKI_ASSERT(0);
	}

	//
	// Move file pointer
	//
	PtrSize skipSize = 0;

	if(preferredCompression == ImageBinaryDataCompression::RAW)
	{
		// Do nothing
	}
	else if(preferredCompression == ImageBinaryDataCompression::S3TC)
	{
		if(!!(header.m_compressionMask & ImageBinaryDataCompression::RAW))
		{
			// If raw compression is present then skip it
			skipSize += calcSizeOfSegment(header, ImageBinaryDataCompression::RAW);
		}
	}
	else if(preferredCompression == ImageBinaryDataCompression::ETC)
	{
		if(!!(header.m_compressionMask & ImageBinaryDataCompression::RAW))
		{
			// If raw compression is present then skip it
			skipSize += calcSizeOfSegment(header, ImageBinaryDataCompression::RAW);
		}

		if(!!(header.m_compressionMask & ImageBinaryDataCompression::S3TC))
		{
			// If s3tc compression is present then skip it
			skipSize += calcSizeOfSegment(header, ImageBinaryDataCompression::S3TC);
		}
	}
	else if(preferredCompression == ImageBinaryDataCompression::ASTC)
	{
		if(!!(header.m_compressionMask & ImageBinaryDataCompression::RAW))
		{
			// If raw compression is present then skip it
			skipSize += calcSizeOfSegment(header, ImageBinaryDataCompression::RAW);
		}

		if(!!(header.m_compressionMask & ImageBinaryDataCompression::S3TC))
		{
			// If s3tc compression is present then skip it
			skipSize += calcSizeOfSegment(header, ImageBinaryDataCompression::S3TC);
		}

		if(!!(header.m_compressionMask & ImageBinaryDataCompression::ETC))
		{
			// If ETC compression is present then skip it
			skipSize += calcSizeOfSegment(header, ImageBinaryDataCompression::ETC);
		}
	}

	if(skipSize)
	{
		ANKI_CHECK(file.seek(skipSize, FileSeekOrigin::CURRENT));
	}

	//
	// It's time to read
	//

	// Allocate the surfaces
	mipCount = 0;
	if(header.m_type != ImageBinaryType::_3D)
	{
		// Read all surfaces
		U32 mipWidth = header.m_width;
		U32 mipHeight = header.m_height;
		for(U32 mip = 0; mip < header.m_mipmapCount; mip++)
		{
			for(U32 l = 0; l < layerCount; l++)
			{
				for(U32 f = 0; f < faceCount; ++f)
				{
					const PtrSize dataSize =
						calcSurfaceSize(mipWidth, mipHeight, preferredCompression, header.m_colorFormat,
										UVec2(header.m_astcBlockSizeX, header.m_astcBlockSizeY));

					// Check if this mipmap can be skipped because of size
					if(max(mipWidth, mipHeight) <= maxImageSize || mip == header.m_mipmapCount - 1)
					{
						ImageLoaderSurface& surf = *surfaces.emplaceBack(alloc);
						surf.m_width = mipWidth;
						surf.m_height = mipHeight;

						surf.m_data.create(alloc, dataSize);
						ANKI_CHECK(file.read(&surf.m_data[0], dataSize));

						mipCount = max(header.m_mipmapCount - mip, mipCount);
					}
					else
					{
						ANKI_CHECK(file.seek(dataSize, FileSeekOrigin::CURRENT));
					}
				}
			}

			mipWidth /= 2;
			mipHeight /= 2;
		}

		width = surfaces[0].m_width;
		height = surfaces[0].m_height;
		depth = MAX_U32;
	}
	else
	{
		U32 mipWidth = header.m_width;
		U32 mipHeight = header.m_height;
		U32 mipDepth = header.m_depthOrLayerCount;
		for(U32 mip = 0; mip < header.m_mipmapCount; mip++)
		{
			const U32 dataSize =
				U32(calcVolumeSize(mipWidth, mipHeight, mipDepth, preferredCompression, header.m_colorFormat));

			// Check if this mipmap can be skipped because of size
			if(max(max(mipWidth, mipHeight), mipDepth) <= maxImageSize || mip == header.m_mipmapCount - 1)
			{
				ImageLoaderVolume& vol = *volumes.emplaceBack(alloc);
				vol.m_width = mipWidth;
				vol.m_height = mipHeight;
				vol.m_depth = mipDepth;

				vol.m_data.create(alloc, dataSize);
				ANKI_CHECK(file.read(&vol.m_data[0], dataSize));

				mipCount = max(header.m_mipmapCount - mip, mipCount);
			}
			else
			{
				ANKI_CHECK(file.seek(dataSize, FileSeekOrigin::CURRENT));
			}

			mipWidth /= 2;
			mipHeight /= 2;
			mipDepth /= 2;
		}

		width = volumes[0].m_width;
		height = volumes[0].m_height;
		depth = volumes[0].m_depth;
	}

	return Error::NONE;
}

Error ImageLoader::loadStb(Bool isFloat, FileInterface& fs, U32& width, U32& height, DynamicArray<U8, PtrSize>& data,
						   GenericMemoryPoolAllocator<U8>& alloc)
{
	// Read the file
	DynamicArrayAuto<U8, PtrSize> fileData(alloc);
	const PtrSize fileSize = fs.getSize();
	fileData.create(fileSize);
	ANKI_CHECK(fs.read(&fileData[0], fileSize));

	// Use STB to read the image
	int stbw, stbh, comp;
	stbi_set_flip_vertically_on_load_thread(true);
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
		return Error::FUNCTION_FAILED;
	}

	// Store it
	width = U32(stbw);
	height = U32(stbh);
	const U32 componentSize = (isFloat) ? sizeof(F32) : sizeof(U8);
	data.create(alloc, width * height * 4 * componentSize);
	memcpy(&data[0], stbdata, data.getSize());

	// Cleanup
	stbi_image_free(stbdata);

	return Error::NONE;
}

Error ImageLoader::load(ResourceFilePtr rfile, const CString& filename, U32 maxImageSize)
{
	RsrcFile file;
	file.m_rfile = rfile;

	const Error err = loadInternal(file, filename, maxImageSize);
	if(err)
	{
		ANKI_RESOURCE_LOGE("Failed to read image: %s", filename.cstr());
	}

	return err;
}

Error ImageLoader::load(const CString& filename, U32 maxImageSize)
{
	SystemFile file;
	ANKI_CHECK(file.m_file.open(filename, FileOpenFlag::READ | FileOpenFlag::BINARY));

	const Error err = loadInternal(file, filename, maxImageSize);
	if(err)
	{
		ANKI_RESOURCE_LOGE("Failed to read image: %s", filename.cstr());
	}

	return err;
}

Error ImageLoader::loadInternal(FileInterface& file, const CString& filename, U32 maxImageSize)
{
	// get the extension
	StringAuto ext(m_alloc);
	getFilepathExtension(filename, ext);

	if(ext.isEmpty())
	{
		ANKI_RESOURCE_LOGE("Failed to get filename extension");
		return Error::USER_DATA;
	}

	// load from this extension
	m_imageType = ImageBinaryType::_2D;
	m_compression = ImageBinaryDataCompression::RAW;

	if(ext == "tga")
	{
		m_surfaces.create(m_alloc, 1);

		m_mipmapCount = 1;
		m_depth = 1;
		m_layerCount = 1;
		U32 bpp = 0;
		ANKI_CHECK(loadTga(file, m_surfaces[0].m_width, m_surfaces[0].m_height, bpp, m_surfaces[0].m_data, m_alloc));

		m_width = m_surfaces[0].m_width;
		m_height = m_surfaces[0].m_height;

		if(bpp == 32)
		{
			m_colorFormat = ImageBinaryColorFormat::RGBA8;
		}
		else if(bpp == 24)
		{
			m_colorFormat = ImageBinaryColorFormat::RGB8;
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}
	else if(ext == "ankitex")
	{
#if ANKI_PLATFORM_MOBILE
		m_compression = ImageBinaryDataCompression::ASTC;
#else
		m_compression = ImageBinaryDataCompression::S3TC;
#endif

		ANKI_CHECK(loadAnkiImage(file, maxImageSize, m_compression, m_surfaces, m_volumes, m_alloc, m_width, m_height,
								 m_depth, m_layerCount, m_mipmapCount, m_imageType, m_colorFormat, m_astcBlockSize));
	}
	else if(ext == "png" || ext == "jpg")
	{
		m_surfaces.create(m_alloc, 1);

		m_mipmapCount = 1;
		m_depth = 1;
		m_layerCount = 1;
		m_colorFormat = ImageBinaryColorFormat::RGBA8;

		ANKI_CHECK(loadStb(false, file, m_surfaces[0].m_width, m_surfaces[0].m_height, m_surfaces[0].m_data, m_alloc));

		m_width = m_surfaces[0].m_width;
		m_height = m_surfaces[0].m_height;
	}
	else if(ext == "hdr")
	{
		m_surfaces.create(m_alloc, 1);

		m_mipmapCount = 1;
		m_depth = 1;
		m_layerCount = 1;
		m_colorFormat = ImageBinaryColorFormat::RGBAF32;

		ANKI_CHECK(loadStb(true, file, m_surfaces[0].m_width, m_surfaces[0].m_height, m_surfaces[0].m_data, m_alloc));

		m_width = m_surfaces[0].m_width;
		m_height = m_surfaces[0].m_height;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Unsupported extension: %s", &ext[0]);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

const ImageLoaderSurface& ImageLoader::getSurface(U32 level, U32 face, U32 layer) const
{
	ANKI_ASSERT(level < m_mipmapCount);

	U32 idx = 0;

	switch(m_imageType)
	{
	case ImageBinaryType::_2D:
		idx = level;
		break;
	case ImageBinaryType::CUBE:
		ANKI_ASSERT(face < 6);
		idx = level * 6 + face;
		break;
	case ImageBinaryType::_3D:
		ANKI_ASSERT(0 && "Can't use that for 3D images");
		break;
	case ImageBinaryType::_2D_ARRAY:
		idx = level * m_layerCount + layer;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return m_surfaces[idx];
}

const ImageLoaderVolume& ImageLoader::getVolume(U32 level) const
{
	ANKI_ASSERT(m_imageType == ImageBinaryType::_3D);
	return m_volumes[level];
}

void ImageLoader::destroy()
{
	for(ImageLoaderSurface& surf : m_surfaces)
	{
		surf.m_data.destroy(m_alloc);
	}

	m_surfaces.destroy(m_alloc);

	for(ImageLoaderVolume& v : m_volumes)
	{
		v.m_data.destroy(m_alloc);
	}

	m_volumes.destroy(m_alloc);
}

} // end namespace anki
