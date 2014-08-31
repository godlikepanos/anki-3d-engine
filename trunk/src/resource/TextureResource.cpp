// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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
static void deleteImageCallback(void* data)
{
	Image* image = reinterpret_cast<Image*>(data);
	auto alloc = image->getAllocator();
	alloc.deleteInstance(image);
}

//==============================================================================
void TextureResource::load(const char* filename, ResourceInitializer& init)
{
	try
	{
		loadInternal(filename, init);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load texture") << e;
	}
}

//==============================================================================
void TextureResource::loadInternal(const char* filename, 
	ResourceInitializer& rinit)
{
	GlDevice& gl = rinit.m_gl;
	GlCommandBufferHandle jobs(&gl); // Always first to avoid assertions (
	                                 // because of the check of the allocator)

	GlTextureHandle::Initializer init;
	U layers = 0;
	Bool driverShouldGenMipmaps = false;

	// Load image
	Image* imgPtr = rinit.m_alloc.newInstance<Image>(rinit.m_alloc);
	Image& img = *imgPtr;
	img.load(filename, rinit.m_maxTextureSize);
	
	// width + height
	init.m_width = img.getSurface(0, 0).m_width;
	init.m_height = img.getSurface(0, 0).m_height;
	
	// depth
	if(img.getTextureType() == Image::TextureType::_2D_ARRAY 
		|| img.getTextureType() == Image::TextureType::_3D)
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
	case Image::TextureType::_2D:
		init.m_target = GL_TEXTURE_2D;
		layers = 1;
		break;
	case Image::TextureType::CUBE:
		init.m_target = GL_TEXTURE_CUBE_MAP;
		layers = 6;
		break;
	case Image::TextureType::_2D_ARRAY:
		init.m_target = GL_TEXTURE_2D_ARRAY;
		layers = init.m_depth;
		break;
	case Image::TextureType::_3D:
		init.m_target = GL_TEXTURE_3D;
		layers = init.m_depth;
	default:
		ANKI_ASSERT(0);
	}

	// Internal format
	if(img.getColorFormat() == Image::ColorFormat::RGB8)
	{
		switch(img.getCompression())
		{
		case Image::DataCompression::RAW:
#if DRIVER_CAN_COMPRESS
			init.m_internalFormat = GL_COMPRESSED_RGB;
#else
			init.m_internalFormat = GL_RGB;
#endif
			driverShouldGenMipmaps = true;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case Image::DataCompression::S3TC:
			init.m_internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;
#else
		case Image::DataCompression::ETC:
			init.m_internalFormat = GL_COMPRESSED_RGB8_ETC2;
			break;
#endif
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(img.getColorFormat() == Image::ColorFormat::RGBA8)
	{
		switch(img.getCompression())
		{
		case Image::DataCompression::RAW:
#if DRIVER_CAN_COMPRESS
			init.m_internalFormat = GL_COMPRESSED_RGBA;
#else
			init.m_internalFormat = GL_RGBA;
#endif
			driverShouldGenMipmaps = true;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case Image::DataCompression::S3TC:
			init.m_internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
#else
		case Image::DataCompression::ETC:
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
	case Image::ColorFormat::RGB8:
		init.m_format = GL_RGB;
		break;
	case Image::ColorFormat::RGBA8:
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
	init.m_anisotropyLevel = rinit.m_anisotropyLevel;

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
				img.getSurface(level, layer).m_data.size(), 
				(void*)&img.getSurface(level, layer).m_data[0]);
		}
	}

	// Add the GL job to create the texture
	m_tex = GlTextureHandle(jobs, init);

	// Add cleanup job
	jobs.pushBackUserCommand(deleteImageCallback, imgPtr);

	// Finaly enque the GL job chain
	jobs.flush();
}

} // end namespace anki
