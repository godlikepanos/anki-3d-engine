// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/TextureResource.h>
#include <anki/resource/ImageLoader.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/AsyncLoader.h>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

class TexUploadTask : public AsyncLoaderTask
{
public:
	UniquePtr<ImageLoader> m_loader;
	TexturePtr m_tex;
	GrManager* m_gr ANKI_DBG_NULLIFY_PTR;
	U32 m_depth = 0;
	U8 m_faces = 0;

	TexUploadTask(UniquePtr<ImageLoader>& loader,
		TexturePtr tex,
		GrManager* gr,
		U depth,
		U faces)
		: m_loader(std::move(loader))
		, m_tex(tex)
		, m_gr(gr)
		, m_depth(depth)
		, m_faces(faces)
	{
	}

	Error operator()() final;
};

//==============================================================================
Error TexUploadTask::operator()()
{
	CommandBufferPtr cmdb =
		m_gr->newInstance<CommandBuffer>(CommandBufferInitInfo());

	// Upload the data
	for(U depth = 0; depth < m_depth; depth++)
	{
		for(U face = 0; face < m_faces; face++)
		{
			for(U level = 0; level < m_loader->getMipLevelsCount(); level++)
			{
				U surfIdx = max(depth, face);
				const auto& surf = m_loader->getSurface(level, surfIdx);

				DynamicBufferToken token;
				void* data = m_gr->allocateFrameHostVisibleMemory(
					surf.m_data.getSize(), BufferUsage::TRANSFER, token);
				memcpy(data, &surf.m_data[0], surf.m_data.getSize());

				cmdb->textureUpload(
					m_tex, TextureSurfaceInfo(level, depth, face), token);
			}
		}
	}

	// Finaly enque the GL job chain
	// TODO This is probably a bad idea
	cmdb->finish();

	return ErrorCode::NONE;
}

//==============================================================================
// TextureResource                                                             =
//==============================================================================

//==============================================================================
TextureResource::~TextureResource()
{
}

//==============================================================================
Error TextureResource::load(const ResourceFilename& filename)
{
	GrManager& gr = getManager().getGrManager();
	// Always first to avoid assertions (because of the check of the allocator)
	CommandBufferPtr cmdb =
		gr.newInstance<CommandBuffer>(CommandBufferInitInfo());

	TextureInitInfo init;
	U depth = 0;
	U faces = 0;

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
		init.m_depth = 1;
	}

	// target
	switch(img->getTextureType())
	{
	case ImageLoader::TextureType::_2D:
		init.m_type = TextureType::_2D;
		depth = 1;
		faces = 1;
		break;
	case ImageLoader::TextureType::CUBE:
		init.m_type = TextureType::CUBE;
		depth = 1;
		faces = 6;
		break;
	case ImageLoader::TextureType::_2D_ARRAY:
		init.m_type = TextureType::_2D_ARRAY;
		depth = init.m_depth;
		faces = 1;
		break;
	case ImageLoader::TextureType::_3D:
		init.m_type = TextureType::_3D;
		depth = init.m_depth;
		faces = 1;
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
	init.m_sampling.m_anisotropyLevel = getManager().getTextureAnisotropy();

	// Create the texture
	m_tex = gr.newInstance<Texture>(init);

	// Upload the data asynchronously
	getManager().getAsyncLoader().newTask<TexUploadTask>(
		img, m_tex, &gr, depth, faces);

	m_size = UVec3(init.m_width, init.m_height, init.m_depth);

	return ErrorCode::NONE;
}

} // end namespace anki
