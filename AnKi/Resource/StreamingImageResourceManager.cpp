// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/StreamingImageResourceManager.h>
#include <AnKi/Resource/ImageLoader.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Gr/Texture.h>
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

static TextureType computeType(const ImageLoader& loader)
{
	TextureType type = TextureType::kCount;
	switch(loader.getImageType())
	{
	case ImageBinaryType::k2D:
		type = TextureType::k2D;
		break;
	case ImageBinaryType::kCube:
		type = TextureType::kCube;
		break;
	case ImageBinaryType::k2DArray:
		type = TextureType::k2DArray;
		break;
	case ImageBinaryType::k3D:
		type = TextureType::k3D;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return type;
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

// These data is what the main thead hammers. So it's seperate
class StreamingImageResourceManager::StreamingInfo
{
public:
	U64 m_lastSeenFrame = 0;
	U32 m_resourceUuid = kMaxU32;
	U8 m_lastSeenDetailedMip = kMaxU8;
	U8 m_detailedLoadedMip = kMaxU8;
	U8 m_tailChainMip = kMaxU8;
};

// The rest of the image data
class StreamingImage
{
public:
	Vec4 m_avgColor = Vec4(0.0f);

	Array<TextureMemoryPoolAllocation, kImageDescriptorMaxBindlessTextures> m_texAllocations;
	Array<TexturePtr, kImageDescriptorMaxBindlessTextures> m_textures;

	Format m_format = Format::kNone; // Cache it

	ImageDescriptor m_imageDesc = {};

	ResourceString m_filename;

	mutable Atomic<I32> m_refcount = {0};

	U32 m_arrayIndex = kMaxU32;

	mutable Atomic<Bool> m_isLoaded = {false};

	U8 m_textureCount = 0;
	U8 m_mipCount = 0;

	TextureType m_texType = TextureType::kCount;

	U8 getFirstMipOfTailChain() const
	{
		return m_textureCount - 1;
	}

	U32 getFaceCount() const
	{
		return textureTypeIsCube(m_texType) ? 6 : 1;
	}

	U32 getLayerCount() const
	{
		return (m_texType == TextureType::k3D) ? 1 : m_imageDesc.m_depthOrLayerCount;
	}

	I32 retain() const
	{
		return m_refcount.fetchAdd(1) + 1;
	}

	void release() const
	{
		if(m_refcount.fetchSub(1, AtomicMemoryOrder::kAcqRel) == 1)
		{
			StreamingImageResourceManager::getSingleton().release(m_arrayIndex);
		}
	}

	PtrSize estimateSurfaceMemoryConsumption(U32 firstMip, U32 mipCount) const
	{
		ANKI_ASSERT(firstMip + mipCount <= m_textureCount - 1);

		PtrSize size = 0;
		for(U32 mip = firstMip; mip < firstMip + mipCount; ++mip)
		{
			const U32 width = m_imageDesc.m_width >> mip;
			const U32 height = m_imageDesc.m_height >> mip;

			if(m_texType == TextureType::k3D)
			{
				size += computeVolumeSize(width, height, m_imageDesc.m_depthOrLayerCount >> mip, m_format);
			}
			else
			{
				const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;

				size += computeSurfaceSize(width, height, m_format) * faceCount * m_imageDesc.m_depthOrLayerCount;
			}
		}

		return size;
	}
};

StreamingImageResourceManager::StreamingImageResourceManager()
{
	m_imageDescriptorsBuff = TextureMemoryPool::getSingleton().allocateStructuredBuffer<ImageDescriptor>(g_cvarRsrcMaxImageDescriptors);

	// Zero the descriptor buffer
	CopyEngine::getSingleton().zeroBuffer(m_imageDescriptorsBuff);
	const BufferBarrierInfo imgDescBarrier = {m_imageDescriptorsBuff, BufferUsageBit::kCopyDestination,
											  BufferView(m_imageDescriptorsBuff).getBuffer().getBufferUsage()};
	CopyEngine::getSingleton().setPipelineBarrier({}, {&imgDescBarrier, 1}, {});

	// The 0 descriptor is reserved so that materials can use that to indicate that there is no texture bound
	[[maybe_unused]] auto it1 = m_imageData.emplace();
	[[maybe_unused]] auto it2 = m_streamingInfos.emplace();
	ANKI_ASSERT(it1.getArrayIndex() == 0 && it2.getArrayIndex() == 0);
}

StreamingImageResourceManager::~StreamingImageResourceManager()
{
	for(Garbage& garbage : m_garbage)
	{
		if(garbage.m_fence)
		{
			garbage.m_fence->clientWaitForever();
		}
	}

	collectGarbage(true);

	ANKI_ASSERT(m_imageData.getSize() == 1 && m_streamingInfos.getSize() == 1);

	// Wait for any copy engine to also complete
	CopyEngine::getSingleton().flushAndWaitForAllWork();

	// Now everything is ready to be automatically be deleted
}

Error StreamingImageResourceManager::loadNewImage(ResourceFilename filename, U32 resourceUuid, Bool async, U32& arrayIdx, StreamingImage*& img)
{
	ANKI_ASSERT(!filename.isEmpty());
	ANKI_ASSERT(resourceUuid > 0);
	img = nullptr;
	arrayIdx = 0; // 0 is reserved so 0 means invalid

	class TexUploadTask : public AsyncLoaderTask
	{
	public:
		ImageLoader m_loader{&ResourceMemoryPool::getSingleton()};
		IntrusiveNoDelPtr<StreamingImage> m_img;

		Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
		{
			return StreamingImageResourceManager::getSingleton().loadTailMipChainAsync(*m_img, m_loader);
		}
	};

	TexUploadTask* task = nullptr;
	ImageLoader localLoader(&ResourceMemoryPool::getSingleton());
	ImageLoader* loader = nullptr;
	if(async)
	{
		task = AsyncLoader::getSingleton().newTask<TexUploadTask>();
		loader = &task->m_loader;
	}
	else
	{
		loader = &localLoader;
	}

	Error err = loader->loadHeaderFromResourceFile(filename, g_cvarRsrcMaxImageSize);

	// Some checks
	if(!err)
	{
		U32 minDimension = min(loader->getWidth(), loader->getHeight());
		minDimension = (loader->getImageType() == ImageBinaryType::k3D) ? min(minDimension, loader->getDepth()) : minDimension;

		if(minDimension < kImageDescriptorSmallestMipmapSize)
		{
			ANKI_RESOURCE_LOGE("Can't have images with a dimension less than %u", kImageDescriptorSmallestMipmapSize);
			err = Error::kUserData;
		}
	}

	// Init StreamingImage and StreamingInfo
	if(!err)
	{
		LockGuard lock(m_mtx);

		auto it1 = m_imageData.emplace();
		auto it2 = m_streamingInfos.emplace();

		ANKI_ASSERT(it1.getArrayIndex() == it2.getArrayIndex());

		arrayIdx = it1.getArrayIndex();

		if(it1.getArrayIndex() >= g_cvarRsrcMaxImageDescriptors)
		{
			ANKI_RESOURCE_LOGE("We are out of image descriptors. Increase: %s", g_cvarRsrcMaxImageDescriptors.getName().cstr());
			err = Error::kOutOfMemory;
		}

		if(!err)
		{
			m_maxDescriptorIndex = max(m_maxDescriptorIndex, arrayIdx);

			img = &(*it1);
			img->retain(); // For the caller

			StreamingInfo& streaming = *it2;

			// Set all we can now and under the lock. The StreamingInfo needs to be initialized under the lock because performStreaming() iterates all
			// StreamingInfo
			img->m_arrayIndex = arrayIdx;
			img->m_filename = filename;
			img->m_avgColor = loader->getAverageColor();

			img->m_mipCount =
				(loader->getImageType() != ImageBinaryType::k3D)
					? computeMaxMipmapCount2d(loader->getWidth(), loader->getHeight(), kImageDescriptorSmallestMipmapSize)
					: computeMaxMipmapCount3d(loader->getWidth(), loader->getHeight(), loader->getDepth(), kImageDescriptorSmallestMipmapSize);
			img->m_mipCount = min(img->m_mipCount, U8(loader->getMipmapCount()));

			img->m_textureCount = U8(max(I32(img->m_mipCount) - I32(kImageDescriptorTailChainMipmapCount) + 1, 1));
			img->m_imageDesc.m_resourceUuid = resourceUuid;

			img->m_imageDesc.m_width = loader->getWidth();
			img->m_imageDesc.m_height = loader->getHeight();
			if(loader->getImageType() == ImageBinaryType::k3D)
			{
				img->m_imageDesc.m_depthOrLayerCount = loader->getDepth();
			}
			else if(loader->getImageType() == ImageBinaryType::k2DArray)
			{
				img->m_imageDesc.m_depthOrLayerCount = loader->getLayerCount();
			}
			else
			{
				img->m_imageDesc.m_depthOrLayerCount = 1;
			}

			img->m_texType = computeType(*loader);
			img->m_format = computeFormat(*loader);

			const U8 tailChainTexIdx = img->m_textureCount - 1;
			const U8 firstMipOfTailChain = tailChainTexIdx;

			img->m_imageDesc.m_firstMipmap = firstMipOfTailChain;
			img->m_imageDesc.m_lastMipmap = img->m_mipCount - 1;

			streaming.m_resourceUuid = resourceUuid;
			streaming.m_lastSeenDetailedMip = firstMipOfTailChain;
			streaming.m_detailedLoadedMip = firstMipOfTailChain;
			streaming.m_tailChainMip = firstMipOfTailChain;
		}
	}

	// Do the work or defer it
	if(!err)
	{
		if(async)
		{
			task->m_img.reset(img);
			AsyncLoader::getSingleton().submitTask(task, AsyncLoaderPriority::kHigh);
		}
		else
		{
			err = loadTailMipChainAsync(*img, *loader);
		}
	}

	// Cleanup
	if(err)
	{
		if(task)
		{
			StreamingImage* data;
			task->m_img.moveAndReset(data); // It will be erased bellow
			deleteInstance(ResourceMemoryPool::getSingleton(), task);
		}

		if(arrayIdx != 0)
		{
			LockGuard lock(m_mtx);
			m_imageData.erase(arrayIdx);
			m_streamingInfos.erase(arrayIdx);
		}

		arrayIdx = kMaxU32;
		img = nullptr;
	}

	return err;
}

Error StreamingImageResourceManager::loadTailMipChainAsync(StreamingImage& img, ImageLoader& loader)
{
	ANKI_ASSERT(img.m_isLoaded.load() == false);

	const U8 tailChainTexIdx = img.m_textureCount - 1;
	const U8 firstMipOfTailChain = tailChainTexIdx;

	// Create the tail chain texture
	{
		TextureInitInfo texInit;
		fillTextureInitInfo(loader, texInit);
		texInit.m_mipmapCount = min(U8(kImageDescriptorTailChainMipmapCount), img.m_mipCount);
		texInit.m_width = loader.getWidth() >> firstMipOfTailChain;
		texInit.m_height = loader.getHeight() >> firstMipOfTailChain;
		if(texInit.m_type == TextureType::k3D)
		{
			texInit.m_depth = loader.getDepth() >> firstMipOfTailChain;
		}

		const String filenameExt = anki::getFilename(img.m_filename);
		texInit.setName(ResourceString().sprintf("%s tail", filenameExt.cstr()));

		const PtrSize memReq = GrManager::getSingleton().getTextureMemoryRequirement(texInit);
		img.m_texAllocations[tailChainTexIdx] = TextureMemoryPool::getSingleton().allocate(memReq);

		texInit.m_memoryBuffer = img.m_texAllocations[tailChainTexIdx];
		img.m_textures[tailChainTexIdx] = GrManager::getSingleton().newTexture(texInit);
	}

	// Do the copies
	{
		Texture& chainTex = *img.m_textures[tailChainTexIdx];
		const U32 faceCount = textureTypeIsCube(chainTex.getTextureType()) ? 6 : 1;
		const U32 layerCount = chainTex.getLayerCount();

		// With GFXR enabled we can't do fwrite directly to mapped VkBuffer. So we need to first fwrite to a CPU buffer and copy that to the mapped
		// VkBuffer
		const Bool bGfxreconstruct = GrManager::getSingleton().getDeviceCapabilities().m_gfxReconstruct;

		// Set barriers
		Array<TextureBarrierInfo, kImageDescriptorTailChainMipmapCount> barriers;
		U32 barrierCount = 0;
		for(U32 l = 0; l < layerCount; ++l)
		{
			for(U32 f = 0; f < faceCount; ++f)
			{
				barrierCount = 0;

				for(U32 mip = firstMipOfTailChain; mip < img.m_mipCount; ++mip)
				{
					const U32 tailChainMip = mip - firstMipOfTailChain;

					barriers[barrierCount++] = {TextureView(&chainTex, TextureSubresourceDesc::surface(tailChainMip, f, l)), TextureUsageBit::kNone,
												TextureUsageBit::kCopyDestination};
				}

				CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});
			}
		}

		// Do the copies
		for(U32 l = 0; l < layerCount; ++l)
		{
			for(U32 f = 0; f < faceCount; ++f)
			{
				for(U32 mip = firstMipOfTailChain; mip < img.m_mipCount; ++mip)
				{
					const U32 tailChainMip = mip - firstMipOfTailChain;

					PtrSize allocationSize;
					if(chainTex.getTextureType() == TextureType::k3D)
					{
						allocationSize = computeVolumeSize(chainTex.getWidth() >> tailChainMip, chainTex.getHeight() >> tailChainMip,
														   chainTex.getDepth() >> tailChainMip, chainTex.getFormat());
					}
					else
					{
						allocationSize =
							computeSurfaceSize(chainTex.getWidth() >> tailChainMip, chainTex.getHeight() >> tailChainMip, chainTex.getFormat());
					}

					WeakArray<U8> mappedMem;
					const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToTexture(
						U32(allocationSize), mappedMem, TextureView(&chainTex, TextureSubresourceDesc::surface(tailChainMip, f, l)));

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

					ANKI_CHECK(loader.loadSurfaceOrVolume(mip, f, l, copyDest));

					if(bGfxreconstruct)
					{
						memcpy(mappedMem.getBegin(), copyDest.getBegin(), copyDest.getSizeInBytes());
					}
				}
			}
		}

		// Final barriers
		for(U32 l = 0; l < layerCount; ++l)
		{
			for(U32 f = 0; f < faceCount; ++f)
			{
				barrierCount = 0;

				for(U32 mip = firstMipOfTailChain; mip < img.m_mipCount; ++mip)
				{
					const U32 tailChainMip = mip - firstMipOfTailChain;

					barriers[barrierCount++] = {TextureView(&chainTex, TextureSubresourceDesc::surface(tailChainMip, f, l)),
												TextureUsageBit::kCopyDestination, TextureUsageBit::kAllSrv};
				}

				CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});
			}
		}
	}

	// Create and upload the image descriptor after the copies
	{
		const U32 tailChainTexBindlessIdx = img.m_textures[tailChainTexIdx]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());

		for(U32 i = firstMipOfTailChain; i < img.m_mipCount; ++i)
		{
			U32 packedBindlessIndexAndLod = tailChainTexBindlessIdx << 8u;
			packedBindlessIndexAndLod |= U8(i - firstMipOfTailChain);
			ANKI_ASSERT(packedBindlessIndexAndLod >> 8u == tailChainTexBindlessIdx);

			img.m_imageDesc.m_bindlessTextureIndexAndLod[i] = packedBindlessIndexAndLod;
		}

		// Upload
		BufferView buffView = m_imageDescriptorsBuff;
		buffView.incrementOffset(img.m_arrayIndex * sizeof(ImageDescriptor)).setRange(sizeof(ImageDescriptor));

		WeakArray<U8> mappedMem;
		auto lock = CopyEngine::getSingleton().copyBufferToBuffer(sizeof(ImageDescriptor), mappedMem, buffView);
		memcpy(mappedMem.getBegin(), &img.m_imageDesc, sizeof(img.m_imageDesc));
		lock.unlock();

		// Barrier
		const BufferUsageBit allUsage = BufferView(m_imageDescriptorsBuff).getBuffer().getBufferUsage();
		const BufferBarrierInfo imgDescBarrier = {m_imageDescriptorsBuff, BufferUsageBit::kCopyDestination, allUsage};
		CopyEngine::getSingleton().setPipelineBarrier({}, {&imgDescBarrier, 1}, {});
	}

	// Done. Use kAcqRel to be sure that the atomic operation will not be re-ordered by the compiler
	img.m_isLoaded.exchange(true, AtomicMemoryOrder::kRelease);

	return Error::kNone;
}

void StreamingImageResourceManager::loadOtherMips(ConstWeakArray<LoadMipsRequest> requests)
{
	class MyTask : public AsyncLoaderTask
	{
	public:
		ResourceDynamicArray<LoadMipsRequest> m_requests;

		Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
		{
			return StreamingImageResourceManager::getSingleton().loadOtherMipsAsync(m_requests);
		}
	};

	MyTask* task = AsyncLoader::getSingleton().newTask<MyTask>();
	task->m_requests = requests;

	AsyncLoader::getSingleton().submitTask(task, AsyncLoaderPriority::kMedium);
}

void StreamingImageResourceManager::unloadOtherMips(ConstWeakArray<LoadMipsRequest> requests)
{
	class MyTask : public AsyncLoaderTask
	{
	public:
		ResourceDynamicArray<LoadMipsRequest> m_requests;

		Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
		{
			return StreamingImageResourceManager::getSingleton().unloadOtherMipsAsync(m_requests);
		}
	};

	MyTask* task = AsyncLoader::getSingleton().newTask<MyTask>();
	task->m_requests = requests;

	AsyncLoader::getSingleton().submitTask(task, AsyncLoaderPriority::kMedium);
}

Error StreamingImageResourceManager::loadOtherMipsAsync(ConstWeakArray<LoadMipsRequest> requests)
{
	ANKI_ASSERT(requests.getSize() > 0);

	// With GFXR enabled we can't do fwrite directly to mapped VkBuffer. So we need to first fwrite to a CPU buffer and copy that to the mapped
	// VkBuffer
	const Bool bGfxreconstruct = GrManager::getSingleton().getDeviceCapabilities().m_gfxReconstruct;

	// Init the loaders
	ResourceDynamicArray<ImageLoader> loaders;
	loaders.resize(requests.getSize(), &ResourceMemoryPool::getSingleton());
	U32 count = 0;
	for(const LoadMipsRequest& req : requests)
	{
		ANKI_CHECKF(loaders[count++].loadHeaderFromResourceFile(req.m_img->m_filename, g_cvarRsrcMaxImageSize));
	}

	// Create the textures
	count = 0;
	for(const LoadMipsRequest& req : requests)
	{
		StreamingImage& img = *req.m_img;
		ImageLoader& loader = loaders[count++];
		const String filenameExt = anki::getFilename(img.m_filename);

		for(U32 mip = req.m_firstMip; mip < req.m_firstMip + req.m_mipCount; ++mip)
		{
			ANKI_ASSERT(mip < img.getFirstMipOfTailChain());

			TextureInitInfo texInit;
			fillTextureInitInfo(loader, texInit);
			texInit.setName(ResourceString().sprintf("%s #%u", filenameExt.cstr(), mip));

			texInit.m_mipmapCount = 1;

			texInit.m_width = loader.getWidth() >> mip;
			texInit.m_height = loader.getHeight() >> mip;
			if(texInit.m_type == TextureType::k3D)
			{
				texInit.m_depth = loader.getDepth() >> mip;
			}

			const PtrSize memReq = GrManager::getSingleton().getTextureMemoryRequirement(texInit);
			ANKI_ASSERT(!img.m_texAllocations[mip]);
			img.m_texAllocations[mip] = TextureMemoryPool::getSingleton().allocate(memReq);

			texInit.m_memoryBuffer = img.m_texAllocations[mip];
			ANKI_ASSERT(!img.m_textures[mip]);
			img.m_textures[mip] = GrManager::getSingleton().newTexture(texInit);
		}

		g_svarRsrcStreamingMipsUploaded.increment(req.m_mipCount);
		g_svarRsrcTotalStreamingMipsUploaded.increment(req.m_mipCount);
	}

	// Set the barriers
	Array<TextureBarrierInfo, kImageDescriptorMaxMipmaps - kImageDescriptorTailChainMipmapCount> barriers;
	for(const LoadMipsRequest& req : requests)
	{
		StreamingImage& img = *req.m_img;
		U32 barrierCount = 0;

		for(U32 l = 0; l < img.getLayerCount(); ++l)
		{
			for(U32 f = 0; f < img.getFaceCount(); ++f)
			{
				barrierCount = 0;

				for(U32 mip = req.m_firstMip; mip < req.m_firstMip + req.m_mipCount; ++mip)
				{
					barriers[barrierCount++] = {TextureView(img.m_textures[mip].get(), TextureSubresourceDesc::surface(0, f, l)),
												TextureUsageBit::kNone, TextureUsageBit::kCopyDestination};
				}

				CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});
			}
		}
	}

	// Do the copies
	count = 0;
	for(const LoadMipsRequest& req : requests)
	{
		StreamingImage& img = *req.m_img;
		ImageLoader& loader = loaders[count++];

		for(U32 l = 0; l < img.getLayerCount(); ++l)
		{
			for(U32 f = 0; f < img.getFaceCount(); ++f)
			{
				for(U32 mip = req.m_firstMip; mip < req.m_firstMip + req.m_mipCount; ++mip)
				{
					Texture& tex = *img.m_textures[mip];

					PtrSize allocationSize;
					if(tex.getTextureType() == TextureType::k3D)
					{
						allocationSize = computeVolumeSize(tex.getWidth(), tex.getHeight(), tex.getDepth(), tex.getFormat());
					}
					else
					{
						allocationSize = computeSurfaceSize(tex.getWidth(), tex.getHeight(), tex.getFormat());
					}

					WeakArray<U8> mappedMem;
					const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToTexture(
						U32(allocationSize), mappedMem, TextureView(&tex, TextureSubresourceDesc::surface(0, f, l)));

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

					ANKI_CHECKF(loader.loadSurfaceOrVolume(mip, f, l, copyDest));

					if(bGfxreconstruct)
					{
						memcpy(mappedMem.getBegin(), copyDest.getBegin(), copyDest.getSizeInBytes());
					}
				}
			}
		}
	}

	// Set the post copy barriers
	for(const LoadMipsRequest& req : requests)
	{
		StreamingImage& img = *req.m_img;
		U32 barrierCount = 0;

		for(U32 l = 0; l < img.getLayerCount(); ++l)
		{
			for(U32 f = 0; f < img.getFaceCount(); ++f)
			{
				barrierCount = 0;

				for(U32 mip = req.m_firstMip; mip < req.m_firstMip + req.m_mipCount; ++mip)
				{
					barriers[barrierCount++] = {TextureView(img.m_textures[mip].get(), TextureSubresourceDesc::surface(0, f, l)),
												TextureUsageBit::kCopyDestination, TextureUsageBit::kAllSrv};
				}

				CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});
			}
		}
	}

	// Also barrier for the descriptor update that follow
	// WARNING: The barrier is not ideal but it's enough to block the desc update until the texture copies above have completed
	const BufferUsageBit allUsage = BufferView(m_imageDescriptorsBuff).getBuffer().getBufferUsage();
	const BufferBarrierInfo buffBarr = {m_imageDescriptorsBuff, allUsage, BufferUsageBit::kCopyDestination};
	CopyEngine::getSingleton().setPipelineBarrier({}, {&buffBarr, 1}, {});

	// Update the descriptors
	// WARNING: The update of the descriptor will happen in the async queue while the renderer (which runs in the generic queue) is accessing it
	for(const LoadMipsRequest& req : requests)
	{
		StreamingImage& img = *req.m_img;

		img.m_imageDesc.m_firstMipmap = req.m_firstMip;

		for(U32 mip = req.m_firstMip; mip < req.m_firstMip + req.m_mipCount; ++mip)
		{
			ANKI_ASSERT(img.m_imageDesc.m_bindlessTextureIndexAndLod[mip] == 0);
			img.m_imageDesc.m_bindlessTextureIndexAndLod[mip] = img.m_textures[mip]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all())
																<< 8u;
		}

		BufferView buffView = m_imageDescriptorsBuff;
		buffView.incrementOffset(req.m_arrayIndex * sizeof(ImageDescriptor)).setRange(sizeof(ImageDescriptor));

		WeakArray<U8> mappedMem;
		auto lock = CopyEngine::getSingleton().copyBufferToBuffer(sizeof(ImageDescriptor), mappedMem, buffView);
		memcpy(mappedMem.getBegin(), &img.m_imageDesc, sizeof(img.m_imageDesc));
	}

	return Error::kNone;
}

Error StreamingImageResourceManager::unloadOtherMipsAsync(ConstWeakArray<LoadMipsRequest> requests)
{
	ANKI_ASSERT(requests.getSize());

	// Block all prev operations before our new copies
	const BufferUsageBit allUsage = BufferView(m_imageDescriptorsBuff).getBuffer().getBufferUsage();
	const BufferBarrierInfo buffBarr = {m_imageDescriptorsBuff, allUsage, BufferUsageBit::kCopyDestination};
	CopyEngine::getSingleton().setPipelineBarrier({}, {&buffBarr, 1}, {});

	class CallbackData
	{
	public:
		ResourceDynamicArray<TexturePtr> m_texturesToDel;
		ResourceDynamicArray<TextureMemoryPoolAllocation> m_allocsToFree;
	};

	CallbackData* callbackData = newInstance<CallbackData>(ResourceMemoryPool::getSingleton());

	for(const LoadMipsRequest& req : requests)
	{
		StreamingImage& img = *req.m_img;

		// Update the image descriptor
		[[maybe_unused]] const U32 firstMipmapOfTailChain = img.m_textureCount - 1;
		ANKI_ASSERT(req.m_firstMip + req.m_mipCount <= firstMipmapOfTailChain);
		ANKI_ASSERT(req.m_firstMip == img.m_imageDesc.m_firstMipmap && "Can only evict the finest resident mips");
		const U8 minLoadedMip = U8(req.m_firstMip + req.m_mipCount);
		img.m_imageDesc.m_firstMipmap = minLoadedMip;
		for(U32 mip = req.m_firstMip; mip < req.m_firstMip + req.m_mipCount; ++mip)
		{
			ANKI_ASSERT(img.m_imageDesc.m_bindlessTextureIndexAndLod[mip] != 0);
			img.m_imageDesc.m_bindlessTextureIndexAndLod[mip] = 0;
		}

		// Copy the image descriptor
		BufferView buffView = m_imageDescriptorsBuff;
		buffView.incrementOffset(req.m_arrayIndex * sizeof(ImageDescriptor)).setRange(sizeof(ImageDescriptor));

		WeakArray<U8> mappedMem;
		auto lock = CopyEngine::getSingleton().copyBufferToBuffer(sizeof(ImageDescriptor), mappedMem, buffView);
		memcpy(mappedMem.getBegin(), &img.m_imageDesc, sizeof(img.m_imageDesc));
		lock.unlock();

		for(U32 mip = req.m_firstMip; mip < req.m_firstMip + req.m_mipCount; ++mip)
		{
			ANKI_ASSERT(!!img.m_textures[mip]);
			callbackData->m_texturesToDel.emplaceBack(std::move(img.m_textures[mip]));
			ANKI_ASSERT(!!img.m_texAllocations[mip]);
			callbackData->m_allocsToFree.emplaceBack(std::move(img.m_texAllocations[mip]));
		}

		g_svarRsrcStreamingMipsEvicted.increment(req.m_mipCount);
		g_svarRsrcTotalStreamingMipsEvicted.increment(req.m_mipCount);
	}

	// When the above changes are submitted the copy engine will trigger the Function bellow which will then send the textures to the manager
	// for deletion when its safe
	CopyEngine::getSingleton().addPostFlushCallback([this, callbackData](Fence*) {
		LockGuard lock(m_mtx);

		if(m_garbage.getSize() == 0 || m_garbage.getBack().m_fence)
		{
			m_garbage.emplaceBack();
		}

		Garbage& garbage = m_garbage.getBack();

		garbage.m_allocsToFree.emplaceDynamicArrayBack(std::move(callbackData->m_allocsToFree));
		garbage.m_texturesToDelete.emplaceDynamicArrayBack(std::move(callbackData->m_texturesToDel));

		deleteInstance(ResourceMemoryPool::getSingleton(), callbackData);
	});

	return Error::kNone;
}

void StreamingImageResourceManager::release(U32 arrayIndex)
{
	LockGuard lock(m_mtx);

	// Release resources but don't touch the block arrays

	for(TexturePtr& tex : m_imageData[arrayIndex].m_textures)
	{
		tex.reset(nullptr);
	}

	for(TextureMemoryPoolAllocation& alloc : m_imageData[arrayIndex].m_texAllocations)
	{
		alloc.deferredFree();
	}

	if(m_garbage.getSize() == 0 || m_garbage.getBack().m_fence)
	{
		m_garbage.emplaceBack();
	}

	Garbage& garbage = m_garbage.getBack();

	ANKI_ASSERT(!garbage.m_freedDecriptorMask.getBit(arrayIndex));
	garbage.m_freedDecriptorMask.setBit(arrayIndex);

	ANKI_ASSERT(!m_pendingDeletionMask.getBit(arrayIndex));
	m_pendingDeletionMask.setBit(arrayIndex);
}

void StreamingImageResourceManager::appendStreamingRequests(ConstWeakArray<ImageStreamingRequest> requests)
{
	g_svarRsrcStreamingRequestCount.increment(requests.getSize());

	if(requests.getSize() == 0)
	{
		return;
	}

	for(U32 i = 0; i < requests.getSize(); ++i)
	{
		ANKI_ASSERT(requests[i].m_descriptorIndex != 0);

		for(U32 j = 0; j < requests.getSize(); ++j)
		{
			if(i == j)
			{
				continue;
			}

			ANKI_ASSERT(requests[i].m_descriptorIndex != requests[j].m_descriptorIndex && "Can't have duplicates");
		}
	}

	LockGuard lock(m_mtx);

	for(const ImageStreamingRequest& req : requests)
	{
		if(isArrayIndexFree(req.m_descriptorIndex))
		{
			// It got freed, skip
			continue;
		}

		if(m_pendingDeletionMask.getBit(req.m_descriptorIndex))
		{
			// Pending deletion, skip
			continue;
		}

		StreamingInfo& streamingInf = m_streamingInfos[req.m_descriptorIndex];

		if(req.m_resourceUuid != streamingInf.m_resourceUuid)
		{
			// Something got deleted and the slot got recycled, skip
			continue;
		}

		if(streamingInf.m_lastSeenFrame != m_frame)
		{
			streamingInf.m_lastSeenFrame = m_frame;
			streamingInf.m_lastSeenDetailedMip = req.m_mipmap;
		}
		else
		{
			// Already a pending request for this image this frame, merge the requests
			streamingInf.m_lastSeenDetailedMip = min(streamingInf.m_lastSeenDetailedMip, U8(req.m_mipmap));
		}

		if(req.m_mipmap < streamingInf.m_detailedLoadedMip)
		{
			// Asks for more detailed mip than what it has, remember it
			m_uploadDescriptorMask.setBit(req.m_descriptorIndex);
		}
	}
}

void StreamingImageResourceManager::performStreaming(ResourceDynamicArray<IntrusiveNoDelPtr<StreamingImage>>& retainedImages)
{
	const PtrSize originalTexMemPoolEstimatedSize = TextureMemoryPool::getSingleton().getAllocatedSize();

	auto isUnderMemPressure = [](PtrSize size) -> Bool {
		const PtrSize maxTexMempoolSize = g_cvarGpuMemTextureMemoryPoolChunkSize * g_cvarGpuMemTextureMemoryPoolMaxChunks;
		return F64(size) / F64(maxTexMempoolSize) >= g_cvarRsrcMaxTextureMemoryLoadFactor;
	};

	// Gather the upload requests
	PtrSize intendedUploadSize = 0;
	U32 mipmapsToUploadCount = g_cvarRsrcMaxMipmapUploadsPerFrame;
	ResourceDynamicArray<LoadMipsRequest> loadRequests;
	m_uploadDescriptorMask.iterateSetBitsFromLeastSignificant([&](U32 descIdx) {
		if(isArrayIndexFree(descIdx))
		{
			// It got freed, skip
			return FunctorContinue::kContinue;
		}

		if(m_pendingDeletionMask.getBit(descIdx))
		{
			// Pending deletion, skip
			return FunctorContinue::kContinue;
		}

		StreamingInfo& streamingInf = m_streamingInfos[descIdx];
		StreamingImage& img = m_imageData[descIdx];

		const I32 refcount = img.retain();
		if(refcount == 1)
		{
			// Image is about to be deleted but StreamingImageResourceManager::release() hasn't been called yet, skip
			return FunctorContinue::kContinue;
		}
		retainedImages.emplaceBack(&img);
		img.release();

		ANKI_ASSERT(streamingInf.m_lastSeenDetailedMip < streamingInf.m_detailedLoadedMip);

		const U32 mipCount = 1;
		const U32 mip = streamingInf.m_detailedLoadedMip - 1;

		const PtrSize crntUploadSize = img.estimateSurfaceMemoryConsumption(mip, mipCount);
		intendedUploadSize += crntUploadSize;
		if(mipmapsToUploadCount == 0 || isUnderMemPressure(originalTexMemPoolEstimatedSize + intendedUploadSize))
		{
			return FunctorContinue::kContinue;
		}

		loadRequests.emplaceBack(LoadMipsRequest{.m_img{&img}, .m_arrayIndex = descIdx, .m_firstMip = U8(mip), .m_mipCount = mipCount});

		streamingInf.m_detailedLoadedMip = U8(mip);

		--mipmapsToUploadCount;

		return FunctorContinue::kContinue;
	});

	// Fire the uplod requests in batches, we don't a single huge async task that monopolizes the async loader
	if(loadRequests.getSize())
	{
		constexpr U32 kMaxTexturesPerAsyncJob = 4;
		const U32 batchCount = (loadRequests.getSize() + kMaxTexturesPerAsyncJob - 1) / kMaxTexturesPerAsyncJob;
		for(U32 batch = 0; batch < batchCount; ++batch)
		{
			const U32 first = batch * kMaxTexturesPerAsyncJob;
			const U32 end = min(loadRequests.getSize(), (batch + 1) * kMaxTexturesPerAsyncJob);
			loadOtherMips(ConstWeakArray<LoadMipsRequest>(loadRequests.getBegin() + first, loadRequests.getBegin() + end));
		}
	}
	m_uploadDescriptorMask.destroy();

	// Evict mips to reduce mem pressure
	const Bool underMemPressure = isUnderMemPressure(originalTexMemPoolEstimatedSize + intendedUploadSize);

	if((underMemPressure && (originalTexMemPoolEstimatedSize < m_texPoolAllocatedSizeOnPrevEviction || m_framesSinceLastEviction >= 4))
	   || m_forceEvictAll)
	{
		// We are under pressure and there was some memory freed from previous evictions

		// Gather the candidates
		ResourceDynamicArray<std::pair<StreamingInfo*, U32>> streamingInfos;
		for(auto it = m_streamingInfos.getBegin(); it != m_streamingInfos.getEnd(); ++it)
		{
			if(it.getArrayIndex() == 0)
			{
				continue;
			}

			if(m_pendingDeletionMask.getBit(it.getArrayIndex()))
			{
				// Pending deletion, skip
				continue;
			}

			if(it->m_detailedLoadedMip == it->m_tailChainMip)
			{
				// Image at its mip limit, can't evict more
				continue;
			}

			if(!m_forceEvictAll && it->m_lastSeenFrame + g_cvarRsrcFramesUntilEviction >= m_frame)
			{
				// Image has been seen somewhat recently, can't evict it just yet
				continue;
			}

			StreamingImage& img = m_imageData[it.getArrayIndex()];

			const I32 refcount = img.retain();
			if(refcount == 1)
			{
				// Image is about to be deleted but StreamingImageResourceManager::release() hasn't been called yet, skip
				continue;
			}
			retainedImages.emplaceBack(&img);
			img.release();

			streamingInfos.emplaceBack(std::make_pair(&(*it), it.getArrayIndex()));
		}

		// Sort candidates from previously seen to newly seen
		std::sort(streamingInfos.getBegin(), streamingInfos.getEnd(), [](auto& a, auto& b) {
			return a.first->m_lastSeenFrame < b.first->m_lastSeenFrame;
		});

		// Gather mem frees until we are no longer under mem pressure
		ResourceDynamicArray<LoadMipsRequest> requests;
		PtrSize memSize = originalTexMemPoolEstimatedSize + intendedUploadSize;
		for(std::pair<StreamingInfo*, U32>& streamingPair : streamingInfos)
		{
			StreamingInfo& streaming = *streamingPair.first;
			StreamingImage& img = m_imageData[streamingPair.second];

			const U32 firstMip = streaming.m_detailedLoadedMip;
			ANKI_ASSERT(streaming.m_detailedLoadedMip < streaming.m_tailChainMip);
			const U32 mipCount = (m_forceEvictAll) ? (streaming.m_tailChainMip - streaming.m_detailedLoadedMip) : 1;

			const PtrSize crntMemToFree = img.estimateSurfaceMemoryConsumption(firstMip, mipCount);

			requests.emplaceBack(
				LoadMipsRequest{.m_img{&img}, .m_arrayIndex = streamingPair.second, .m_firstMip = U8(firstMip), .m_mipCount = U8(mipCount)});

			streaming.m_detailedLoadedMip = U8(firstMip + mipCount);

			if(!m_forceEvictAll)
			{
				ANKI_ASSERT(memSize >= crntMemToFree);
				memSize -= crntMemToFree;
				if(!isUnderMemPressure(memSize))
				{
					break;
				}
			}
		}

		// Now submit the eviction requests
		if(requests.getSize())
		{
			unloadOtherMips(requests);

			m_texPoolAllocatedSizeOnPrevEviction = originalTexMemPoolEstimatedSize;

			m_framesSinceLastEviction = 0;
		}
	}
	else
	{
		++m_framesSinceLastEviction;
	}

	if(!underMemPressure)
	{
		m_texPoolAllocatedSizeOnPrevEviction = kMaxPtrSize;
	}

	m_forceEvictAll = false;
}

void StreamingImageResourceManager::endFrame(Fence* fence)
{
	ANKI_ASSERT(fence);

	ResourceDynamicArray<IntrusiveNoDelPtr<StreamingImage>> retainedImages; // Should be released outside the lock

	{
		LockGuard lock(m_mtx);

		// Set the new fence
		if(m_garbage.getSize() && !m_garbage.getBack().m_fence)
		{
			m_garbage.getBack().m_fence.reset(fence);
		}

		collectGarbage(false);

		performStreaming(retainedImages);

		++m_frame;
	}
}

void StreamingImageResourceManager::collectGarbage(Bool forceFullCleanup)
{
	while(m_garbage.getSize() && (forceFullCleanup || (m_garbage.getFront().m_fence && m_garbage.getFront().m_fence->signaled())))
	{
		// Fence is signaled, collect the garbage

		Garbage& garbage = m_garbage.getFront();

		// Realy free the element of the array
		garbage.m_freedDecriptorMask.iterateSetBitsFromLeastSignificant([this](U32 bit) {
			ANKI_ASSERT(m_pendingDeletionMask.getBit(bit));
			m_pendingDeletionMask.unsetBit(bit);

			m_imageData.erase(bit);
			m_streamingInfos.erase(bit);

			return FunctorContinue::kContinue;
		});

		// Free texture memory
		for(TextureMemoryPoolAllocation& alloc : garbage.m_allocsToFree)
		{
			alloc.deferredFree();
		}

		// Free textures
		garbage.m_texturesToDelete.destroy();

		// Done
		m_garbage.erase(m_garbage.getBegin());
	}
}

void StreamingImageResourceManager::freeImage(StreamingImage* img)
{
	if(img)
	{
		img->release();
	}
}

Bool StreamingImageResourceManager::isImageLoaded(StreamingImage* img)
{
	ANKI_ASSERT(img);
	return img->m_isLoaded.load(AtomicMemoryOrder::kAcquire); // Make sure that we load the atomic is not re-ordered by the compiler
}

Texture& StreamingImageResourceManager::getImageTailChainTexture(StreamingImage* img)
{
	ANKI_ASSERT(img && isImageLoaded(img));
	return *img->m_textures[img->m_textureCount - 1];
}

Vec4 StreamingImageResourceManager::getImageAverageColor(StreamingImage* img)
{
	ANKI_ASSERT(img && isImageLoaded(img));
	return img->m_avgColor;
}

} // end namespace anki
