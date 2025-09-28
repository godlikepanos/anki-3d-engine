// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ImageLoader.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

class ImageResource::LoadingContext
{
public:
	ImageLoader m_loader{&ResourceMemoryPool::getSingleton()};
	ImageResourcePtr m_image;
};

/// Image upload async task.
class ImageResource::TexUploadTask : public AsyncLoaderTask
{
public:
	ImageResource::LoadingContext m_ctx;

	Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
	{
		return m_ctx.m_image->loadAsync(m_ctx);
	}

	static BaseMemoryPool& getMemoryPool()
	{
		return ResourceMemoryPool::getSingleton();
	}
};

ImageResource::~ImageResource()
{
}

Error ImageResource::load(const ResourceFilename& filename, Bool async)
{
	UniquePtr<TexUploadTask> task;
	LoadingContext* ctx;
	LoadingContext localCtx;

	if(async)
	{
		task.reset(AsyncLoader::getSingleton().newTask<TexUploadTask>());
		ctx = &task->m_ctx;
		ctx->m_image.reset(this);
	}
	else
	{
		ctx = &localCtx;
	}
	ImageLoader& loader = ctx->m_loader;

	String filenameExt;
	getFilepathFilename(filename, filenameExt);

	TextureInitInfo init(filenameExt);
	init.m_usage = TextureUsageBit::kAllSrv | TextureUsageBit::kCopyDestination;
	U32 faces = 0;

	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	ANKI_CHECK(loader.load(file, filename, g_cvarRsrcMaxImageSize));

	m_avgColor = loader.getAverageColor();

	// Various sizes
	init.m_width = loader.getWidth();
	init.m_height = loader.getHeight();

	switch(loader.getImageType())
	{
	case ImageBinaryType::k2D:
		init.m_type = TextureType::k2D;
		init.m_depth = 1;
		faces = 1;
		init.m_layerCount = 1;
		break;
	case ImageBinaryType::kCube:
		init.m_type = TextureType::kCube;
		init.m_depth = 1;
		faces = 6;
		init.m_layerCount = 1;
		break;
	case ImageBinaryType::k2DArray:
		init.m_type = TextureType::k2DArray;
		init.m_layerCount = loader.getLayerCount();
		init.m_depth = 1;
		faces = 1;
		break;
	case ImageBinaryType::k3D:
		init.m_type = TextureType::k3D;
		init.m_depth = loader.getDepth();
		init.m_layerCount = 1;
		faces = 1;
		break;
	default:
		ANKI_ASSERT(0);
	}

	// Internal format
	if(loader.getColorFormat() == ImageBinaryColorFormat::kRgb8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			init.m_format = Format::kR8G8B8_Unorm;
			break;
		case ImageBinaryDataCompression::kS3tc:
			init.m_format = Format::kBC1_Rgba_Unorm_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				init.m_format = Format::kASTC_4x4_Unorm_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				init.m_format = Format::kASTC_8x8_Unorm_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kRgba8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			init.m_format = Format::kR8G8B8A8_Unorm;
			break;
		case ImageBinaryDataCompression::kS3tc:
			init.m_format = Format::kBC3_Unorm_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				init.m_format = Format::kASTC_4x4_Unorm_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				init.m_format = Format::kASTC_8x8_Unorm_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kRgbFloat)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kS3tc:
			init.m_format = Format::kBC6H_Ufloat_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
			init.m_format = Format::kASTC_8x8_Sfloat_Block;
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kRgbaFloat)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			init.m_format = Format::kR32G32B32A32_Sfloat;
			break;
		case ImageBinaryDataCompression::kAstc:
			ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
			init.m_format = Format::kASTC_8x8_Sfloat_Block;
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else
	{
		ANKI_ASSERT(0);
	}

	// mipmapsCount
	init.m_mipmapCount = U8(loader.getMipmapCount());

	// Create the texture
	m_tex = GrManager::getSingleton().newTexture(init);

	// Upload the data
	if(async)
	{
		TexUploadTask* pTask;
		task.moveAndReset(pTask);
		AsyncLoader::getSingleton().submitTask(pTask, AsyncLoaderPriority::kMedium);
	}
	else
	{
		ANKI_CHECK(loadAsync(*ctx));
	}

	return Error::kNone;
}

Error ImageResource::loadAsync(LoadingContext& ctx) const
{
	const U32 faceCount = textureTypeIsCube(m_tex->getTextureType()) ? 6 : 1;
	const U32 copyCount = m_tex->getLayerCount() * faceCount * ctx.m_loader.getMipmapCount();

	for(U32 b = 0; b < copyCount; b += kMaxCopiesBeforeFlush)
	{
		const U32 begin = b;
		const U32 end = min(copyCount, b + kMaxCopiesBeforeFlush);

		CommandBufferInitInfo ci;
		ci.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(ci);

		// Set the barriers of the batch
		Array<TextureBarrierInfo, kMaxCopiesBeforeFlush> barriers;
		U32 barrierCount = 0;
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(m_tex->getLayerCount(), faceCount, ctx.m_loader.getMipmapCount(), i, layer, face, mip);

			barriers[barrierCount++] = {TextureView(m_tex.get(), TextureSubresourceDesc::surface(mip, face, layer)), TextureUsageBit::kAllSrv,
										TextureUsageBit::kCopyDestination};
		}
		cmdb->setPipelineBarrier({&barriers[0], barrierCount}, {}, {});

		// Do the copies
		Array<TransferGpuAllocatorHandle, kMaxCopiesBeforeFlush> handles;
		U32 handleCount = 0;
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(m_tex->getLayerCount(), faceCount, ctx.m_loader.getMipmapCount(), i, layer, face, mip);

			PtrSize surfOrVolSize;
			const void* surfOrVolData;
			PtrSize allocationSize;

			if(m_tex->getTextureType() == TextureType::k3D)
			{
				const auto& vol = ctx.m_loader.getVolume(mip);
				surfOrVolSize = vol.m_data.getSize();
				surfOrVolData = &vol.m_data[0];

				allocationSize = computeVolumeSize(m_tex->getWidth() >> mip, m_tex->getHeight() >> mip, m_tex->getDepth() >> mip, m_tex->getFormat());
			}
			else
			{
				const auto& surf = ctx.m_loader.getSurface(mip, face, layer);
				surfOrVolSize = surf.m_data.getSize();
				surfOrVolData = &surf.m_data[0];

				allocationSize = computeSurfaceSize(m_tex->getWidth() >> mip, m_tex->getHeight() >> mip, m_tex->getFormat());
			}

			ANKI_ASSERT(allocationSize >= surfOrVolSize);
			TransferGpuAllocatorHandle& handle = handles[handleCount++];
			ANKI_CHECK(TransferGpuAllocator::getSingleton().allocate(allocationSize, handle));
			void* data = handle.getMappedMemory();
			ANKI_ASSERT(data);

			memcpy(data, surfOrVolData, surfOrVolSize);

			// Create temp tex view
			const TextureSubresourceDesc subresource = TextureSubresourceDesc::surface(mip, face, layer);
			cmdb->copyBufferToTexture(handle, TextureView(m_tex.get(), subresource));
		}

		// Set the barriers of the batch
		barrierCount = 0;
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(m_tex->getLayerCount(), faceCount, ctx.m_loader.getMipmapCount(), i, layer, face, mip);

			barriers[barrierCount++] = {TextureView(m_tex.get(), TextureSubresourceDesc::surface(mip, face, layer)),
										TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel | TextureUsageBit::kSrvGeometry};
		}
		cmdb->setPipelineBarrier({&barriers[0], barrierCount}, {}, {});

		// Flush batch
		FencePtr fence;
		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get(), {}, &fence);

		for(U i = 0; i < handleCount; ++i)
		{
			TransferGpuAllocator::getSingleton().release(handles[i], fence);
		}
		cmdb.reset(nullptr);
	}

	m_loadedMipCount.store(m_tex->getMipmapCount());
	return Error::kNone;
}

} // end namespace anki
