#include "anki/resource/TextureResource.h"
#include "anki/resource/Image.h"
#include "anki/util/Exception.h"
#include "anki/renderer/MainRenderer.h"

#if ANKI_GL == ANKI_GL_DESKTOP
#	define DRIVER_CAN_COMPRESS 1
#else
#	define DRIVER_CAN_COMPRESS 0
#endif

namespace anki {

//==============================================================================
void deleteImageCallback(void* data)
{
	Image* image = (Image*)data;
	delete image;
}

//==============================================================================
void TextureResource::load(const char* filename)
{
	try
	{
		loadInternal(filename);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load texture") << e;
	}
}

//==============================================================================
void TextureResource::loadInternal(const char* filename)
{
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl); // Always first to avoid assertions (because of
	                            // the check of the allocator)

	GlTextureHandle::Initializer init;
	U layers = 0;
	Bool driverShouldGenMipmaps = false;

	// Load image
	Image* imgPtr = new Image;
	Image& img = *imgPtr;
	img.load(filename, MainRendererSingleton::get().getMaxTextureSize());
	
	// width + height
	init.m_width = img.getSurface(0, 0).width;
	init.m_height = img.getSurface(0, 0).height;
	
	// depth
	if(img.getTextureType() == Image::TT_2D_ARRAY 
		|| img.getTextureType() == Image::TT_3D)
	{
		init.m_depth = img.getDepth();
	}
	else
	{
		init.m_depth = 0;
	}

	// target
	switch(img.getTextureType())
	{
	case Image::TT_2D:
		init.m_target = GL_TEXTURE_2D;
		layers = 1;
		break;
	case Image::TT_CUBE:
		init.m_target = GL_TEXTURE_CUBE_MAP;
		layers = 6;
		break;
	case Image::TT_2D_ARRAY:
		init.m_target = GL_TEXTURE_2D_ARRAY;
		layers = init.m_depth;
		break;
	case Image::TT_3D:
		init.m_target = GL_TEXTURE_3D;
		layers = init.m_depth;
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
			init.m_internalFormat = GL_COMPRESSED_RGB;
#else
			init.m_internalFormat = GL_RGB;
#endif
			driverShouldGenMipmaps = true;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case Image::DC_S3TC:
			init.m_internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;
#else
		case Image::DC_ETC:
			init.m_internalFormat = GL_COMPRESSED_RGB8_ETC2;
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
			init.m_internalFormat = GL_COMPRESSED_RGBA;
#else
			init.m_internalFormat = GL_RGBA;
#endif
			driverShouldGenMipmaps = true;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case Image::DC_S3TC:
			init.m_internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
#else
		case Image::DC_ETC:
			init.m_internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
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
		init.m_format = GL_RGB;
		break;
	case Image::CF_RGBA8:
		init.m_format = GL_RGBA;
		break;
	default:
		ANKI_ASSERT(0);
	}

	// type
	init.m_type = GL_UNSIGNED_BYTE;

	// mipmapsCount
	init.m_mipmapsCount = img.getMipLevelsCount();

	// filteringType
	init.m_filterType = GlTextureHandle::Filter::TRILINEAR;

	// repeat
	init.m_repeat = true;

	// anisotropyLevel
	//init.anisotropyLevel = XXX;
	printf("TODO\n");

	// genMipmaps
	if(init.m_mipmapsCount == 1 || driverShouldGenMipmaps)
	{
		init.m_genMipmaps = true;
	}
	else
	{
		init.m_genMipmaps = false;
	}

	// Now assign the data
	for(U layer = 0; layer < layers; layer++)
	{
		for(U level = 0; level < init.m_mipmapsCount; level++)
		{
			GlClientBufferHandle& buff = init.m_data[level][layer];

			buff = GlClientBufferHandle(
				jobs, 
				img.getSurface(level, layer).data.size(), 
				(void*)&img.getSurface(level, layer).data[0]);
		}
	}

	// Add the GL job to create the texture
	m_tex = GlTextureHandle(jobs, init);

	// Add cleanup job
	jobs.pushBackUserJob(deleteImageCallback, imgPtr);

	// Finaly enque the GL job chain
	jobs.flush();
}

} // end namespace anki
