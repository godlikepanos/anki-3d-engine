// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ImageLoader.h>
#include <anki/util/Logger.h>
#include <anki/util/Filesystem.h>
#include <anki/util/Assert.h>
#include <anki/util/Array.h>

namespace anki {

//==============================================================================
// TGA                                                                         =
//==============================================================================

//==============================================================================
static U8 tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static U8 tgaHeaderCompressed[12] = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//==============================================================================
static ANKI_USE_RESULT Error loadUncompressedTga(
	ResourceFilePtr fs, U32& width, U32& height, U32& bpp, DArray<U8>& data,
	GenericMemoryPoolAllocator<U8>& alloc)
{
	Array<U8, 6> header6;

	// read the info from header
	ANKI_CHECK(fs->read((char*)&header6[0], sizeof(header6)));

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width == 0) || (height == 0) || ((bpp != 24) && (bpp != 32)))
	{
		ANKI_LOGE("Invalid image information");
		return ErrorCode::USER_DATA;
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

	return ErrorCode::NONE;
}

//==============================================================================
static ANKI_USE_RESULT Error loadCompressedTga(
	ResourceFilePtr fs, U32& width, U32& height, U32& bpp, DArray<U8>& data,
	GenericMemoryPoolAllocator<U8>& alloc)
{
	Array<U8, 6> header6;
	ANKI_CHECK(fs->read(reinterpret_cast<char*>(&header6[0]), sizeof(header6)));

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp	= header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32)))
	{
		ANKI_LOGE("Invalid texture information");
		return ErrorCode::USER_DATA;
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
					ANKI_LOGE("Too many pixels read");
					return ErrorCode::USER_DATA;
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
					ANKI_LOGE("Too many pixels read");
					data.destroy(alloc);
					return ErrorCode::USER_DATA;
				}
			}
		}
	} while(currentpixel < pixelcount);

	return ErrorCode::NONE;
}

//==============================================================================
static ANKI_USE_RESULT Error loadTga(ResourceFilePtr fs,
	U32& width, U32& height, U32& bpp, DArray<U8>& data,
	GenericMemoryPoolAllocator<U8>& alloc)
{
	char myTgaHeader[12];

	ANKI_CHECK(fs->read(&myTgaHeader[0], sizeof(myTgaHeader)));

	if(memcmp(tgaHeaderUncompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		ANKI_CHECK(loadUncompressedTga(fs, width, height, bpp, data, alloc));
	}
	else if(std::memcmp(tgaHeaderCompressed, &myTgaHeader[0],
		sizeof(myTgaHeader)) == 0)
	{
		ANKI_CHECK(loadCompressedTga(fs, width, height, bpp, data, alloc));
	}
	else
	{
		ANKI_LOGE("Invalid image header");
		return ErrorCode::USER_DATA;
	}

	if(bpp != 32 && bpp != 24)
	{
		ANKI_LOGE("Invalid bpp");
		return ErrorCode::USER_DATA;
	}

	return ErrorCode::NONE;
}

//==============================================================================
// ANKI                                                                        =
//==============================================================================

//==============================================================================
class AnkiTextureHeader
{
public:
	Array<U8, 8> m_magic;
	U32 m_width;
	U32 m_height;
	U32 m_depth;
	ImageLoader::TextureType m_type;
	ImageLoader::ColorFormat m_colorFormat;
	ImageLoader::DataCompression m_compressionFormats;
	U32 m_normal;
	U32 m_mipLevels;
	U8 m_padding[88];
};

static_assert(sizeof(AnkiTextureHeader) == 128,
	"Check sizeof AnkiTextureHeader");

//==============================================================================
/// Get the size in bytes of a single surface
static PtrSize calcSurfaceSize(const U width, const U height,
	const ImageLoader::DataCompression comp, const ImageLoader::ColorFormat cf)
{
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4);

	switch(comp)
	{
	case ImageLoader::DataCompression::RAW:
		out = width * height * ((cf == ImageLoader::ColorFormat::RGB8) ? 3 : 4);
		break;
	case ImageLoader::DataCompression::S3TC:
		out = (width / 4) * (height / 4)
			* ((cf == ImageLoader::ColorFormat::RGB8) ? 8 : 16); // block size
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

//==============================================================================
/// Calculate the size of a compressed or uncomressed color data
static PtrSize calcSizeOfSegment(const AnkiTextureHeader& header,
	ImageLoader::DataCompression comp)
{
	PtrSize out = 0;
	U width = header.m_width;
	U height = header.m_height;
	U mips = header.m_mipLevels;
	U layers = 0;

	ANKI_ASSERT(mips > 0);

	switch(header.m_type)
	{
	case ImageLoader::TextureType::_2D:
		layers = 1;
		break;
	case ImageLoader::TextureType::CUBE:
		layers = 6;
		break;
	case ImageLoader::TextureType::_2D_ARRAY:
	case ImageLoader::TextureType::_3D:
		layers = header.m_depth;
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	while(mips-- != 0)
	{
		out += calcSurfaceSize(width, height, comp, header.m_colorFormat)
			* layers;

		width /= 2;
		height /= 2;
	}

	return out;
}

//==============================================================================
static ANKI_USE_RESULT Error loadAnkiTexture(
	ResourceFilePtr file,
	U32 maxTextureSize,
	ImageLoader::DataCompression& preferredCompression,
	DArray<ImageLoader::Surface>& surfaces,
	GenericMemoryPoolAllocator<U8>& alloc,
	U8& depth,
	U8& mipLevels,
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
		ANKI_LOGE("Wrong magic word");
		return ErrorCode::USER_DATA;
	}

	if(header.m_width == 0
		|| !isPowerOfTwo(header.m_width)
		|| header.m_width > 4096
		|| header.m_height == 0
		|| !isPowerOfTwo(header.m_height)
		|| header.m_height > 4096)
	{
		ANKI_LOGE("Incorrect width/height value");
		return ErrorCode::USER_DATA;
	}

	if(header.m_depth < 1 || header.m_depth > 128)
	{
		ANKI_LOGE("Zero or too big depth");
		return ErrorCode::USER_DATA;
	}

	if(header.m_type < ImageLoader::TextureType::_2D
		|| header.m_type > ImageLoader::TextureType::_2D_ARRAY)
	{
		ANKI_LOGE("Incorrect header: texture type");
		return ErrorCode::USER_DATA;
	}

	if(header.m_colorFormat < ImageLoader::ColorFormat::RGB8
		|| header.m_colorFormat > ImageLoader::ColorFormat::RGBA8)
	{
		ANKI_LOGE("Incorrect header: color format");
		return ErrorCode::USER_DATA;
	}

	if((header.m_compressionFormats & preferredCompression)
		== ImageLoader::DataCompression::NONE)
	{
		ANKI_LOGW("File does not contain the requested compression");

		// Fallback
		preferredCompression = ImageLoader::DataCompression::RAW;

		if((header.m_compressionFormats & preferredCompression)
			== ImageLoader::DataCompression::NONE)
		{
			ANKI_LOGE("File does not contain raw compression");
			return ErrorCode::USER_DATA;
		}
	}

	if(header.m_normal != 0 && header.m_normal != 1)
	{
		ANKI_LOGE("Incorrect header: normal");
		return ErrorCode::USER_DATA;
	}

	// Check mip levels
	U size = min(header.m_width, header.m_height);
	U maxsize = max(header.m_width, header.m_height);
	mipLevels = 0;
	U tmpMipLevels = 0;
	while(size >= 4) // The minimum size is 4x4
	{
		++tmpMipLevels;

		if(maxsize <= maxTextureSize)
		{
			++mipLevels;
		}

		size /= 2;
		maxsize /= 2;
	}

	if(header.m_mipLevels > tmpMipLevels)
	{
		ANKI_LOGE("Incorrect number of mip levels");
		return ErrorCode::USER_DATA;
	}

	mipLevels = min<U>(mipLevels, header.m_mipLevels);

	colorFormat = header.m_colorFormat;

	switch(header.m_type)
	{
	case ImageLoader::TextureType::_2D:
		depth = 1;
		break;
	case ImageLoader::TextureType::CUBE:
		depth = 6;
		break;
	case ImageLoader::TextureType::_3D:
	case ImageLoader::TextureType::_2D_ARRAY:
		depth = header.m_depth;
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
		if((header.m_compressionFormats & ImageLoader::DataCompression::RAW)
			!= ImageLoader::DataCompression::NONE)
		{
			// If raw compression is present then skip it
			ANKI_CHECK(file->seek(
				calcSizeOfSegment(header, ImageLoader::DataCompression::RAW),
				ResourceFile::SeekOrigin::CURRENT));
		}
	}
	else if(preferredCompression == ImageLoader::DataCompression::ETC)
	{
		if((header.m_compressionFormats & ImageLoader::DataCompression::RAW)
			!= ImageLoader::DataCompression::NONE)
		{
			// If raw compression is present then skip it
			ANKI_CHECK(file->seek(
				calcSizeOfSegment(header, ImageLoader::DataCompression::RAW),
				ResourceFile::SeekOrigin::CURRENT));
		}

		if((header.m_compressionFormats & ImageLoader::DataCompression::S3TC)
			!= ImageLoader::DataCompression::NONE)
		{
			// If s3tc compression is present then skip it
			ANKI_CHECK(file->seek(
				calcSizeOfSegment(header, ImageLoader::DataCompression::S3TC),
				ResourceFile::SeekOrigin::CURRENT));
		}
	}

	//
	// It's time to read
	//

	// Allocate the surfaces
	surfaces.create(alloc, mipLevels * depth);

	// Read all surfaces
	U mipWidth = header.m_width;
	U mipHeight = header.m_height;
	U index = 0;
	for(U mip = 0; mip < header.m_mipLevels; mip++)
	{
		for(U d = 0; d < depth; d++)
		{
			U dataSize = calcSurfaceSize(
				mipWidth, mipHeight, preferredCompression,
				header.m_colorFormat);

			// Check if this mipmap can be skipped because of size
			if(max(mipWidth, mipHeight) <= maxTextureSize)
			{
				ImageLoader::Surface& surf = surfaces[index++];
				surf.m_width = mipWidth;
				surf.m_height = mipHeight;

				surf.m_data.create(alloc, dataSize);

				ANKI_CHECK(file->read(&surf.m_data[0], dataSize));
			}
			else
			{
				ANKI_CHECK(
					file->seek(dataSize, ResourceFile::SeekOrigin::CURRENT));
			}
		}

		mipWidth /= 2;
		mipHeight /= 2;
	}

	return ErrorCode::NONE;
}

//==============================================================================
// ImageLoader                                                                 =
//==============================================================================

//==============================================================================
Error ImageLoader::load(ResourceFilePtr file, const CString& filename,
	U32 maxTextureSize)
{
	// get the extension
	StringAuto ext(m_alloc);
	getFileExtension(filename, m_alloc, ext);

	if(ext.isEmpty())
	{
		ANKI_LOGE("Failed to get filename extension");
		return ErrorCode::USER_DATA;
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
		ANKI_CHECK(loadTga(file, m_surfaces[0].m_width,
			m_surfaces[0].m_height, bpp, m_surfaces[0].m_data, m_alloc));

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

		ANKI_CHECK(loadAnkiTexture(file, maxTextureSize,
			m_compression, m_surfaces, m_alloc, m_depth,
			m_mipLevels, m_textureType, m_colorFormat));

	}
	else
	{
		ANKI_LOGE("Unsupported extension: %s", &ext[0]);
		return ErrorCode::USER_DATA;
	}

	return ErrorCode::NONE;
}

//==============================================================================
const ImageLoader::Surface& ImageLoader::getSurface(U mipLevel, U layer) const
{
	ANKI_ASSERT(mipLevel < m_mipLevels);

	U layers = 0;

	switch(m_textureType)
	{
	case TextureType::_2D:
		layers = 1;
		break;
	case TextureType::CUBE:
		layers = 6;
		break;
	case TextureType::_3D:
	case TextureType::_2D_ARRAY:
		layers = m_depth;
		break;
	default:
		ANKI_ASSERT(0);
	}

	// [mip][depthFace]
	U index = mipLevel * layers + layer;

	ANKI_ASSERT(index < m_surfaces.getSize());

	return m_surfaces[index];
}

//==============================================================================
void ImageLoader::destroy()
{
	for(ImageLoader::Surface& surf : m_surfaces)
	{
		surf.m_data.destroy(m_alloc);
	}

	m_surfaces.destroy(m_alloc);
}

} // end namespace anki
