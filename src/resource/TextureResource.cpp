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
// TexUploadTask                                                               =
//==============================================================================

/// Texture upload async task.
class TexUploadTask : public AsyncLoaderTask
{
public:
	ImageLoader m_loader;
	TexturePtr m_tex;
	GrManager* m_gr ANKI_DBG_NULLIFY_PTR;
	U m_depth = 0;
	U m_faces = 0;

	class
	{
	public:
		U m_depth = 0;
		U m_face = 0;
		U m_mip = 0;
	} m_ctx;

	TexUploadTask(GenericMemoryPoolAllocator<U8> alloc)
		: m_loader(alloc)
	{
	}

	Error operator()(AsyncLoaderTaskContext& ctx) final;
};

//==============================================================================
Error TexUploadTask::operator()(AsyncLoaderTaskContext& ctx)
{
	CommandBufferPtr cmdb;

	// Upload the data
	for(U depth = m_ctx.m_depth; depth < m_depth; ++depth)
	{
		for(U face = m_ctx.m_face; face < m_faces; ++face)
		{
			for(U mip = m_ctx.m_mip; mip < m_loader.getMipLevelsCount(); ++mip)
			{
				U surfIdx = max(depth, face);
				const auto& surf = m_loader.getSurface(mip, surfIdx);

				DynamicBufferToken token;
				void* data = m_gr->allocateFrameHostVisibleMemory(
					surf.m_data.getSize(), BufferUsage::TRANSFER, token);

				if(data)
				{
					// There is enough transfer memory

					memcpy(data, &surf.m_data[0], surf.m_data.getSize());

					if(!cmdb)
					{
						cmdb = m_gr->newInstance<CommandBuffer>(
							CommandBufferInitInfo());
					}

					cmdb->textureUpload(
						m_tex, TextureSurfaceInfo(mip, depth, face), token);
				}
				else
				{
					// Not enough transfer memory. Move the work to the future

					if(cmdb)
					{
						cmdb->flush();
					}

					m_ctx.m_depth = depth;
					m_ctx.m_mip = mip;
					m_ctx.m_face = face;

					ctx.m_pause = true;
					ctx.m_resubmitTask = true;

					return ErrorCode::NONE;
				}
			}
		}
	}

	// Finaly enque the command buffer
	if(cmdb)
	{
		cmdb->flush();
	}

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
	TextureInitInfo init;
	U depth = 0;
	U faces = 0;

	// Load image
	TexUploadTask* task = getManager().getAsyncLoader().newTask<TexUploadTask>(
		getManager().getAsyncLoader().getAllocator());
	ImageLoader& loader = task->m_loader;

	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	ANKI_CHECK(loader.load(file, filename, getManager().getMaxTextureSize()));

	// width + height
	const auto& tmpSurf = loader.getSurface(0, 0);
	init.m_width = tmpSurf.m_width;
	init.m_height = tmpSurf.m_height;

	// depth
	if(loader.getTextureType() == ImageLoader::TextureType::_2D_ARRAY
		|| loader.getTextureType() == ImageLoader::TextureType::_3D)
	{
		init.m_depth = loader.getDepth();
	}
	else
	{
		init.m_depth = 1;
	}

	// target
	switch(loader.getTextureType())
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

	if(loader.getColorFormat() == ImageLoader::ColorFormat::RGB8)
	{
		switch(loader.getCompression())
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
	else if(loader.getColorFormat() == ImageLoader::ColorFormat::RGBA8)
	{
		switch(loader.getCompression())
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
	init.m_mipmapsCount = loader.getMipLevelsCount();

	// filteringType
	init.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	init.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;

	// repeat
	init.m_sampling.m_repeat = true;

	// anisotropyLevel
	init.m_sampling.m_anisotropyLevel = getManager().getTextureAnisotropy();

	// Create the texture
	m_tex = getManager().getGrManager().newInstance<Texture>(init);

	// Upload the data asynchronously
	task->m_depth = depth;
	task->m_faces = faces;
	task->m_gr = &getManager().getGrManager();
	task->m_tex = m_tex;

	getManager().getAsyncLoader().submitTask(task);

	// Done
	m_size = UVec3(init.m_width, init.m_height, init.m_depth);
	return ErrorCode::NONE;
}

} // end namespace anki
