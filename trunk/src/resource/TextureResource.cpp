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
	init.width = img.getSurface(0, 0).width;
	init.height = img.getSurface(0, 0).height;
	init.dataSize = img.getSurface(0, 0).data.size();

	Bool compressionEnabled = 
		TextureManagerSingleton::get().getCompressionEnabled();
	(void)compressionEnabled;

	switch(img.getColorFormat())
	{
	case Image::CF_RGB8:
#if DRIVER_CAN_COMPRESS
		init.internalFormat = (compressionEnabled) 
			? GL_COMPRESSED_RGB : GL_RGB;
#else
		init.internalFormat = GL_RGB;
#endif
		init.format = GL_RGB;
		init.type = GL_UNSIGNED_BYTE;
		break;

	case Image::CF_RGBA8:
#if DRIVER_CAN_COMPRESS
		init.internalFormat = (compressionEnabled) 
			? GL_COMPRESSED_RGBA : GL_RGBA;
#else
		init.internalFormat = GL_RGBA;
#endif
		init.format = GL_RGBA;
		init.type = GL_UNSIGNED_BYTE;
		break;

	default:
		ANKI_ASSERT(0);
	}

	switch(img.getCompression())
	{
	case Image::DC_RAW:
		break;

#if ANKI_GL == ANKI_GL_DESKTOP
	case Image::DC_DXT1:
		if(img.getColorFormat() == Image::CF_RGB8)
		{
			init.internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		}
		else
		{
			ANKI_ASSERT(0 && "DXT1 should only be RGB");
		}
		break;

	case Image::DC_DXT5:
		if(img.getColorFormat() == Image::CF_RGBA8)
		{
			init.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		}
		else
		{
			ANKI_ASSERT(0 && "DXT5 should only be RGB");
		}
		break;
#else
	case Image::DC_ETC2:
		ANKI_ASSERT(0 && "ToDo");
		break;
#endif
	default:
		ANKI_ASSERT(0);
	}

	init.data[0] = &img.getSurface(0, 0).data[0];
	init.mipmapping = TextureManagerSingleton::get().getMipmappingEnabled();
	init.filteringType = init.mipmapping ? TFT_TRILINEAR : TFT_LINEAR;
	init.repeat = true;
	init.anisotropyLevel = TextureManagerSingleton::get().getAnisotropyLevel();

	create(init);
}

} // end namespace anki
