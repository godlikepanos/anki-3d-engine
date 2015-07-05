// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/TextureResource.h"
#include "anki/resource/ImageLoader.h"
#include "anki/resource/ResourceManager.h"

namespace anki {

//==============================================================================
TextureResource::~TextureResource()
{}

//==============================================================================
Error TextureResource::load(const ResourceFilename& filename)
{
	GrManager& gr = getManager().getGrManager();
	// Always first to avoid assertions (because of the check of the allocator)
	CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>();

	TextureInitializer init;
	U layers = 0;

	// Load image
	UniquePtr<ImageLoader> img;
	img.reset(getAllocator().newInstance<ImageLoader>(getAllocator()));

	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	ANKI_CHECK(img->load(file, filename, getManager().getMaxTextureSize()));

	// width + height
	const auto& tmpSurf = img->getSurface(0, 0);
	init.m_width = tmpSurf.m_width;
	init.m_height = tmpSurf.m_height;

	// depth
	if(img->getTextureType() == ImageLoader::TextureType::_2D_ARRAY
		|| img->getTextureType() == ImageLoader::TextureType::_3D)
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
	case ImageLoader::TextureType::_2D:
		init.m_type = TextureType::_2D;
		layers = 1;
		break;
	case ImageLoader::TextureType::CUBE:
		init.m_type = TextureType::CUBE;
		layers = 6;
		break;
	case ImageLoader::TextureType::_2D_ARRAY:
		init.m_type = TextureType::_2D_ARRAY;
		layers = init.m_depth;
		break;
	case ImageLoader::TextureType::_3D:
		init.m_type = TextureType::_3D;
		layers = init.m_depth;
		break;
	default:
		ANKI_ASSERT(0);
	}

	// Internal format
	init.m_format.m_transform = TransformFormat::UNORM;
	init.m_format.m_srgb = false;

	if(img->getColorFormat() == ImageLoader::ColorFormat::RGB8)
	{
		switch(img->getCompression())
		{
		case ImageLoader::DataCompression::RAW:
			init.m_format.m_components = ComponentFormat::R8G8B8;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case ImageLoader::DataCompression::S3TC:
			init.m_format.m_components = ComponentFormat::R8G8B8_S3TC;
			break;
#else
		case ImageLoader::DataCompression::ETC:
			init.m_format.m_components = ComponentFormat::R8G8B8_ETC2;
			break;
#endif
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(img->getColorFormat() == ImageLoader::ColorFormat::RGBA8)
	{
		switch(img->getCompression())
		{
		case ImageLoader::DataCompression::RAW:
			init.m_format.m_components = ComponentFormat::R8G8B8A8;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case ImageLoader::DataCompression::S3TC:
			init.m_format.m_components = ComponentFormat::R8G8B8A8_S3TC;
			break;
#else
		case ImageLoader::DataCompression::ETC:
			init.m_format.m_components = ComponentFormat::R8G8B8A8_ETC2;
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
	init.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	init.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;

	// repeat
	init.m_sampling.m_repeat = true;

	// anisotropyLevel
	init.m_sampling.m_anisotropyLevel =
		getManager().getTextureAnisotropy();

	// Create the texture
	m_tex = gr.newInstance<Texture>(init);

	// Upload the data
	for(U layer = 0; layer < layers; layer++)
	{
		for(U level = 0; level < init.m_mipmapsCount; level++)
		{
			const auto& surf = img->getSurface(level, layer);

			void* data;
			cmdb->textureUpload(
				m_tex, level, layer, surf.m_data.getSize(), data);

			memcpy(data, &surf.m_data[0], surf.m_data.getSize());
		}
	}

	// Finaly enque the GL job chain
	// TODO Make asynchronous
	cmdb->finish();

	m_size = UVec3(init.m_width, init.m_height, init.m_depth);

	return ErrorCode::NONE;
}

} // end namespace anki
