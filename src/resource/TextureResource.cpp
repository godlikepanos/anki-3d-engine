#include "anki/resource/TextureResource.h"
#include "anki/resource/Image.h"
#include "anki/util/Exception.h"

#if ANKI_GL == ANKI_GL_DESKTOP
#	define DRIVER_CAN_COMPRESS 1
#else
#	define DRIVER_CAN_COMPRESS 0
#endif

namespace anki {

//==============================================================================
void TextureResource::load(const char* filename)
{
	try
	{
		Image img;
		img.load(filename);
		load(img);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("File \"" + filename + "\"") << e;
	}
}

//==============================================================================
void TextureResource::load(const Image& img)
{
	Initializer init;
	U layers = 0;
	Bool driverShouldGenMipmaps = false;
	
	// width + height
	init.width = img.getSurface(0, 0).width;
	init.height = img.getSurface(0, 0).height;
	
	// depth
	if(img.getTextureType() == Image::TT_2D_ARRAY 
		|| img.getTextureType() == Image::TT_3D)
	{
		init.depth = img.getDepth();
		ANKI_ASSERT(init.depth > 1);
	}
	else
	{
		init.depth = 0;
	}

	// target
	switch(img.getTextureType())
	{
	case Image::TT_2D:
		init.target = GL_TEXTURE_2D;
		layers = 1;
		break;
	case Image::TT_CUBE:
		init.target = GL_TEXTURE_CUBE_MAP;
		layers = 6;
		break;
	case Image::TT_2D_ARRAY:
		init.target = GL_TEXTURE_2D_ARRAY;
		layers = init.depth;
		break;
	case Image::TT_3D:
		init.target = GL_TEXTURE_3D;
		layers = init.depth;
	default:
		ANKI_ASSERT(0);
	}

	// Internal format
	if(img.getColorFormat() == Image::CF_RGB8)
	{
		switch(img.getCompression())
		{
		case Image::DC_RAW:
#if DRIVER_CAN_COMPRESS
			init.internalFormat = GL_COMPRESSED_RGB;
#else
			init.internalFormat = GL_RGB;
#endif
			driverShouldGenMipmaps = true;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case Image::DC_S3TC:
			init.internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;
#else
		case Image::DC_ETC:
			init.internalFormat = GL_COMPRESSED_RGB8_ETC2;
			break;
#endif
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(img.getColorFormat() == Image::CF_RGBA8)
	{
		switch(img.getCompression())
		{
		case Image::DC_RAW:
#if DRIVER_CAN_COMPRESS
			init.internalFormat = GL_COMPRESSED_RGBA;
#else
			init.internalFormat = GL_RGBA;
#endif
			driverShouldGenMipmaps = true;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case Image::DC_S3TC:
			init.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
#else
		case Image::DC_ETC:
			init.internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			break;
#endif
		default:
			ANKI_ASSERT(0);
		}
	}
	else
	{
		ANKI_ASSERT(0);
	}

	// format
	switch(img.getColorFormat())
	{
	case Image::CF_RGB8:
		init.format = GL_RGB;
		break;
	case Image::CF_RGBA8:
		init.format = GL_RGBA;
		break;
	default:
		ANKI_ASSERT(0);
	}

	// type
	init.type = GL_UNSIGNED_BYTE;

	// mipmapsCount
	init.mipmapsCount = img.getMipLevelsCount();

	// filteringType
	init.filteringType = TFT_TRILINEAR;

	// repeat
	init.repeat = true;

	// anisotropyLevel
	init.anisotropyLevel = TextureManagerSingleton::get().getAnisotropyLevel();

	// genMipmaps
	if(init.mipmapsCount == 1 || driverShouldGenMipmaps)
	{
		init.genMipmaps = true;
	}
	else
	{
		init.genMipmaps = false;
	}

	// Now assign the data
	for(U layer = 0; layer < layers; layer++)
	{
		for(U level = 0; level < init.mipmapsCount; level++)
		{
			init.data[level][layer].ptr = &img.getSurface(level, layer).data[0];
			init.data[level][layer].size = 
				img.getSurface(level, layer).data.size();
		}
	}

	// Finaly create
	create(init);
}

} // end namespace anki
