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

StreamingImageResourceManager::~StreamingImageResourceManager()
{
	for(Garbage& g : m_garbage)
	{
		if(g.m_fence)
		{
			g.m_fence->clientWaitForever();
		}
	}

	collectGarbage(false);

	ANKI_ASSERT(m_garbage.getSize() == 0);
	ANKI_ASSERT(m_freeDescriptorMask.countSetBits() == g_cvarRsrcMaxImageDescriptors - 1 && "Forgot to free image desciptor");
}

void StreamingImageResourceManager::init()
{
	m_imageDescriptorsBuff = TextureMemoryPool::getSingleton().allocateStructuredBuffer<ImageDescriptor>(g_cvarRsrcMaxImageDescriptors);

	for(U32 i = 0; i < g_cvarRsrcMaxImageDescriptors; ++i)
	{
		m_freeDescriptorMask.setBit(i);
	}

	// The 0 descriptor is reserved so that materials can use that to indicate that there is no texture bound
	m_freeDescriptorMask.unsetBit(0);
	m_resources.resize(1);
	uploadImageDescriptor(0, ImageDescriptor{});
}

ImageDescriptorHandle StreamingImageResourceManager::newImageDescriptor(StreamingImageResource* image, U8 tailChainMip)
{
	ANKI_ASSERT(image);
	LockGuard lock(m_mtx);

	const U32 index = m_freeDescriptorMask.getLeastSignificantBit();
	if(index == kMaxU32) [[unlikely]]
	{
		ANKI_RESOURCE_LOGF("Out of free descriptors. Increase %s", g_cvarRsrcMaxImageDescriptors.getName().cstr());
	}

	m_freeDescriptorMask.unsetBit(index);

	if(index >= m_resources.getSize())
	{
		m_resources.resize(index + 1);
	}

	ANKI_ASSERT(m_resources[index].m_image == nullptr);
	m_resources[index].m_image = image;
	m_resources[index].m_resourceUuid = image->getUuid();
	m_resources[index].m_lastSeenDetailedMip = tailChainMip;
	m_resources[index].m_detailedLoadedMip = tailChainMip;
	m_resources[index].m_tailChainMip = tailChainMip;

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

	// Invalidate the bookkeeping now to avoid doing streaming requests
	ANKI_ASSERT(m_resources[index].m_image != nullptr);
	m_resources[index] = ResourceBookkeeping();

	m_uploadDescriptorMask.unsetBit(index);
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

	// Set the new fence
	if(m_garbage.getSize() && !m_garbage.getBack().m_fence)
	{
		m_garbage.getBack().m_fence.reset(fence);
	}

	collectGarbage(true);

	performStreaming();

	++m_frame;
}

void StreamingImageResourceManager::collectGarbage(Bool waitForFences)
{
	while(m_garbage.getSize() && (!waitForFences || (m_garbage.getFront().m_fence && m_garbage.getFront().m_fence->signaled())))
	{
		// Fence is signaled, collect the garbage

		m_garbage.getFront().m_freedDecriptorMask.iterateSetBitsFromLeastSignificant([this](U32 bit) {
			ANKI_ASSERT(!m_freeDescriptorMask.getBit(bit));
			m_freeDescriptorMask.setBit(bit);

			return FunctorContinue::kContinue;
		});

		m_garbage.erase(m_garbage.getBegin());
	}
}

void StreamingImageResourceManager::appendStreamingRequests(ConstWeakArray<StreamingImageRequest> requests)
{
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

	for(const StreamingImageRequest& req : requests)
	{
		const Bool isFree = m_freeDescriptorMask.getBit(req.m_descriptorIndex);
		if(isFree)
		{
			// It got freed, skip
			continue;
		}

		if(m_resources[req.m_descriptorIndex].m_image == nullptr)
		{
			// Pending deletion, skip
			continue;
		}

		if(req.m_resourceUuid != m_resources[req.m_descriptorIndex].m_resourceUuid)
		{
			// Something got deleted and the slot got recycled, skip
			continue;
		}

		ResourceBookkeeping& resource = m_resources[req.m_descriptorIndex];

		if(resource.m_lastSeenFrame != m_frame)
		{
			resource.m_lastSeenFrame = m_frame;
			resource.m_lastSeenDetailedMip = req.m_mipmap;
		}
		else
		{
			// Already a pending request for this image this frame, merge the requests
			resource.m_lastSeenDetailedMip = min(resource.m_lastSeenDetailedMip, U8(req.m_mipmap));
		}

		if(req.m_mipmap < resource.m_detailedLoadedMip)
		{
			// Asks for more detailed mip than what it has, remember it
			m_uploadDescriptorMask.setBit(req.m_descriptorIndex);
		}
	}
}

void StreamingImageResourceManager::performStreaming()
{
	const PtrSize originalTexMemPoolEstimatedSize = TextureMemoryPool::getSingleton().getAllocatedSize();

	auto isUnderMemPressure = [](PtrSize size) -> Bool {
		return F64(size) / F64(g_cvarRsrcMaxTextureMemoryPoolSize) >= g_cvarRsrcMaxTextureMemoryLoadFactor;
	};

	// Do the uploads
	PtrSize intendedUploadSize = 0;
	U32 mipmapsToUploadCount = g_cvarRsrcMaxMipmapUploadsPerFrame;
	m_uploadDescriptorMask.iterateSetBitsFromLeastSignificant([&](U32 descIdx) {
		ResourceBookkeeping& resource = m_resources[descIdx];

		ANKI_ASSERT(resource.m_lastSeenDetailedMip < resource.m_detailedLoadedMip);

		const U32 mipCount = 1;
		const U32 mip = resource.m_detailedLoadedMip - 1;

		const PtrSize crntUploadSize = resource.m_image->estimateSurfaceMemoryConsumption(mip, mipCount);
		intendedUploadSize += crntUploadSize;
		if(mipmapsToUploadCount == 0 || isUnderMemPressure(originalTexMemPoolEstimatedSize + intendedUploadSize))
		{
			return FunctorContinue::kContinue;
		}

		resource.m_image->submitLoadsOfNonTailMips(mip, mipCount);
		resource.m_detailedLoadedMip = U8(mip);

		--mipmapsToUploadCount;

		return FunctorContinue::kContinue;
	});

	// Evict mips to reduce mem pressure
	if(isUnderMemPressure(originalTexMemPoolEstimatedSize + intendedUploadSize))
	{
		// Gather the candidates
		ResourceDynamicArray<ResourceBookkeeping*> resourcesToReduceMips;
		for(auto it = m_resources.getBegin() + 1; it < m_resources.getEnd(); ++it)
		{
			if(it->m_image == nullptr)
			{
				continue;
			}

			if(it->m_detailedLoadedMip == it->m_tailChainMip)
			{
				// Image at its mip limit, can't evict more
				continue;
			}

			if(it->m_lastSeenDetailedMip <= it->m_detailedLoadedMip)
			{
				// Image wants more detail mips, can't evict that one
				continue;
			}

			if(it->m_lastSeenFrame + g_cvarRsrcFramesUntilEviction >= m_frame)
			{
				// Image has been seen somewhat recently, can't evict it just yet
				continue;
			}

			resourcesToReduceMips.emplaceBack(&(*it));
		}

		// Sort candidates from previously seen to newly seen
		std::sort(resourcesToReduceMips.getBegin(), resourcesToReduceMips.getEnd(), [](ResourceBookkeeping* a, ResourceBookkeeping* b) {
			return a->m_lastSeenFrame < b->m_lastSeenFrame;
		});

		// Execute mem frees until we are no longer under mem pressure
		PtrSize memSize = originalTexMemPoolEstimatedSize + intendedUploadSize;
		for(ResourceBookkeeping* rsrc : resourcesToReduceMips)
		{
			const U32 firstMip = rsrc->m_detailedLoadedMip;
			const U32 mipCount = 1;

			const PtrSize crntMemToFree = rsrc->m_image->estimateSurfaceMemoryConsumption(firstMip, mipCount);

			rsrc->m_image->submitUnloadsOfNonTailMips(firstMip, mipCount);

			rsrc->m_detailedLoadedMip = U8(firstMip + mipCount);

			ANKI_ASSERT(memSize >= crntMemToFree);
			memSize -= crntMemToFree;
			if(!isUnderMemPressure(memSize))
			{
				break;
			}
		}
	}

	// Done
	m_uploadDescriptorMask.destroy();
}

StreamingImageResource::~StreamingImageResource()
{
	if(m_imageDescHandle != kMaxU32)
	{
		StreamingImageResourceManager::getSingleton().freeImageDescriptor(m_imageDescHandle);
	}
}

Error StreamingImageResource::load(const ResourceFilename& filename, Bool async)
{
	class TexUploadTask : public AsyncLoaderTask
	{
	public:
		ImageLoader m_loader{&ResourceMemoryPool::getSingleton()};
		StreamingImageResourcePtr m_image;

		Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
		{
			return m_image->loadTailMipChainAsync(m_loader);
		}
	};

	TexUploadTask* task = nullptr;
	ANKI_DEFER(deleteInstance(ResourceMemoryPool::getSingleton(), task));

	ImageLoader localLoader{&ResourceMemoryPool::getSingleton()};
	ImageLoader* loader = nullptr;

	if(async)
	{
		task = AsyncLoader::getSingleton().newTask<TexUploadTask>();
		loader = &task->m_loader;
		task->m_image.reset(this);
	}
	else
	{
		loader = &localLoader;
	}

	ANKI_CHECK(loader->loadHeaderFromResourceFile(filename, g_cvarRsrcMaxImageSize2));

	U32 minDimension = min(loader->getWidth(), loader->getHeight());
	if(loader->getImageType() == ImageBinaryType::k3D)
	{
		minDimension = min(minDimension, loader->getDepth());
	}
	if(minDimension < kImageDescriptorSmallestMipmapSize)
	{
		ANKI_RESOURCE_LOGE("Can't have images with a dimension less than %u", kImageDescriptorSmallestMipmapSize);
		return Error::kUserData;
	}

	m_avgColor = loader->getAverageColor();

	m_mipCount = (loader->getImageType() != ImageBinaryType::k3D)
					 ? computeMaxMipmapCount2d(loader->getWidth(), loader->getHeight(), kImageDescriptorSmallestMipmapSize)
					 : computeMaxMipmapCount3d(loader->getWidth(), loader->getHeight(), loader->getDepth(), kImageDescriptorSmallestMipmapSize);
	m_mipCount = min(m_mipCount, U8(loader->getMipmapCount()));

	m_textureCount = U8(max(I32(m_mipCount) - I32(kImageDescriptorTailChainMipmapCount) + 1, 1));

	const U8 tailChainTexIdx = m_textureCount - 1;

	// Create the tail chain texture
	{
		TextureInitInfo texInit;
		fillTextureInitInfo(*loader, texInit);
		texInit.m_mipmapCount = min(U8(kImageDescriptorTailChainMipmapCount), m_mipCount);
		texInit.m_width = loader->getWidth() >> tailChainTexIdx;
		texInit.m_height = loader->getHeight() >> tailChainTexIdx;
		if(texInit.m_type == TextureType::k3D)
		{
			texInit.m_depth = loader->getDepth() >> tailChainTexIdx;
		}

		const String filenameExt = anki::getFilename(filename);
		texInit.setName(ResourceString().sprintf("%s tail", filenameExt.cstr()));

		const PtrSize memReq = GrManager::getSingleton().getTextureMemoryRequirement(texInit);
		m_texAllocations[tailChainTexIdx] = TextureMemoryPool::getSingleton().allocate(memReq);

		texInit.m_memoryBuffer = m_texAllocations[tailChainTexIdx];
		m_textures[tailChainTexIdx] = GrManager::getSingleton().newTexture(texInit);

		m_format = m_textures[tailChainTexIdx]->getFormat();
		m_texType = m_textures[tailChainTexIdx]->getTextureType();
	}

	// Create the image descriptor
	{
		m_imageDesc.m_width = loader->getWidth();
		m_imageDesc.m_height = loader->getHeight();
		if(loader->getImageType() == ImageBinaryType::k3D)
		{
			m_imageDesc.m_depthOrLayerCount = loader->getDepth();
		}
		else if(loader->getImageType() == ImageBinaryType::k2DArray)
		{
			m_imageDesc.m_depthOrLayerCount = loader->getLayerCount();
		}
		else
		{
			m_imageDesc.m_depthOrLayerCount = 1;
		}

		m_imageDesc.m_firstMipmap = tailChainTexIdx;
		m_imageDesc.m_lastMipmap = m_mipCount - 1;

		m_imageDesc.m_resourceUuid = getUuid();

		const U32 tailChainTexBindlessIdx = m_textures[tailChainTexIdx]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());

		for(U32 i = tailChainTexIdx; i < m_mipCount; ++i)
		{
			U32 packedBindlessIndexAndLod = tailChainTexBindlessIdx << 8u;
			packedBindlessIndexAndLod |= U8(i - tailChainTexIdx);
			ANKI_ASSERT(packedBindlessIndexAndLod >> 8u == tailChainTexBindlessIdx);

			m_imageDesc.m_bindlessTextureIndexAndLod[i] = packedBindlessIndexAndLod;
		}

		m_imageDescHandle = StreamingImageResourceManager::getSingleton().newImageDescriptor(this, m_textureCount - 1);
		StreamingImageResourceManager::getSingleton().uploadImageDescriptor(m_imageDescHandle, m_imageDesc);
	}

	// Upload the data
	if(async)
	{
		AsyncLoader::getSingleton().submitTask(task, AsyncLoaderPriority::kHigh);
		task = nullptr;
	}
	else
	{
		ANKI_CHECK(loadTailMipChainAsync(*loader));
	}

	return Error::kNone;
}

Error StreamingImageResource::loadTailMipChainAsync(ImageLoader& loader) const
{
	const U32 firstMipmapOfTailChain = m_textureCount - 1;
	Texture& chainTex = *m_textures[firstMipmapOfTailChain];
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

			for(U32 mip = firstMipmapOfTailChain; mip < m_mipCount; ++mip)
			{
				const U32 tailChainMip = mip - firstMipmapOfTailChain;

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
			for(U32 mip = firstMipmapOfTailChain; mip < m_mipCount; ++mip)
			{
				const U32 tailChainMip = mip - firstMipmapOfTailChain;

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

			for(U32 mip = firstMipmapOfTailChain; mip < m_mipCount; ++mip)
			{
				const U32 tailChainMip = mip - firstMipmapOfTailChain;

				barriers[barrierCount++] = {TextureView(&chainTex, TextureSubresourceDesc::surface(tailChainMip, f, l)),
											TextureUsageBit::kCopyDestination, TextureUsageBit::kAllSrv};
			}

			CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});
		}
	}

	m_isLoaded.exchange(true);
	return Error::kNone;
}

Error StreamingImageResource::loadNonTailMipsAsync(U32 firstMip, U32 mipCount)
{
	const U32 firstMipmapOfTailChain = m_textureCount - 1;
	ANKI_ASSERT(firstMip + mipCount <= firstMipmapOfTailChain);

	ImageLoader loader{&ResourceMemoryPool::getSingleton()};
	ANKI_CHECK(loader.loadHeaderFromResourceFile(getFilename(), g_cvarRsrcMaxImageSize2));

	const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;
	const U32 layerCount = (m_texType == TextureType::k3D) ? 1 : m_imageDesc.m_depthOrLayerCount;

	// With GFXR enabled we can't do fwrite directly to mapped VkBuffer. So we need to first fwrite to a CPU buffer and copy that to the mapped
	// VkBuffer
	const Bool bGfxreconstruct = GrManager::getSingleton().getDeviceCapabilities().m_gfxReconstruct;

	// Create the textures
	const String filenameExt = anki::getFilename(getFilename());
	for(U32 mip = firstMip; mip < firstMip + mipCount; ++mip)
	{
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
		ANKI_ASSERT(!m_texAllocations[mip]);
		m_texAllocations[mip] = TextureMemoryPool::getSingleton().allocate(memReq);

		texInit.m_memoryBuffer = m_texAllocations[mip];
		ANKI_ASSERT(!m_textures[mip]);
		m_textures[mip] = GrManager::getSingleton().newTexture(texInit);
	}

	// Set barriers
	Array<TextureBarrierInfo, kImageDescriptorMaxMipmaps - kImageDescriptorTailChainMipmapCount> barriers;
	U32 barrierCount = 0;
	for(U32 l = 0; l < layerCount; ++l)
	{
		for(U32 f = 0; f < faceCount; ++f)
		{
			barrierCount = 0;

			for(U32 mip = firstMip; mip < firstMip + mipCount; ++mip)
			{
				barriers[barrierCount++] = {TextureView(m_textures[mip].get(), TextureSubresourceDesc::surface(0, f, l)), TextureUsageBit::kNone,
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
			for(U32 mip = firstMip; mip < firstMip + mipCount; ++mip)
			{
				Texture& tex = *m_textures[mip];

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

				ANKI_CHECK(loader.loadSurfaceOrVolume(mip, f, l, copyDest));

				if(bGfxreconstruct)
				{
					memcpy(mappedMem.getBegin(), copyDest.getBegin(), copyDest.getSizeInBytes());
				}
			}
		}
	}

	// Set the post copy barriers
	for(U32 l = 0; l < layerCount; ++l)
	{
		for(U32 f = 0; f < faceCount; ++f)
		{
			barrierCount = 0;

			for(U32 mip = firstMip; mip < firstMip + mipCount; ++mip)
			{
				barriers[barrierCount++] = {TextureView(m_textures[mip].get(), TextureSubresourceDesc::surface(0, f, l)),
											TextureUsageBit::kCopyDestination, TextureUsageBit::kAllSrv};
			}

			CopyEngine::getSingleton().setPipelineBarrier({&barriers[0], barrierCount}, {}, {});
		}
	}

	// Update the descriptor
	// WARNING. Two problems with this:
	// - The update of the descriptor might happen in the async queue while the renderer is accessing it
	// - The barrier is not ideal but it's enough to block the desc update until the texture copies above have completed
	{
		const BufferBarrierInfo buffBarr = {StreamingImageResourceManager::getSingleton().getBuffer(), BufferUsageBit::kAll,
											BufferUsageBit::kCopyDestination};
		CopyEngine::getSingleton().setPipelineBarrier({}, {&buffBarr, 1}, {});

		m_imageDesc.m_firstMipmap = firstMip;

		for(U32 mip = firstMip; mip < firstMip + mipCount; ++mip)
		{
			ANKI_ASSERT(m_imageDesc.m_bindlessTextureIndexAndLod[mip] == 0);
			m_imageDesc.m_bindlessTextureIndexAndLod[mip] = m_textures[mip]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all()) << 8u;
		}

		StreamingImageResourceManager::getSingleton().uploadImageDescriptor(m_imageDescHandle, m_imageDesc);
	}

	return Error::kNone;
}

void StreamingImageResource::unloadNonTailMipsAsync(U32 firstMip, U32 mipCount)
{
	const U32 firstMipmapOfTailChain = m_textureCount - 1;
	ANKI_ASSERT(firstMip + mipCount <= firstMipmapOfTailChain);

	const U8 minLoadedMip = U8(firstMip + mipCount);

	m_imageDesc.m_firstMipmap = minLoadedMip;
	for(U32 mip = firstMip; mip < firstMip + mipCount; ++mip)
	{
		ANKI_ASSERT(m_imageDesc.m_bindlessTextureIndexAndLod[mip] != 0);
		m_imageDesc.m_bindlessTextureIndexAndLod[mip] = 0;

		ANKI_ASSERT(!!m_textures[mip]);
		m_textures[mip].reset(nullptr);
		ANKI_ASSERT(!!m_texAllocations[mip]);
		m_texAllocations[mip].free();
	}

	const BufferBarrierInfo buffBarr = {StreamingImageResourceManager::getSingleton().getBuffer(), BufferUsageBit::kAll,
										BufferUsageBit::kCopyDestination};
	CopyEngine::getSingleton().setPipelineBarrier({}, {&buffBarr, 1}, {});

	StreamingImageResourceManager::getSingleton().uploadImageDescriptor(m_imageDescHandle, m_imageDesc);
}

void StreamingImageResource::submitLoadsOfNonTailMips(U32 firstMip, U32 mipCount)
{
	const U32 firstMipmapOfTailChain = m_textureCount - 1;
	ANKI_ASSERT(firstMip + mipCount <= firstMipmapOfTailChain);

	class MyTask : public AsyncLoaderTask
	{
	public:
		StreamingImageResourcePtr m_image;
		U32 m_firstMipToLoad;
		U32 m_mipCountToLoad;

		Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
		{
			return m_image->loadNonTailMipsAsync(m_firstMipToLoad, m_mipCountToLoad);
		}
	};

	MyTask* task = AsyncLoader::getSingleton().newTask<MyTask>();
	task->m_image.reset(this);
	task->m_firstMipToLoad = firstMip;
	task->m_mipCountToLoad = mipCount;

	AsyncLoader::getSingleton().submitTask(task, AsyncLoaderPriority::kMedium);
}

void StreamingImageResource::submitUnloadsOfNonTailMips(U32 firstMip, U32 mipCount)
{
	const U32 firstMipmapOfTailChain = m_textureCount - 1;
	ANKI_ASSERT(firstMip + mipCount <= firstMipmapOfTailChain);

	class MyTask : public AsyncLoaderTask
	{
	public:
		StreamingImageResourcePtr m_image;
		U32 m_firstMipToUnload;
		U32 m_mipCountToUnload;

		Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
		{
			m_image->unloadNonTailMipsAsync(m_firstMipToUnload, m_mipCountToUnload);
			return Error::kNone;
		}
	};

	MyTask* task = AsyncLoader::getSingleton().newTask<MyTask>();
	task->m_image.reset(this);
	task->m_firstMipToUnload = firstMip;
	task->m_mipCountToUnload = mipCount;

	AsyncLoader::getSingleton().submitTask(task, AsyncLoaderPriority::kMedium);
}

Texture& StreamingImageResource::getTailChainTexture() const
{
	return *m_textures[m_textureCount - 1];
}

PtrSize StreamingImageResource::estimateSurfaceMemoryConsumption(U32 firstMip, U32 mipCount) const
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

} // end namespace anki
