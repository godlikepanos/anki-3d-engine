// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Image.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/util/File.h"
#include "anki/util/Assert.h"
#include "anki/util/Array.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// TGA                                                                         =
//==============================================================================

//==============================================================================
static U8 tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static U8 tgaHeaderCompressed[12] = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//==============================================================================
static void loadUncompressedTga(
	File& fs, U32& width, U32& height, U32& bpp, ResourceVector<U8>& data)
{
	// read the info from header
	U8 header6[6];
	fs.read((char*)&header6[0], sizeof(header6));

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32)))
	{
		throw ANKI_EXCEPTION("Invalid image information");
	}

	// read the data
	I bytesPerPxl	= (bpp / 8);
	I imageSize = bytesPerPxl * width * height;
	data.resize(imageSize);

	fs.read(reinterpret_cast<char*>(&data[0]), imageSize);

	// swap red with blue
	for(int i = 0; i < int(imageSize); i += bytesPerPxl)
	{
		U32 temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}
}

//==============================================================================
static void loadCompressedTga(
	File& fs, U32& width, U32& height, U32& bpp, ResourceVector<U8>& data)
{
	U8 header6[6];
	fs.read((char*)&header6[0], sizeof(header6));

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp	= header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32)))
	{
		throw ANKI_EXCEPTION("Invalid texture information");
	}

	I bytesPerPxl = (bpp / 8);
	I imageSize = bytesPerPxl * width * height;
	data.resize(imageSize);

	U32 pixelcount = height * width;
	U32 currentpixel = 0;
	U32 currentbyte = 0;
	U8 colorbuffer[4];

	do
	{
		U8 chunkheader = 0;

		fs.read((char*)&chunkheader, sizeof(U8));

		if(chunkheader < 128)
		{
			chunkheader++;
			for(int counter = 0; counter < chunkheader; counter++)
			{
				fs.read((char*)&colorbuffer[0], bytesPerPxl);

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
					throw ANKI_EXCEPTION("Too many pixels read");
				}
			}
		}
		else
		{
			chunkheader -= 127;
			fs.read((char*)&colorbuffer[0], bytesPerPxl);

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
					throw ANKI_EXCEPTION("Too many pixels read");
				}
			}
		}
	} while(currentpixel < pixelcount);
}

//==============================================================================
static void loadTga(const char* filename, 
	U32& width, U32& height, U32& bpp, ResourceVector<U8>& data)
{
	File fs(filename, File::OpenFlag::READ | File::OpenFlag::BINARY);
	char myTgaHeader[12];

	fs.read(&myTgaHeader[0], sizeof(myTgaHeader));

	if(std::memcmp(
		tgaHeaderUncompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		loadUncompressedTga(fs, width, height, bpp, data);
	}
	else if(std::memcmp(tgaHeaderCompressed, &myTgaHeader[0],
		sizeof(myTgaHeader)) == 0)
	{
		loadCompressedTga(fs, width, height, bpp, data);
	}
	else
	{
		throw ANKI_EXCEPTION("Invalid image header");
	}

	if(bpp != 32 && bpp != 24)
	{
		throw ANKI_EXCEPTION("Invalid bpp");
	}
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
	Image::TextureType m_type;
	Image::ColorFormat m_colorFormat;
	Image::DataCompression m_compressionFormats;
	U32 m_normal;
	U32 m_mipLevels;
	U8 m_padding[88];
};

static_assert(sizeof(AnkiTextureHeader) == 128, 
	"Check sizeof AnkiTextureHeader");

//==============================================================================
/// Get the size in bytes of a single surface
static PtrSize calcSurfaceSize(const U width, const U height, 
	const Image::DataCompression comp, const Image::ColorFormat cf)
{
	PtrSize out = 0;

	ANKI_ASSERT(width >= 4 || height >= 4);

	switch(comp)
	{
	case Image::DataCompression::RAW:
		out = width * height * ((cf == Image::ColorFormat::RGB8) ? 3 : 4);
		break;
	case Image::DataCompression::S3TC:
		out = (width / 4) * (height / 4) 
			* ((cf == Image::ColorFormat::RGB8) ? 8 : 16); // This is the 
		                                                   // block size
		break;
	case Image::DataCompression::ETC:
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
	Image::DataCompression comp)
{
	PtrSize out = 0;
	U width = header.m_width;
	U height = header.m_height;
	U mips = header.m_mipLevels;
	U layers = 0;

	switch(header.m_type)
	{
	case Image::TextureType::_2D:
		layers = 1;
		break;
	case Image::TextureType::CUBE:
		layers = 6;
		break;
	case Image::TextureType::_2D_ARRAY:
	case Image::TextureType::_3D:
		layers = header.m_depth;
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	while(mips-- != 0)
	{
		U l = layers;

		while(l-- != 0)
		{
			out += calcSurfaceSize(width, height, comp, header.m_colorFormat);
		}

		width /= 2;
		height /= 2;
	}

	return out;
}

//==============================================================================
static void loadAnkiTexture(
	const char* filename, 
	U32 maxTextureSize,
	Image::DataCompression& preferredCompression,
	ResourceVector<Image::Surface>& surfaces, 
	U8& depth, 
	U8& mipLevels, 
	Image::TextureType& textureType,
	Image::ColorFormat& colorFormat)
{
	File file(filename, 
		File::OpenFlag::READ | File::OpenFlag::BINARY 
		| File::OpenFlag::LITTLE_ENDIAN);

	//
	// Read and check the header
	//
	AnkiTextureHeader header;
	file.read(&header, sizeof(AnkiTextureHeader));

	if(std::memcmp(&header.m_magic[0], "ANKITEX1", 8) != 0)
	{
		throw ANKI_EXCEPTION("Wrong magic word");
	}

	if(header.m_width == 0 
		|| !isPowerOfTwo(header.m_width) 
		|| header.m_width > 4096
		|| header.m_height == 0 
		|| !isPowerOfTwo(header.m_height) 
		|| header.m_height > 4096)
	{
		throw ANKI_EXCEPTION("Incorrect width/height value");
	}

	if(header.m_depth < 1 || header.m_depth > 16)
	{
		throw ANKI_EXCEPTION("Zero or too big depth");
	}

	if(header.m_type < Image::TextureType::_2D 
		|| header.m_type > Image::TextureType::_2D_ARRAY)
	{
		throw ANKI_EXCEPTION("Incorrect header: texture type");
	}

	if(header.m_colorFormat < Image::ColorFormat::RGB8 
		|| header.m_colorFormat > Image::ColorFormat::RGBA8)
	{
		throw ANKI_EXCEPTION("Incorrect header: color format");
	}

	if((header.m_compressionFormats & preferredCompression) 
		== Image::DataCompression::NONE)
	{
		throw ANKI_EXCEPTION("File does not contain the wanted compression");
	}

	if(header.m_normal != 0 && header.m_normal != 1)
	{
		throw ANKI_EXCEPTION("Incorrect header: normal");
	}

	// Check mip levels
	U size = std::min(header.m_width, header.m_height);
	U maxsize = std::max(header.m_width, header.m_height);
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

	if(tmpMipLevels != header.m_mipLevels)
	{
		throw ANKI_EXCEPTION("Incorrect number of mip levels");
	}

	colorFormat = header.m_colorFormat;

	switch(header.m_type)
	{
		case Image::TextureType::_2D:
			depth = 1;
			break;
		case Image::TextureType::CUBE:
			depth = 6;
			break;
		case Image::TextureType::_3D:
		case Image::TextureType::_2D_ARRAY:
			depth = header.m_depth;
			break;
		default:
			ANKI_ASSERT(0);
	}

	textureType = header.m_type;

	//
	// Move file pointer
	//

	if(preferredCompression == Image::DataCompression::RAW)
	{
		// Do nothing
	}
	else if(preferredCompression == Image::DataCompression::S3TC)
	{
		if((header.m_compressionFormats & Image::DataCompression::RAW)
			!= Image::DataCompression::NONE)
		{
			// If raw compression is present then skip it
			file.seek(
				calcSizeOfSegment(header, Image::DataCompression::RAW), 
				File::SeekOrigin::CURRENT);
		}
	}
	else if(preferredCompression == Image::DataCompression::ETC)
	{
		if((header.m_compressionFormats & Image::DataCompression::RAW)
			!= Image::DataCompression::NONE)
		{
			// If raw compression is present then skip it
			file.seek(
				calcSizeOfSegment(header, Image::DataCompression::RAW), 
				File::SeekOrigin::CURRENT);
		}

		if((header.m_compressionFormats & Image::DataCompression::S3TC)
			!= Image::DataCompression::NONE)
		{
			// If s3tc compression is present then skip it
			file.seek(
				calcSizeOfSegment(header, Image::DataCompression::S3TC), 
				File::SeekOrigin::CURRENT);
		}
	}

	//
	// It's time to read
	//

	// Allocate the surfaces
	ResourceAllocator<U8> alloc = surfaces.get_allocator(); 
	surfaces.resize(mipLevels * depth, Image::Surface(alloc));

	// Read all surfaces
	U mipWidth = header.m_width;
	U mipHeight = header.m_height;
	for(U mip = 0; mip < header.m_mipLevels; mip++)
	{
		for(U d = 0; d < depth; d++)
		{
			U dataSize = calcSurfaceSize(
				mipWidth, mipHeight, preferredCompression, 
				header.m_colorFormat);

			// Check if this mipmap can be skipped because of size
			if(std::max(mipWidth, mipHeight) <= maxTextureSize)
			{
				U index = (mip - tmpMipLevels + mipLevels) * depth + d;
				ANKI_ASSERT(index < surfaces.size());
				Image::Surface& surf = surfaces[index];
				surf.m_width = mipWidth;
				surf.m_height = mipHeight;

				surf.m_data.resize(dataSize);
				file.read(&surf.m_data[0], dataSize);
			}
			else
			{
				file.seek(dataSize, File::SeekOrigin::CURRENT);
			}
		}

		mipWidth /= 2;
		mipHeight /= 2;
	}
}

//==============================================================================
// Image                                                                       =
//==============================================================================

//==============================================================================
void Image::load(const char* filename, U32 maxTextureSize)
{
	// get the extension
	const char* ext = File::getFileExtension(filename);
	
	if(ext == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to get filename extension");
	}

	// load from this extension
	try
	{
		U32 bpp;
		m_textureType = TextureType::_2D;
		m_compression = DataCompression::RAW;

		if(std::strcmp(ext, "tga") == 0)
		{
			ResourceAllocator<U8> alloc = m_surfaces.get_allocator(); 
			m_surfaces.resize(1, Surface(alloc));
			m_mipLevels = 1;
			m_depth = 1;
			loadTga(filename, m_surfaces[0].m_width, m_surfaces[0].m_height, 
				bpp, m_surfaces[0].m_data);

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
		else if(std::strcmp(ext, "ankitex") == 0)
		{
#if 0
			compression = Image::DataCompression::RAW;
#elif ANKI_GL == ANKI_GL_DESKTOP
			m_compression = Image::DataCompression::S3TC;
#else
			m_compression = Image::DataCompression::ETC;
#endif

			loadAnkiTexture(filename, maxTextureSize, 
				m_compression, m_surfaces, m_depth, 
				m_mipLevels, m_textureType, m_colorFormat);

		}
		else
		{
			throw ANKI_EXCEPTION("Unsupported extension");
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load image") << e;
	}
}

//==============================================================================
const Image::Surface& Image::getSurface(U mipLevel, U layer) const
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
	
	ANKI_ASSERT(index < m_surfaces.size());

	return m_surfaces[index];
}

} // end namespace anki
