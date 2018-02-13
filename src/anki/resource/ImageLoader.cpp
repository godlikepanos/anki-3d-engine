// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ImageLoader.h>
#include <anki/util/Logger.h>
#include <anki/util/Filesystem.h>
#include <anki/util/Assert.h>
#include <anki/util/Array.h>

namespace anki
{

static U8 tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static U8 tgaHeaderCompressed[12] = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static ANKI_USE_RESULT Error loadUncompressedTga(ResourceFilePtr fs,
	U32& width,
	U32& height,
	U32& bpp,
	DynamicArray<U8>& data,
	GenericMemoryPoolAllocator<U8>& alloc)
{
	Array<U8, 6> header6;

	// read the info from header
	ANKI_CHECK(fs->read((char*)&header6[0], sizeof(header6)));

	width = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width == 0) || (height == 0) || ((bpp != 24) && (bpp != 32)))
	{
		ANKI_RESOURCE_LOGE("Invalid image information");
		return Error::USER_DATA;
	}

	// read the data
	I bytesPerPxl = (bpp / 8);
	I imageSize = bytesPerPxl * width * height;
	data.create(alloc, imageSize);

	ANKI_CHECK(fs->read(reinterpret_cast<char*>(&data[0]), imageSize));

	// swap red with blue
	for(I i = 0; i < imageSize; i += bytesPerPxl)
	{
		U32 temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	return Error::NONE;
}

static ANKI_USE_RESULT Error loadCompressedTga(ResourceFilePtr fs,
	U32& width,
	U32& height,
	U32& bpp,
	DynamicArray<U8>& data,
	GenericMemoryPoolAllocator<U8>& alloc)
{
	Array<U8, 6> header6;
	ANKI_CHECK(fs->read(reinterpret_cast<char*>(&header6[0]), sizeof(header6)));

	width = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32)))
	{
		ANKI_RESOURCE_LOGE("Invalid texture information");
		return Error::USER_DATA;
	}

	I bytesPerPxl = (bpp / 8);
	I imageSize = bytesPerPxl * width * height;
	data.create(alloc, imageSize);

	U pixelcount = height * width;
	U currentpixel = 0;
	U currentbyte = 0;
	U8 colorbuffer[4];

	do
	{
		U8 chunkheader = 0;

		ANKI_CHECK(fs->read((char*)&chunkheader, sizeof(U8)));

		if(chunkheader < 128)
		{
			chunkheader++;
			for(int counter = 0; counter < chunkheader; counter++)
			{
				ANKI_CHECK(fs->read((char*)&colorbuffer[0], bytesPerPxl));

				data[currentbyte] = colorbuffer[2];
				data[currentbyte + 1] = colorbuffer[1];
				data[currentbyte + 2] = colorbuffer[0];

				if(bytesPerPxl == 4)
				{
					data[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += bytesPerPxl;
				currentpixel++;

				if(currentpixel > pixelcount)
				{
					ANKI_RESOURCE_LOGE("Too many pixels read");
					return Error::USER_DATA;
				}
			}
		}
		else
		{
			chunkheader -= 127;
			ANKI_CHECK(fs->read((char*)&colorbuffer[0], bytesPerPxl));

			for(int counter = 0; counter < chunkheader; counter++)
			{
				data[currentbyte] = colorbuffer[2];
				data[currentbyte + 1] = colorbuffer[1];
				data[currentbyte + 2] = colorbuffer[0];

				if(bytesPerPxl == 4)
				{
					data[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += bytesPerPxl;
				currentpixel++;

				if(currentpixel > pixelcount)
				{
					ANKI_RESOURCE_LOGE("Too many pixels read");
					data.destroy(alloc);
					return Error::USER_DATA;
				}
			}
		}
	} while(currentpixel < pixelcount);

	return Error::NONE;
}

static ANKI_USE_RESULT Error loadTga(ResourceFilePtr fs,
	U32& width,
	U32& height,
	U32& bpp,
	DynamicArray<U8>& data,
	GenericMemoryPoolAllocator<U8>& alloc)
{
	char myTgaHeader[12];

	ANKI_CHECK(fs->read(&myTgaHeader[0], sizeof(myTgaHeader)));

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

class AnkiTextureHeader
{
public:
	Array<U8, 8> m_magic;
	U32 m_width;
	U32 m_height;
	U32 m_depthOrLayerCount;
	ImageLoader::TextureType m_type;
	ImageLoader::ColorFormat m_colorFormat;
	ImageLoader::DataCompression m_compressionFormats;
	U32 m_normal;
	U32 m_mipLevels;
	U8 m_padding[88];
};

static_assert(sizeof(AnkiTextureHeader) == 128, "Check sizeof AnkiTextureHeader");

/// Get the size in bytes of a single surface
static PtrSize calcSurfaceSize(
	const U width, const U height, const ImageLoader::DataCompression comp, const ImageLoader::ColorFormat cf)
{
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4);

	switch(comp)
	{
	case ImageLoader::DataCompression::RAW:
		out = width * height * ((cf == ImageLoader::ColorFormat::RGB8) ? 3 : 4);
		break;
	case ImageLoader::DataCompression::S3TC:
		out = (width / 4) * (height / 4) * ((cf == ImageLoader::ColorFormat::RGB8) ? 8 : 16); // block size
		break;
	case ImageLoader::DataCompression::ETC:
		out = (width / 4) * (height / 4) * 8;
		break;
	default:
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out > 0);

	return out;
}

/// Get the size in bytes of a single volume
static PtrSize calcVolumeSize(const U width,
	const U height,
	const U depth,
	const ImageLoader::DataCompression comp,
	const ImageLoader::ColorFormat cf)
{
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4 || depth >= 4);

	switch(comp)
	{
	case ImageLoader::DataCompression::RAW:
		out = width * height * depth * ((cf == ImageLoader::ColorFormat::RGB8) ? 3 : 4);
		break;
	default:
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out > 0);

	return out;
}

/// Calculate the size of a compressed or uncomressed color data
static PtrSize calcSizeOfSegment(const AnkiTextureHeader& header, ImageLoader::DataCompression comp)
{
	PtrSize out = 0;
	U width = header.m_width;
	U height = header.m_height;
	U mips = header.m_mipLevels;
	ANKI_ASSERT(mips > 0);

	if(header.m_type != ImageLoader::TextureType::_3D)
	{
		U surfCountPerMip = 0;

		switch(header.m_type)
		{
		case ImageLoader::TextureType::_2D:
			surfCountPerMip = 1;
			break;
		case ImageLoader::TextureType::CUBE:
			surfCountPerMip = 6;
			break;
		case ImageLoader::TextureType::_2D_ARRAY:
			surfCountPerMip = header.m_depthOrLayerCount;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}

		while(mips-- != 0)
		{
			out += calcSurfaceSize(width, height, comp, header.m_colorFormat) * surfCountPerMip;

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

static ANKI_USE_RESULT Error loadAnkiTexture(ResourceFilePtr file,
	U32 maxTextureSize,
	ImageLoader::DataCompression& preferredCompression,
	DynamicArray<ImageLoader::Surface>& surfaces,
	DynamicArray<ImageLoader::Volume>& volumes,
	GenericMemoryPoolAllocator<U8>& alloc,
	U32& width,
	U32& height,
	U32& depth,
	U32& layerCount,
	U8& toLoadMipCount,
	ImageLoader::TextureType& textureType,
	ImageLoader::ColorFormat& colorFormat)
{
	//
	// Read and check the header
	//
	AnkiTextureHeader header;
	ANKI_CHECK(file->read(&header, sizeof(AnkiTextureHeader)));

	if(std::memcmp(&header.m_magic[0], "ANKITEX1", 8) != 0)
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

	if(header.m_type < ImageLoader::TextureType::_2D || header.m_type > ImageLoader::TextureType::_2D_ARRAY)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: texture type");
		return Error::USER_DATA;
	}

	if(header.m_colorFormat < ImageLoader::ColorFormat::RGB8 || header.m_colorFormat > ImageLoader::ColorFormat::RGBA8)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: color format");
		return Error::USER_DATA;
	}

	if((header.m_compressionFormats & preferredCompression) == ImageLoader::DataCompression::NONE)
	{
		ANKI_RESOURCE_LOGW("File does not contain the requested compression");

		// Fallback
		preferredCompression = ImageLoader::DataCompression::RAW;

		if((header.m_compressionFormats & preferredCompression) == ImageLoader::DataCompression::NONE)
		{
			ANKI_RESOURCE_LOGE("File does not contain raw compression");
			return Error::USER_DATA;
		}
	}

	if(header.m_normal != 0 && header.m_normal != 1)
	{
		ANKI_RESOURCE_LOGE("Incorrect header: normal");
		return Error::USER_DATA;
	}

	width = header.m_width;
	height = header.m_height;

	// Check mip levels
	U size = min(header.m_width, header.m_height);
	U maxSize = max(header.m_width, header.m_height);
	if(header.m_type == ImageLoader::TextureType::_3D)
	{
		maxSize = max<U>(maxSize, header.m_depthOrLayerCount);
		size = min<U>(size, header.m_depthOrLayerCount);
	}
	toLoadMipCount = 0;
	U tmpMipLevels = 0;
	while(size >= 4) // The minimum size is 4x4
	{
		++tmpMipLevels;

		if(maxSize <= maxTextureSize)
		{
			++toLoadMipCount;
		}

		size /= 2;
		maxSize /= 2;
	}

	if(header.m_mipLevels > tmpMipLevels)
	{
		ANKI_RESOURCE_LOGE("Incorrect number of mip levels");
		return Error::USER_DATA;
	}

	toLoadMipCount = min<U>(toLoadMipCount, header.m_mipLevels);

	colorFormat = header.m_colorFormat;

	U faceCount = 1;
	switch(header.m_type)
	{
	case ImageLoader::TextureType::_2D:
		depth = 1;
		layerCount = 1;
		break;
	case ImageLoader::TextureType::CUBE:
		depth = 1;
		layerCount = 1;
		faceCount = 6;
		break;
	case ImageLoader::TextureType::_3D:
		depth = header.m_depthOrLayerCount;
		layerCount = 1;
		break;
	case ImageLoader::TextureType::_2D_ARRAY:
		depth = 1;
		layerCount = header.m_depthOrLayerCount;
		break;
	default:
		ANKI_ASSERT(0);
	}

	textureType = header.m_type;

	//
	// Move file pointer
	//

	if(preferredCompression == ImageLoader::DataCompression::RAW)
	{
		// Do nothing
	}
	else if(preferredCompression == ImageLoader::DataCompression::S3TC)
	{
		if((header.m_compressionFormats & ImageLoader::DataCompression::RAW) != ImageLoader::DataCompression::NONE)
		{
			// If raw compression is present then skip it
			ANKI_CHECK(file->seek(
				calcSizeOfSegment(header, ImageLoader::DataCompression::RAW), ResourceFile::SeekOrigin::CURRENT));
		}
	}
	else if(preferredCompression == ImageLoader::DataCompression::ETC)
	{
		if((header.m_compressionFormats & ImageLoader::DataCompression::RAW) != ImageLoader::DataCompression::NONE)
		{
			// If raw compression is present then skip it
			ANKI_CHECK(file->seek(
				calcSizeOfSegment(header, ImageLoader::DataCompression::RAW), ResourceFile::SeekOrigin::CURRENT));
		}

		if((header.m_compressionFormats & ImageLoader::DataCompression::S3TC) != ImageLoader::DataCompression::NONE)
		{
			// If s3tc compression is present then skip it
			ANKI_CHECK(file->seek(
				calcSizeOfSegment(header, ImageLoader::DataCompression::S3TC), ResourceFile::SeekOrigin::CURRENT));
		}
	}

	//
	// It's time to read
	//

	// Allocate the surfaces
	if(header.m_type != ImageLoader::TextureType::_3D)
	{
		surfaces.create(alloc, toLoadMipCount * layerCount * faceCount);

		// Read all surfaces
		U mipWidth = header.m_width;
		U mipHeight = header.m_height;
		U index = 0;
		for(U mip = 0; mip < header.m_mipLevels; mip++)
		{
			for(U l = 0; l < layerCount; l++)
			{

				for(U f = 0; f < faceCount; ++f)
				{
					U dataSize = calcSurfaceSize(mipWidth, mipHeight, preferredCompression, header.m_colorFormat);

					// Check if this mipmap can be skipped because of size
					if(maxSize <= maxTextureSize)
					{
						ImageLoader::Surface& surf = surfaces[index++];
						surf.m_width = mipWidth;
						surf.m_height = mipHeight;

						surf.m_data.create(alloc, dataSize);

						ANKI_CHECK(file->read(&surf.m_data[0], dataSize));
					}
					else
					{
						ANKI_CHECK(file->seek(dataSize, ResourceFile::SeekOrigin::CURRENT));
					}
				}
			}

			mipWidth /= 2;
			mipHeight /= 2;
		}
	}
	else
	{
		volumes.create(alloc, toLoadMipCount);

		U mipWidth = header.m_width;
		U mipHeight = header.m_height;
		U mipDepth = header.m_depthOrLayerCount;
		for(U mip = 0; mip < header.m_mipLevels; mip++)
		{
			U dataSize = calcVolumeSize(mipWidth, mipHeight, mipDepth, preferredCompression, header.m_colorFormat);

			// Check if this mipmap can be skipped because of size
			if(maxSize <= maxTextureSize)
			{
				ImageLoader::Volume& vol = volumes[mip];
				vol.m_width = mipWidth;
				vol.m_height = mipHeight;
				vol.m_depth = mipDepth;

				vol.m_data.create(alloc, dataSize);

				ANKI_CHECK(file->read(&vol.m_data[0], dataSize));
			}
			else
			{
				ANKI_CHECK(file->seek(dataSize, ResourceFile::SeekOrigin::CURRENT));
			}

			mipWidth /= 2;
			mipHeight /= 2;
			mipDepth /= 2;
		}
	}

	return Error::NONE;
}

Error ImageLoader::load(ResourceFilePtr file, const CString& filename, U32 maxTextureSize)
{
	// get the extension
	StringAuto ext(m_alloc);
	getFileExtension(filename, m_alloc, ext);

	if(ext.isEmpty())
	{
		ANKI_RESOURCE_LOGE("Failed to get filename extension");
		return Error::USER_DATA;
	}

	// load from this extension
	U32 bpp = 0;
	m_textureType = TextureType::_2D;
	m_compression = DataCompression::RAW;

	if(ext == "tga")
	{
		m_surfaces.create(m_alloc, 1);

		m_mipLevels = 1;
		m_depth = 1;
		m_layerCount = 1;
		ANKI_CHECK(loadTga(file, m_surfaces[0].m_width, m_surfaces[0].m_height, bpp, m_surfaces[0].m_data, m_alloc));

		m_width = m_surfaces[0].m_width;
		m_height = m_surfaces[0].m_height;

		if(bpp == 32)
		{
			m_colorFormat = ColorFormat::RGBA8;
		}
		else if(bpp == 24)
		{
			m_colorFormat = ColorFormat::RGB8;
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}
	else if(ext == "ankitex")
	{
#if 0
		compression = ImageLoader::DataCompression::RAW;
#elif ANKI_GL == ANKI_GL_DESKTOP
		m_compression = ImageLoader::DataCompression::S3TC;
#else
		m_compression = ImageLoader::DataCompression::ETC;
#endif

		ANKI_CHECK(loadAnkiTexture(file,
			maxTextureSize,
			m_compression,
			m_surfaces,
			m_volumes,
			m_alloc,
			m_width,
			m_height,
			m_depth,
			m_layerCount,
			m_mipLevels,
			m_textureType,
			m_colorFormat));
	}
	else
	{
		ANKI_RESOURCE_LOGE("Unsupported extension: %s", &ext[0]);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

const ImageLoader::Surface& ImageLoader::getSurface(U level, U face, U layer) const
{
	ANKI_ASSERT(level < m_mipLevels);

	U idx = 0;

	switch(m_textureType)
	{
	case TextureType::_2D:
		idx = level;
		break;
	case TextureType::CUBE:
		ANKI_ASSERT(face < 6);
		idx = level * 6 + face;
		break;
	case TextureType::_3D:
		ANKI_ASSERT(0 && "Can't use that for 3D textures");
		break;
	case TextureType::_2D_ARRAY:
		idx = level * m_layerCount + layer;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return m_surfaces[idx];
}

const ImageLoader::Volume& ImageLoader::getVolume(U level) const
{
	ANKI_ASSERT(m_textureType == TextureType::_3D);
	return m_volumes[level];
}

void ImageLoader::destroy()
{
	for(Surface& surf : m_surfaces)
	{
		surf.m_data.destroy(m_alloc);
	}

	m_surfaces.destroy(m_alloc);

	for(Volume& v : m_volumes)
	{
		v.m_data.destroy(m_alloc);
	}

	m_volumes.destroy(m_alloc);
}

} // end namespace anki
