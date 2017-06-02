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

/// Texture upload async task.
class TexUploadTask : public AsyncLoaderTask
{
public:
	static constexpr U MAX_COPIES_BEFORE_FLUSH = 8;

	ImageLoader m_loader;
	TexturePtr m_tex;
	GrManager* m_gr ANKI_DBG_NULLIFY;
	TransferGpuAllocator* m_transferAlloc ANKI_DBG_NULLIFY;
	U m_layers = 0;
	U m_faces = 0;
	TextureType m_texType;

	TexUploadTask(GenericMemoryPoolAllocator<U8> alloc)
		: m_loader(alloc)
	{
	}

	Error operator()(AsyncLoaderTaskContext& ctx) final;

	void flush(WeakArray<TransferGpuAllocatorHandle> handles, CommandBufferPtr& cmdb);
};

void TexUploadTask::flush(WeakArray<TransferGpuAllocatorHandle> handles, CommandBufferPtr& cmdb)
{
	FencePtr fence;
	cmdb->flush(&fence);

	for(TransferGpuAllocatorHandle& handle : handles)
	{
		m_transferAlloc->release(handle, fence);
	}

	cmdb.reset(nullptr);
}

Error TexUploadTask::operator()(AsyncLoaderTaskContext& ctx)
{
	CommandBufferPtr cmdb;

	Array<TransferGpuAllocatorHandle, MAX_COPIES_BEFORE_FLUSH> handles;
	U handleCount = 0;

	// Upload the data
	for(U layer = 0; layer < m_layers; ++layer)
	{
		for(U face = 0; face < m_faces; ++face)
		{
			for(U mip = 0; mip < m_loader.getMipLevelsCount(); ++mip)
			{
				if(!cmdb)
				{
					CommandBufferInitInfo ci;
					ci.m_flags = CommandBufferFlag::TRANSFER_WORK | CommandBufferFlag::SMALL_BATCH;
					cmdb = m_gr->newInstance<CommandBuffer>(ci);
				}

				PtrSize surfOrVolSize;
				const void* surfOrVolData;
				PtrSize allocationSize;

				if(m_texType == TextureType::_3D)
				{
					const auto& vol = m_loader.getVolume(mip);
					surfOrVolSize = vol.m_data.getSize();
					surfOrVolData = &vol.m_data[0];

					m_gr->getTextureVolumeUploadInfo(m_tex, TextureVolumeInfo(mip), allocationSize);
				}
				else
				{
					const auto& surf = m_loader.getSurface(mip, face, layer);
					surfOrVolSize = surf.m_data.getSize();
					surfOrVolData = &surf.m_data[0];

					m_gr->getTextureSurfaceUploadInfo(m_tex, TextureSurfaceInfo(mip, 0, face, layer), allocationSize);
				}

				ANKI_ASSERT(allocationSize >= surfOrVolSize);

				TransferGpuAllocatorHandle& handle = handles[handleCount++];
				ANKI_CHECK(m_transferAlloc->allocate(allocationSize, handle));
				void* data = handle.getMappedMemory();
				ANKI_ASSERT(data);

				memcpy(data, surfOrVolData, surfOrVolSize);

				if(m_texType == TextureType::_3D)
				{
					TextureVolumeInfo vol(mip);

					cmdb->setTextureVolumeBarrier(
						m_tex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, vol);

					cmdb->copyBufferToTextureVolume(
						handle.getBuffer(), handle.getOffset(), handle.getRange(), m_tex, vol);

					cmdb->setTextureVolumeBarrier(m_tex,
						TextureUsageBit::TRANSFER_DESTINATION,
						TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION,
						vol);
				}
				else
				{
					TextureSurfaceInfo surf(mip, 0, face, layer);

					cmdb->setTextureSurfaceBarrier(
						m_tex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, surf);

					cmdb->copyBufferToTextureSurface(
						handle.getBuffer(), handle.getOffset(), handle.getRange(), m_tex, surf);

					cmdb->setTextureSurfaceBarrier(m_tex,
						TextureUsageBit::TRANSFER_DESTINATION,
						TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION,
						surf);
				}

				// Check if you should flush the batch
				if(handleCount == handles.getSize())
				{
					flush({&handles[0], handleCount}, cmdb);
					handleCount = 0;
				}
			}
		}
	}

	// Enque what remains
	if(handleCount)
	{
		flush({&handles[0], handleCount}, cmdb);
	}

	return ErrorCode::NONE;
}

TextureResource::~TextureResource()
{
}

Error TextureResource::load(const ResourceFilename& filename)
{
	TextureInitInfo init;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION
		| TextureUsageBit::TRANSFER_DESTINATION;
	init.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION;
	U faces = 0;

	// Load image
	TexUploadTask* task =
		getManager().getAsyncLoader().newTask<TexUploadTask>(getManager().getAsyncLoader().getAllocator());
	ImageLoader& loader = task->m_loader;

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

	// Upload the data asynchronously
	task->m_layers = init.m_layerCount;
	task->m_faces = faces;
	task->m_gr = &getManager().getGrManager();
	task->m_transferAlloc = &getManager().getTransferGpuAllocator();
	task->m_tex = m_tex;
	task->m_texType = init.m_type;

	getManager().getAsyncLoader().submitTask(task);

	// Done
	m_size = UVec3(init.m_width, init.m_height, init.m_depth);
	m_layerCount = init.m_layerCount;
	return ErrorCode::NONE;
}

} // end namespace anki
