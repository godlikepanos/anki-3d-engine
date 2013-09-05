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
	File& fs, U32& width, U32& height, U32& bpp, Vector<U8>& data)
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
	File& fs, U32& width, U32& height, U32& bpp, Vector<U8>& data)
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
	U32& width, U32& height, U32& bpp, Vector<U8>& data)
{
	File fs(filename, File::OF_READ | File::OF_BINARY);
	char myTgaHeader[12];

	fs.read(&myTgaHeader[0], sizeof(myTgaHeader));

	if(memcmp(tgaHeaderUncompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		loadUncompressedTga(fs, width, height, bpp, data);
	}
	else if(memcmp(tgaHeaderCompressed, &myTgaHeader[0],
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
struct AnkiTextureHeader
{
	Array<U8, 8> magic;
	U32 width;
	U32 height;
	U32 depth;
	U32 type;
	U32 colorFormat;
	U32 compressionFormats;
	U32 normal;
	U32 mipLevels;
	U8 padding[88];
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
	case Image::DC_RAW:
		out = width * height * ((cf == Image::CF_RGB8) ? 3 : 4);
		break;
	case Image::DC_S3TC:
		out = (width / 4) * (height / 4) 
			* (cf == Image::CF_RGB8) ? 8 : 16; // This is the block size
		break;
	case Image::DC_ETC:
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
	U width = header.width;
	U height = header.height;
	U mips = header.mipLevels;
	U layers = 0;

	switch(header.type)
	{
	case Image::TT_2D:
		layers = 1;
		break;
	case Image::TT_CUBE:
		layers = 6;
		break;
	case Image::TT_2D_ARRAY:
	case Image::TT_3D:
		layers = header.depth;
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
			out += calcSurfaceSize(width, height, comp, 
				(Image::ColorFormat)header.colorFormat);
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
	Vector<Image::Surface>& surfaces, 
	U8& depth, 
	U8& mipLevels, 
	Image::TextureType& textureType,
	Image::ColorFormat& colorFormat)
{
	File file(filename, 
		File::OF_READ | File::OF_BINARY | File::E_LITTLE_ENDIAN);

	//
	// Read and check the header
	//
	AnkiTextureHeader header;
	file.read(&header, sizeof(AnkiTextureHeader));

	if(memcmp(&header.magic[0], "ANKITEX1", 8) != 0)
	{
		throw ANKI_EXCEPTION("Wrong magic word");
	}

	if(header.width == 0 
		|| !isPowerOfTwo(header.width) 
		|| header.width > 4096
		|| header.height == 0 
		|| !isPowerOfTwo(header.height) 
		|| header.height > 4096)
	{
		throw ANKI_EXCEPTION("Incorrect width/height value");
	}

	if(header.depth < 1 || header.depth > 16)
	{
		throw ANKI_EXCEPTION("Zero or too big depth");
	}

	if(header.type < Image::TT_2D || header.type > Image::TT_2D_ARRAY)
	{
		throw ANKI_EXCEPTION("Incorrect header: texture type");
	}

	if(header.colorFormat < Image::CF_RGB8 
		|| header.colorFormat > Image::CF_RGBA8)
	{
		throw ANKI_EXCEPTION("Incorrect header: color format");
	}

	if((header.compressionFormats & preferredCompression) == 0)
	{
		throw ANKI_EXCEPTION("File does not contain the wanted compression");
	}

	if(header.normal != 0 && header.normal != 1)
	{
		throw ANKI_EXCEPTION("Incorrect header: normal");
	}

	// Check mip levels
	U size = std::min(header.width, header.height);
	U maxsize = std::max(header.width, header.height);
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

	if(tmpMipLevels != header.mipLevels)
	{
		throw ANKI_EXCEPTION("Incorrect number of mip levels");
	}

	colorFormat = (Image::ColorFormat)header.colorFormat;

	switch(header.type)
	{
		case Image::TT_2D:
			depth = 1;
			break;
		case Image::TT_CUBE:
			depth = 6;
			break;
		case Image::TT_3D:
		case Image::TT_2D_ARRAY:
			depth = header.depth;
			break;
		default:
			ANKI_ASSERT(0);
	}

	textureType = (Image::TextureType)header.type;

	//
	// Move file pointer
	//

	if(preferredCompression == Image::DC_RAW)
	{
		// Do nothing
	}
	else if(preferredCompression == Image::DC_S3TC)
	{
		if(header.compressionFormats & Image::DC_RAW)
		{
			// If raw compression is present then skip it
			file.seek(
				calcSizeOfSegment(header, Image::DC_RAW), File::SO_CURRENT);
		}
	}
	else if(preferredCompression == Image::DC_ETC)
	{
		if(header.compressionFormats & Image::DC_RAW)
		{
			// If raw compression is present then skip it
			file.seek(
				calcSizeOfSegment(header, Image::DC_RAW), File::SO_CURRENT);
		}

		if(header.compressionFormats & Image::DC_S3TC)
		{
			// If s3tc compression is present then skip it
			file.seek(
				calcSizeOfSegment(header, Image::DC_S3TC), File::SO_CURRENT);
		}
	}

	//
	// It's time to read
	//

	// Allocate the surfaces
	surfaces.resize(mipLevels * depth);

	// Read all surfaces
	U mipWidth = header.width;
	U mipHeight = header.height;
	for(U mip = 0; mip < header.mipLevels; mip++)
	{
		for(U d = 0; d < depth; d++)
		{
			U dataSize = 0;
			switch(preferredCompression)
			{
			case Image::DC_RAW:
				dataSize = mipWidth * mipHeight 
					* ((header.colorFormat == Image::CF_RGB8) ? 3 : 4);
				break;
			case Image::DC_S3TC:
				dataSize = (mipWidth / 4) * (mipHeight / 4)
					* ((header.colorFormat == Image::CF_RGB8) ? 8 : 16);
				break;
			case Image::DC_ETC:
				dataSize = (mipWidth / 4) * (mipHeight / 4) * 8;
				break;
			default:
				ANKI_ASSERT(0);
			}

			if(std::max(mipWidth, mipHeight) <= maxTextureSize)
			{
				U index = (mip - tmpMipLevels + mipLevels) * depth + d;
				ANKI_ASSERT(index < surfaces.size());
				Image::Surface& surf = surfaces[index];
				surf.width = mipWidth;
				surf.height = mipHeight;

				surf.data.resize(dataSize);
				file.read(&surf.data[0], dataSize);
			}
			else
			{
				file.seek(dataSize, File::SO_CURRENT);
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
	ANKI_ASSERT(ext);

	// load from this extension
	try
	{
		U32 bpp;
		textureType = TT_2D;
		compression = DC_RAW;

		if(strcmp(ext, "tga") == 0)
		{
			surfaces.resize(1);
			mipLevels = 1;
			depth = 1;
			loadTga(filename, surfaces[0].width, surfaces[0].height, bpp, 
				surfaces[0].data);

			if(bpp == 32)
			{
				colorFormat = CF_RGBA8;
			}
			else if(bpp == 24)
			{
				colorFormat = CF_RGB8;
			}
			else
			{
				ANKI_ASSERT(0);
			}
		}
		else if(strcmp(ext, "ankitex") == 0)
		{
#if 0
			compression = Image::DC_RAW;
#elif ANKI_GL == ANKI_GL_DESKTOP
			compression = Image::DC_S3TC;
#else
			compression = Image::DC_ETC;
#endif

			loadAnkiTexture(filename, maxTextureSize, 
				compression, surfaces, depth, 
				mipLevels, textureType, colorFormat);

		}
		else
		{
			throw ANKI_EXCEPTION("Unsupported extension");
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("File " + filename) << e;
	}
}

//==============================================================================
const Image::Surface& Image::getSurface(U mipLevel, U layer) const
{
	ANKI_ASSERT(mipLevel < mipLevels);

	U layers = 0;

	switch(textureType)
	{
	case TT_2D:
		layers = 1;
		break;
	case TT_CUBE:
		layers = 6;
		break;
	case TT_3D:
	case TT_2D_ARRAY:
		layers = depth;
		break;
	default:
		ANKI_ASSERT(0);
	}


	// [mip][depthFace]
	U index = mipLevel * layers + layer;
	
	ANKI_ASSERT(index < surfaces.size());

	return surfaces[index];
}

} // end namespace anki
