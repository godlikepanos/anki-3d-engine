// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/TextureResource.h"
#include "anki/resource/Image.h"
#include "anki/resource/ResourceManager.h"

namespace anki {

//==============================================================================
static Error deleteImageCallback(void* data)
{
	Image* image = reinterpret_cast<Image*>(data);
	auto alloc = image->getAllocator();
	alloc.deleteInstance(image);
	return ErrorCode::NONE;
}

//==============================================================================
TextureResource::TextureResource(ResourceAllocator<U8>&)
{}

//==============================================================================
TextureResource::~TextureResource()
{}

//==============================================================================
Error TextureResource::load(const CString& filename, ResourceInitializer& rinit)
{
	GrManager& gr = rinit.m_resources.getGrManager();
	CommandBufferHandle cmdb;
	Error err = cmdb.create(&gl); // Always first to avoid assertions (
	                              // because of the check of the allocator)
	if(err)
	{
		return err;
	}

	GlTextureHandle::Initializer init;
	U layers = 0;

	// Load image
	Image* img = rinit.m_alloc.newInstance<Image>(rinit.m_alloc);
	if(img == nullptr)
	{
		return ErrorCode::OUT_OF_MEMORY;
	}

	err = img->load(filename, rinit.m_resources.getMaxTextureSize());
	if(err)
	{
		rinit.m_alloc.deleteInstance(img);
		return err;
	}
	
	// width + height
	const auto& tmpSurf = img->getSurface(0, 0);
	init.m_width = tmpSurf.m_width;
	init.m_height = tmpSurf.m_height;
	
	// depth
	if(img->getTextureType() == Image::TextureType::_2D_ARRAY 
		|| img->getTextureType() == Image::TextureType::_3D)
	{
		init.m_depth = img->getDepth();
	}
	else
	{
		init.m_depth = 0;
	}

	// target
	switch(img->getTextureType())
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
	if(img->getColorFormat() == Image::ColorFormat::RGB8)
	{
		switch(img->getCompression())
		{
		case Image::DataCompression::RAW:
			init.m_internalFormat = GL_RGB;
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
	else if(img->getColorFormat() == Image::ColorFormat::RGBA8)
	{
		switch(img->getCompression())
		{
		case Image::DataCompression::RAW:
			init.m_internalFormat = GL_RGBA;
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

	// mipmapsCount
	init.m_mipmapsCount = img->getMipLevelsCount();

	// filteringType
	init.m_filterType = GlTextureHandle::Filter::TRILINEAR;

	// repeat
	init.m_repeat = true;

	// anisotropyLevel
	init.m_anisotropyLevel = rinit.m_resources.getTextureAnisotropy();

	// Now assign the data
	for(U layer = 0; layer < layers; layer++)
	{
		for(U level = 0; level < init.m_mipmapsCount; level++)
		{
			GlClientBufferHandle& buff = init.m_data[level][layer];
			const auto& surf = img->getSurface(level, layer);

			err = buff.create(
				cmdb, 
				surf.m_data.getSize(), 
				const_cast<U8*>(&surf.m_data[0]));

			if(err)
			{
				rinit.m_alloc.deleteInstance(img);
				return err;
			}
		}
	}

	// Add the GL job to create the texture
	err = m_tex.create(cmdb, init);
	if(err)
	{
		rinit.m_alloc.deleteInstance(img);
		return err;
	}

	// Add cleanup job
	cmdb.pushBackUserCommand(deleteImageCallback, img);

	// Finaly enque the GL job chain
	cmdb.flush();

	m_size = UVec3(init.m_width, init.m_height, init.m_depth);

	return err;
}

} // end namespace anki
