// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/TextureResource.h>
#include <anki/resource/ImageLoader.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/AsyncLoader.h>

namespace anki
{

class TextureResource::LoadingContext
{
public:
	ImageLoader m_loader;
	U m_faces = 0;
	U m_layerCount = 0;
	GrManager* m_gr ANKI_DBG_NULLIFY;
	TransferGpuAllocator* m_trfAlloc ANKI_DBG_NULLIFY;
	TextureType m_texType;
	TexturePtr m_tex;

	LoadingContext(GenericMemoryPoolAllocator<U8> alloc)
		: m_loader(alloc)
	{
	}
};

/// Texture upload async task.
class TextureResource::TexUploadTask : public AsyncLoaderTask
{
public:
	TextureResource::LoadingContext m_ctx;

	TexUploadTask(GenericMemoryPoolAllocator<U8> alloc)
		: m_ctx(alloc)
	{
	}

	Error operator()(AsyncLoaderTaskContext& ctx) final
	{
		return TextureResource::load(m_ctx);
	}
};

TextureResource::~TextureResource()
{
}

Error TextureResource::load(const ResourceFilename& filename, Bool async)
{
	TexUploadTask* task;
	LoadingContext* ctx;
	LoadingContext localCtx(getTempAllocator());

	if(async)
	{
		task = getManager().getAsyncLoader().newTask<TexUploadTask>(getManager().getAsyncLoader().getAllocator());
		ctx = &task->m_ctx;
	}
	else
	{
		task = nullptr;
		ctx = &localCtx;
	}
	ImageLoader& loader = ctx->m_loader;

	TextureInitInfo init;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION
		| TextureUsageBit::TRANSFER_DESTINATION;
	init.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION;
	U faces = 0;

	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	ANKI_CHECK(loader.load(file, filename, getManager().getMaxTextureSize()));

	// Various sizes
	init.m_width = loader.getWidth();
	init.m_height = loader.getHeight();

	switch(loader.getTextureType())
	{
	case ImageLoader::TextureType::_2D:
		init.m_type = TextureType::_2D;
		init.m_depth = 1;
		faces = 1;
		init.m_layerCount = 1;
		break;
	case ImageLoader::TextureType::CUBE:
		init.m_type = TextureType::CUBE;
		init.m_depth = 1;
		faces = 6;
		init.m_layerCount = 1;
		break;
	case ImageLoader::TextureType::_2D_ARRAY:
		init.m_type = TextureType::_2D_ARRAY;
		init.m_layerCount = loader.getLayerCount();
		init.m_depth = 1;
		faces = 1;
		break;
	case ImageLoader::TextureType::_3D:
		init.m_type = TextureType::_3D;
		init.m_depth = loader.getDepth();
		init.m_layerCount = 1;
		faces = 1;
		break;
	default:
		ANKI_ASSERT(0);
	}

	// Internal format
	init.m_format.m_transform = TransformFormat::UNORM;

	if(loader.getColorFormat() == ImageLoader::ColorFormat::RGB8)
	{
		switch(loader.getCompression())
		{
		case ImageLoader::DataCompression::RAW:
			init.m_format.m_components = ComponentFormat::R8G8B8;
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case ImageLoader::DataCompression::S3TC:
			init.m_format.m_components = ComponentFormat::R8G8B8_BC1;
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
			init.m_format.m_components = ComponentFormat::R8G8B8A8_BC3;
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

	// Anisotropy
	init.m_sampling.m_anisotropyLevel = getManager().getTextureAnisotropy();

	// Create the texture
	m_tex = getManager().getGrManager().newInstance<Texture>(init);

	// Set the context
	ctx->m_faces = faces;
	ctx->m_layerCount = init.m_layerCount;
	ctx->m_gr = &getManager().getGrManager();
	ctx->m_trfAlloc = &getManager().getTransferGpuAllocator();
	ctx->m_texType = init.m_type;
	ctx->m_tex = m_tex;

	// Upload the data
	if(async)
	{
		getManager().getAsyncLoader().submitTask(task);
	}
	else
	{
		ANKI_CHECK(load(*ctx));
	}

	m_size = UVec3(init.m_width, init.m_height, init.m_depth);
	m_layerCount = init.m_layerCount;
	return ErrorCode::NONE;
}

Error TextureResource::load(LoadingContext& ctx)
{
	const U copyCount = ctx.m_layerCount * ctx.m_faces * ctx.m_loader.getMipLevelsCount();

	for(U b = 0; b < copyCount; b += MAX_COPIES_BEFORE_FLUSH)
	{
		const U begin = b;
		const U end = min(copyCount, b + MAX_COPIES_BEFORE_FLUSH);

		CommandBufferInitInfo ci;
		ci.m_flags = CommandBufferFlag::TRANSFER_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = ctx.m_gr->newInstance<CommandBuffer>(ci);

		// Set the barriers of the batch
		for(U i = begin; i < end; ++i)
		{
			U mip, layer, face;
			unflatten3dArrayIndex(ctx.m_layerCount, ctx.m_faces, ctx.m_loader.getMipLevelsCount(), i, layer, face, mip);

			if(ctx.m_texType == TextureType::_3D)
			{
				TextureVolumeInfo vol(mip);
				cmdb->setTextureVolumeBarrier(
					ctx.m_tex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, vol);
			}
			else
			{
				TextureSurfaceInfo surf(mip, 0, face, layer);
				cmdb->setTextureSurfaceBarrier(
					ctx.m_tex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, surf);
			}
		}

		// Do the copies
		Array<TransferGpuAllocatorHandle, MAX_COPIES_BEFORE_FLUSH> handles;
		U handleCount = 0;
		for(U i = begin; i < end; ++i)
		{
			U mip, layer, face;
			unflatten3dArrayIndex(ctx.m_layerCount, ctx.m_faces, ctx.m_loader.getMipLevelsCount(), i, layer, face, mip);

			PtrSize surfOrVolSize;
			const void* surfOrVolData;
			PtrSize allocationSize;

			if(ctx.m_texType == TextureType::_3D)
			{
				const auto& vol = ctx.m_loader.getVolume(mip);
				surfOrVolSize = vol.m_data.getSize();
				surfOrVolData = &vol.m_data[0];

				ctx.m_gr->getTextureVolumeUploadInfo(ctx.m_tex, TextureVolumeInfo(mip), allocationSize);
			}
			else
			{
				const auto& surf = ctx.m_loader.getSurface(mip, face, layer);
				surfOrVolSize = surf.m_data.getSize();
				surfOrVolData = &surf.m_data[0];

				ctx.m_gr->getTextureSurfaceUploadInfo(
					ctx.m_tex, TextureSurfaceInfo(mip, 0, face, layer), allocationSize);
			}

			ANKI_ASSERT(allocationSize >= surfOrVolSize);
			TransferGpuAllocatorHandle& handle = handles[handleCount++];
			ANKI_CHECK(ctx.m_trfAlloc->allocate(allocationSize, handle));
			void* data = handle.getMappedMemory();
			ANKI_ASSERT(data);

			memcpy(data, surfOrVolData, surfOrVolSize);

			if(ctx.m_texType == TextureType::_3D)
			{
				TextureVolumeInfo vol(mip);
				cmdb->copyBufferToTextureVolume(
					handle.getBuffer(), handle.getOffset(), handle.getRange(), ctx.m_tex, vol);
			}
			else
			{
				TextureSurfaceInfo surf(mip, 0, face, layer);

				cmdb->copyBufferToTextureSurface(
					handle.getBuffer(), handle.getOffset(), handle.getRange(), ctx.m_tex, surf);
			}
		}

		// Set the barriers of the batch
		for(U i = begin; i < end; ++i)
		{
			U mip, layer, face;
			unflatten3dArrayIndex(ctx.m_layerCount, ctx.m_faces, ctx.m_loader.getMipLevelsCount(), i, layer, face, mip);

			if(ctx.m_texType == TextureType::_3D)
			{
				TextureVolumeInfo vol(mip);
				cmdb->setTextureVolumeBarrier(ctx.m_tex,
					TextureUsageBit::TRANSFER_DESTINATION,
					TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION,
					vol);
			}
			else
			{
				TextureSurfaceInfo surf(mip, 0, face, layer);
				cmdb->setTextureSurfaceBarrier(ctx.m_tex,
					TextureUsageBit::TRANSFER_DESTINATION,
					TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION,
					surf);
			}
		}

		// Flush batch
		FencePtr fence;
		cmdb->flush(&fence);

		for(U i = 0; i < handleCount; ++i)
		{
			ctx.m_trfAlloc->release(handles[i], fence);
		}
		cmdb.reset(nullptr);
	}

	return ErrorCode::NONE;
}

} // end namespace anki
