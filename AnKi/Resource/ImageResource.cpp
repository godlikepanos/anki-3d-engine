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
	U32 m_faces = 0;
	U32 m_layerCount = 0;
	TextureType m_texType;
	TexturePtr m_tex;
};

/// Image upload async task.
class ImageResource::TexUploadTask : public AsyncLoaderTask
{
public:
	ImageResource::LoadingContext m_ctx;

	Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
	{
		return ImageResource::load(m_ctx);
	}
};

ImageResource::~ImageResource()
{
}

Error ImageResource::load(const ResourceFilename& filename, Bool async)
{
	TexUploadTask* task;
	LoadingContext* ctx;
	LoadingContext localCtx;

	if(async)
	{
		task = AsyncLoader::getSingleton().newTask<TexUploadTask>();
		ctx = &task->m_ctx;
	}
	else
	{
		task = nullptr;
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

	ANKI_CHECK(loader.load(file, filename, g_maxImageSizeCVar));

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

	// Transition it. TODO remove this
	{
		const TextureView view(m_tex.get(), TextureSubresourceDesc::all());

		CommandBufferInitInfo cmdbinit;
		cmdbinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbinit);

		const TextureBarrierInfo barrier = {view, TextureUsageBit::kNone, TextureUsageBit::kAllSrv};
		cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

		FencePtr outFence;
		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get(), {}, &outFence);
		outFence->clientWait(60.0_sec);
	}

	// Set the context
	ctx->m_faces = faces;
	ctx->m_layerCount = init.m_layerCount;
	ctx->m_texType = init.m_type;
	ctx->m_tex = m_tex;

	// Upload the data
	if(async)
	{
		AsyncLoader::getSingleton().submitTask(task);
	}
	else
	{
		ANKI_CHECK(load(*ctx));
	}

	m_size = UVec3(init.m_width, init.m_height, init.m_depth);
	m_layerCount = init.m_layerCount;

	return Error::kNone;
}

Error ImageResource::load(LoadingContext& ctx)
{
	const U32 copyCount = ctx.m_layerCount * ctx.m_faces * ctx.m_loader.getMipmapCount();

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
			unflatten3dArrayIndex(ctx.m_layerCount, ctx.m_faces, ctx.m_loader.getMipmapCount(), i, layer, face, mip);

			barriers[barrierCount++] = {TextureView(ctx.m_tex.get(), TextureSubresourceDesc::surface(mip, face, layer)), TextureUsageBit::kAllSrv,
										TextureUsageBit::kCopyDestination};
		}
		cmdb->setPipelineBarrier({&barriers[0], barrierCount}, {}, {});

		// Do the copies
		Array<TransferGpuAllocatorHandle, kMaxCopiesBeforeFlush> handles;
		U32 handleCount = 0;
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(ctx.m_layerCount, ctx.m_faces, ctx.m_loader.getMipmapCount(), i, layer, face, mip);

			PtrSize surfOrVolSize;
			const void* surfOrVolData;
			PtrSize allocationSize;

			if(ctx.m_texType == TextureType::k3D)
			{
				const auto& vol = ctx.m_loader.getVolume(mip);
				surfOrVolSize = vol.m_data.getSize();
				surfOrVolData = &vol.m_data[0];

				allocationSize = computeVolumeSize(ctx.m_tex->getWidth() >> mip, ctx.m_tex->getHeight() >> mip, ctx.m_tex->getDepth() >> mip,
												   ctx.m_tex->getFormat());
			}
			else
			{
				const auto& surf = ctx.m_loader.getSurface(mip, face, layer);
				surfOrVolSize = surf.m_data.getSize();
				surfOrVolData = &surf.m_data[0];

				allocationSize = computeSurfaceSize(ctx.m_tex->getWidth() >> mip, ctx.m_tex->getHeight() >> mip, ctx.m_tex->getFormat());
			}

			ANKI_ASSERT(allocationSize >= surfOrVolSize);
			TransferGpuAllocatorHandle& handle = handles[handleCount++];
			ANKI_CHECK(TransferGpuAllocator::getSingleton().allocate(allocationSize, handle));
			void* data = handle.getMappedMemory();
			ANKI_ASSERT(data);

			memcpy(data, surfOrVolData, surfOrVolSize);

			// Create temp tex view
			const TextureSubresourceDesc subresource = TextureSubresourceDesc::surface(mip, face, layer);
			cmdb->copyBufferToTexture(handle, TextureView(ctx.m_tex.get(), subresource));
		}

		// Set the barriers of the batch
		barrierCount = 0;
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(ctx.m_layerCount, ctx.m_faces, ctx.m_loader.getMipmapCount(), i, layer, face, mip);

			barriers[barrierCount++] = {TextureView(ctx.m_tex.get(), TextureSubresourceDesc::surface(mip, face, layer)),
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

	return Error::kNone;
}

} // end namespace anki
