// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/StreamingImageResource.h>
#include <AnKi/Resource/ImageLoader.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/GpuMemory/CopyEngine.h>

namespace anki {

static Format computeFormat(const ImageLoader& loader)
{
	Format fmt = Format::kNone;

	if(loader.getColorFormat() == ImageBinaryColorFormat::kRgb8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR8G8B8_Unorm;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC1_Rgba_Unorm_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Unorm_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Unorm_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kSrgb8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR8G8B8_Srgb;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC1_Rgba_Srgb_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Srgb_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Srgb_Block;
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
			fmt = Format::kR8G8B8A8_Unorm;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC3_Unorm_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Unorm_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Unorm_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kSrgba8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR8G8B8A8_Srgb;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC3_Srgb_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Srgb_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Srgb_Block;
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
			fmt = Format::kBC6H_Ufloat_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
			fmt = Format::kASTC_8x8_Sfloat_Block;
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
			fmt = Format::kR32G32B32A32_Sfloat;
			break;
		case ImageBinaryDataCompression::kAstc:
			ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
			fmt = Format::kASTC_8x8_Sfloat_Block;
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return fmt;
}

static void fillTextureInitInfo(const ImageLoader& loader, TextureInitInfo& init)
{
	init.m_width = loader.getWidth();
	init.m_height = loader.getHeight();
	init.m_format = computeFormat(loader);
	init.m_mipmapCount = U8(loader.getMipmapCount());
	init.m_usage = TextureUsageBit::kAllSrv | TextureUsageBit::kCopyDestination;
	init.m_samples = 1;

	switch(loader.getImageType())
	{
	case ImageBinaryType::k2D:
		init.m_type = TextureType::k2D;
		init.m_depth = 1;
		init.m_layerCount = 1;
		break;
	case ImageBinaryType::kCube:
		init.m_type = TextureType::kCube;
		init.m_depth = 1;
		init.m_layerCount = 1;
		break;
	case ImageBinaryType::k2DArray:
		init.m_type = TextureType::k2DArray;
		init.m_layerCount = loader.getLayerCount();
		init.m_depth = 1;
		break;
	case ImageBinaryType::k3D:
		init.m_type = TextureType::k3D;
		init.m_depth = loader.getDepth();
		init.m_layerCount = 1;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

Error StreamingImageResourceManager::init()
{
	m_imageDescriptorsBuff = TextureMemoryPool::getSingleton().allocateStructuredBuffer<ImageDescriptor>(g_cvarRsrcMaxImageDescriptors);

	for(U32 i = 0; i < g_cvarRsrcMaxImageDescriptors; ++i)
	{
		m_freeDescriptorMask.setBit(i);
	}

	return Error::kNone;
}

ImageDescriptorHandle StreamingImageResourceManager::newImageDescriptor()
{
	LockGuard lock(m_mtx);

	const U32 index = m_freeDescriptorMask.getLeastSignificantBit();
	if(index == kMaxU32) [[unlikely]]
	{
		ANKI_RESOURCE_LOGF("Out of free descriptors. Increase %s", g_cvarRsrcMaxImageDescriptors.getName().cstr());
	}

	m_freeDescriptorMask.unsetBit(index);

	return index;
}

void StreamingImageResourceManager::freeImageDescriptor(ImageDescriptorHandle index)
{
	ANKI_ASSERT(index < g_cvarRsrcMaxImageDescriptors);

	LockGuard lock(m_mtx);

	ANKI_ASSERT(!m_freeDescriptorMask.getBit(index));

	if(m_garbage.getSize() == 0 || m_garbage.getBack().m_fence)
	{
		m_garbage.emplaceBack();
	}

	Garbage& garbage = m_garbage.getBack();

	ANKI_ASSERT(!garbage.m_freedDecriptorMask.getBit(index));
	garbage.m_freedDecriptorMask.setBit(index);
}

void StreamingImageResourceManager::uploadImageDescriptor(ImageDescriptorHandle index, const ImageDescriptor& desc) const
{
	ANKI_ASSERT(index < g_cvarRsrcMaxImageDescriptors);

#if ANKI_ASSERTIONS_ENABLED
	{
		LockGuard lock(m_mtx);
		ANKI_ASSERT(!m_freeDescriptorMask.getBit(index));
	}
#endif

	BufferView buffView = m_imageDescriptorsBuff;
	buffView.incrementOffset(index * sizeof(ImageDescriptor)).setRange(sizeof(ImageDescriptor));

	WeakArray<U8> mappedMem;
	auto lock = CopyEngine::getSingleton().copyBufferToBuffer(sizeof(ImageDescriptor), mappedMem, buffView);
	memcpy(mappedMem.getBegin(), &desc, sizeof(desc));
}

void StreamingImageResourceManager::endFrame(Fence* fence)
{
	ANKI_ASSERT(fence);

	LockGuard lock(m_mtx);

	// Collect garbage
	while(m_garbage.getSize() && m_garbage.getFront().m_fence && m_garbage.getFront().m_fence->signaled())
	{
		// Fence is signaled, collect the garbage

		m_garbage.getFront().m_freedDecriptorMask.iterateSetBitsFromLeastSignificant([this](U32 bit) {
			ANKI_ASSERT(!m_freeDescriptorMask.getBit(bit));
			m_freeDescriptorMask.setBit(bit);
			return FunctorContinue::kContinue;
		});

		m_garbage.erase(m_garbage.getBegin());
	}

	// Set the new fence
	if(m_garbage.getSize() && !m_garbage.getBack().m_fence)
	{
		m_garbage.getBack().m_fence.reset(fence);
	}
}

class StreamingImageResource::LoadingContext
{
public:
	ImageLoader m_loader{&ResourceMemoryPool::getSingleton()};
	StreamingImageResourcePtr m_image;
	U32 m_mipCount = 0;
	U32 m_texCount = 0;
};

// Image upload async task.
class StreamingImageResource::TexUploadTask : public AsyncLoaderTask
{
public:
	StreamingImageResource::LoadingContext m_ctx;

	Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
	{
		return m_ctx.m_image->loadAsync(m_ctx);
	}
};

StreamingImageResource::~StreamingImageResource()
{
	if(m_imageDescHandle != kMaxU32)
	{
		StreamingImageResourceManager::getSingleton().freeImageDescriptor(m_imageDescHandle);
	}
}

Error StreamingImageResource::load(const ResourceFilename& filename, Bool async)
{
	TexUploadTask* task = nullptr;
	ANKI_DEFER(deleteInstance(ResourceMemoryPool::getSingleton(), task));

	LoadingContext* ctx;
	LoadingContext localCtx;

	if(async)
	{
		task = AsyncLoader::getSingleton().newTask<TexUploadTask>();
		ctx = &task->m_ctx;
		ctx->m_image.reset(this);
	}
	else
	{
		ctx = &localCtx;
	}

	ImageLoader& loader = ctx->m_loader;

	ANKI_CHECK(loader.loadHeaderFromResourceFile(filename, g_cvarRsrcMaxImageSize2));

	U32 minDimension = min(loader.getWidth(), loader.getHeight());
	if(loader.getImageType() == ImageBinaryType::k3D)
	{
		minDimension = min(minDimension, loader.getDepth());
	}
	if(minDimension < kImageDescriptorSmallestMipmapSize)
	{
		ANKI_RESOURCE_LOGE("Can't have images with a dimension less than %u", kImageDescriptorSmallestMipmapSize);
		return Error::kUserData;
	}

	m_avgColor = loader.getAverageColor();

	// Create the textures
	//
	U32 mipCount = (loader.getImageType() != ImageBinaryType::k3D)
					   ? computeMaxMipmapCount2d(loader.getWidth(), loader.getHeight(), kImageDescriptorSmallestMipmapSize)
					   : computeMaxMipmapCount3d(loader.getWidth(), loader.getHeight(), loader.getDepth(), kImageDescriptorSmallestMipmapSize);
	mipCount = min(mipCount, loader.getMipmapCount());
	ctx->m_mipCount = mipCount;

	TextureInitInfo texInit;
	fillTextureInitInfo(loader, texInit);
	texInit.m_mipmapCount = 1;

	const String filenameExt = anki::getFilename(filename);

	const U32 textureCount = U32(max(I32(mipCount) - I32(kImageDescriptorTailChainMipmapCount) + 1, 1));
	ctx->m_texCount = textureCount;
	for(U32 i = 0; i < textureCount; ++i)
	{
		texInit.setName(ResourceString().sprintf("%s #%u", filenameExt.cstr(), i));
		if(i == textureCount - 1)
		{
			texInit.m_mipmapCount = U8(min(kImageDescriptorTailChainMipmapCount, mipCount));
		}

		const PtrSize memReq = GrManager::getSingleton().getTextureMemoryRequirement(texInit);
		m_texAllocations[i] = TextureMemoryPool::getSingleton().allocate(memReq);

		texInit.m_memoryBuffer = m_texAllocations[i];
		m_textures[i] = GrManager::getSingleton().newTexture(texInit);

		texInit.m_width /= 2;
		texInit.m_height /= 2;
		if(texInit.m_type == TextureType::k3D)
		{
			texInit.m_depth /= 2;
		}
	}

	// Create the image descriptor
	{
		ImageDescriptor desc = {};
		desc.m_width = loader.getWidth();
		desc.m_height = loader.getHeight();
		if(loader.getImageType() == ImageBinaryType::k3D)
		{
			desc.m_depthOrLayerCount = loader.getDepth();
		}
		else if(loader.getImageType() == ImageBinaryType::k2DArray)
		{
			desc.m_depthOrLayerCount = loader.getLayerCount();
		}
		else
		{
			desc.m_depthOrLayerCount = 1;
		}

		desc.m_firstMipmap = 0;
		desc.m_lastMipmap = mipCount - 1;

		for(U32 i = 0; i < mipCount; ++i)
		{
			U32 packedBindlessIndexAndLod = 0;

			const U32 firstMipmapOfTailChain = textureCount - 1;
			if(i < firstMipmapOfTailChain)
			{
				// Not tail chain
				const U32 bindlessIndex = m_textures[i]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
				packedBindlessIndexAndLod = bindlessIndex << 8u;
				ANKI_ASSERT(packedBindlessIndexAndLod >> 8u == bindlessIndex);
				packedBindlessIndexAndLod |= 0;
			}
			else
			{
				// Tail chain
				const U32 bindlessIndex = m_textures[firstMipmapOfTailChain]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
				packedBindlessIndexAndLod = bindlessIndex << 8u;
				ANKI_ASSERT(packedBindlessIndexAndLod >> 8u == bindlessIndex);
				packedBindlessIndexAndLod |= U8(i - firstMipmapOfTailChain);
			}

			desc.m_bindlessTextureIndexAndLod[i] = packedBindlessIndexAndLod;
		}

		m_imageDescHandle = StreamingImageResourceManager::getSingleton().newImageDescriptor();
		StreamingImageResourceManager::getSingleton().uploadImageDescriptor(m_imageDescHandle, desc);
	}

	// Upload the data
	if(async)
	{
		AsyncLoader::getSingleton().submitTask(task, AsyncLoaderPriority::kMedium);
		task = nullptr;
	}
	else
	{
		ANKI_CHECK(loadAsync(*ctx));
	}

	return Error::kNone;
}

Error StreamingImageResource::loadAsync(LoadingContext& ctx) const
{
	const U32 faceCount = textureTypeIsCube(m_textures[0]->getTextureType()) ? 6 : 1;
	const U32 layerCount = m_textures[0]->getLayerCount();
	const U32 mipCount = ctx.m_mipCount;
	const U32 copyCount = layerCount * faceCount * mipCount;
	const U32 texCount = ctx.m_texCount;
	const U32 firstMipmapOfTailChain = texCount - 1;

	// With GFXR enabled we can't do fwrite directly to mapped VkBuffer. So we need to first fwrite to a CPU buffer and copy that to the mapped
	// VkBuffer
	const Bool bGfxreconstruct = GrManager::getSingleton().getDeviceCapabilities().m_gfxReconstruct;

	static constexpr U32 kMaxCopiesBeforeFlush = 4;

	for(U32 b = 0; b < copyCount; b += kMaxCopiesBeforeFlush)
	{
		const U32 begin = b;
		const U32 end = min(copyCount, b + kMaxCopiesBeforeFlush);

		// Set the barriers of the batch
		Array<TextureBarrierInfo, kMaxCopiesBeforeFlush> barriers;
		U32 barrierCount = 0;
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(layerCount, faceCount, mipCount, i, layer, face, mip);

			const U32 texIdx = min(mip, texCount - 1);
			const U32 actualMip = (mip < firstMipmapOfTailChain) ? 0 : mip - firstMipmapOfTailChain;

			barriers[barrierCount++] = {TextureView(m_textures[texIdx].get(), TextureSubresourceDesc::surface(actualMip, face, layer)),
										TextureUsageBit::kNone, TextureUsageBit::kCopyDestination};
		}
		CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});

		// Do the copies
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(layerCount, faceCount, mipCount, i, layer, face, mip);

			const U32 texIdx = min(mip, texCount - 1);
			const U32 actualMip = (mip < firstMipmapOfTailChain) ? 0 : mip - firstMipmapOfTailChain;

			const Texture& firstTex = *m_textures[0];
			PtrSize allocationSize;
			if(m_textures[0]->getTextureType() == TextureType::k3D)
			{
				allocationSize =
					computeVolumeSize(firstTex.getWidth() >> mip, firstTex.getHeight() >> mip, firstTex.getDepth() >> mip, firstTex.getFormat());
			}
			else
			{
				allocationSize = computeSurfaceSize(firstTex.getWidth() >> mip, firstTex.getHeight() >> mip, firstTex.getFormat());
			}

			WeakArray<U8> mappedMem;
			const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToTexture(
				U32(allocationSize), mappedMem, TextureView(m_textures[texIdx].get(), TextureSubresourceDesc::surface(actualMip, face, layer)));

			ResourceDynamicArray<U8> tmpData;
			WeakArray<U8> copyDest;
			if(bGfxreconstruct)
			{
				tmpData.resize(mappedMem.getSize());
				copyDest = tmpData;
			}
			else
			{
				copyDest = mappedMem;
			}

			ANKI_CHECK(ctx.m_loader.loadSurfaceOrVolume(mip, face, layer, copyDest));

			if(bGfxreconstruct)
			{
				memcpy(mappedMem.getBegin(), copyDest.getBegin(), copyDest.getSizeInBytes());
			}
		}

		// Set the barriers of the batch
		barrierCount = 0;
		for(U32 i = begin; i < end; ++i)
		{
			U32 mip, layer, face;
			unflatten3dArrayIndex(layerCount, faceCount, mipCount, i, layer, face, mip);

			const U32 texIdx = min(mip, texCount - 1);
			const U32 actualMip = (mip < firstMipmapOfTailChain) ? 0 : mip - firstMipmapOfTailChain;

			barriers[barrierCount++] = {TextureView(m_textures[texIdx].get(), TextureSubresourceDesc::surface(actualMip, face, layer)),
										TextureUsageBit::kCopyDestination, TextureUsageBit::kAllSrv};
		}
		CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});
	}

	m_isLoaded.fetchAdd(1);
	return Error::kNone;
}

} // end namespace anki
