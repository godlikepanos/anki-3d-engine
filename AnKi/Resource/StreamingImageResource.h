// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/GpuMemory/TextureMemoryPool.h>
#include <AnKi/Shaders/ImageStreaming.h>
#include <AnKi/Util/DynamicBitSet.h>

namespace anki {

// Forward
class ImageLoader;

ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageSize2, kImageDescriptorMaxTextureSize, kImageDescriptorSmallestMipmapSize, kImageDescriptorMaxTextureSize,
		  "Max image size to load")
ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageDescriptors, 1024, 32, kMaxU32, "The size of the ImageDescriptor structured buffer")
ANKI_CVAR(NumericCVar<PtrSize>, Rsrc, MaxTextureMemoryPoolSize, 2_GB, 1_GB, 16_GB, "Try to have TextureMemoryPool be bellow that size")
ANKI_CVAR(NumericCVar<F32>, Rsrc, MaxTextureMemoryLoadFactor, 0.7f, 0.1f, 1.0f,
		  "If the texture memory pool load is above this the streaming manager stops loading mipmaps and starts freeing them")
ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxMipmapUploadsPerFrame, 32, 1, kMaxU32, "How many mips to upload per frame")
ANKI_CVAR(NumericCVar<U32>, Rsrc, FramesUntilEviction, 1 * 60, 1, 100 * 60, "An image will be evicted if not seen that many frames")

using ImageDescriptorHandle = U32;

// A system that handles some GPU memory required for streaming images
class StreamingImageResourceManager : public MakeSingleton<StreamingImageResourceManager>
{
	friend class StreamingImageResource;

public:
	StreamingImageResourceManager()
	{
		init();
	}

	~StreamingImageResourceManager();

	BufferView getBuffer() const
	{
		return m_imageDescriptorsBuff;
	}

	// Thread-safe
	void appendStreamingRequests(ConstWeakArray<StreamingImageRequest> requests);

	// Call it from the main thread
	void endFrame(Fence* fence);

private:
	class Garbage
	{
	public:
		ResourceDynamicBitSet<U32> m_freedDecriptorMask;
		FencePtr m_fence;
	};

	class ResourceBookkeeping
	{
	public:
		StreamingImageResource* m_image = nullptr;
		U64 m_lastSeenFrame = 0;
		U32 m_resourceUuid = kMaxU32;
		U8 m_lastSeenDetailedMip = kMaxU8;
		U8 m_detailedLoadedMip = kMaxU8;
		U8 m_tailChainMip = kMaxU8;
	};

	TextureMemoryPoolAllocation m_imageDescriptorsBuff;

	ResourceDynamicBitSet<U64> m_freeDescriptorMask;

	ResourceDynamicArray<ResourceBookkeeping> m_resources;

	ResourceDynamicArray<Garbage> m_garbage;

	ResourceDynamicBitSet<U64> m_uploadDescriptorMask; // Descriptors that have streaming requests

	mutable Mutex m_mtx;

	U64 m_frame = 1;

	void init();

	void collectGarbage(Bool waitForFences);

	void performStreaming();

	// Thread-safe
	ImageDescriptorHandle newImageDescriptor(StreamingImageResource* image, U8 tailChainMip);

	// Thread-safe
	void freeImageDescriptor(ImageDescriptorHandle index);

	// Thread-safe
	void uploadImageDescriptor(ImageDescriptorHandle index, const ImageDescriptor& desc) const;
};

// Image resource class. It creates textures and then loads them in the GPU. It supports the formats of the ImageLoader.
class StreamingImageResource : public ResourceObject
{
	friend class StreamingImageResourceManager;

public:
	StreamingImageResource(CString fname, U32 uuid)
		: ResourceObject(fname, uuid, ResourceType::kStreamingImageResource)
	{
	}

	~StreamingImageResource();

	Error load(const ResourceFilename& filename, Bool async);

	Vec4 getAverageColor() const
	{
		return m_avgColor;
	}

	// Returns true if the tail mip chain has been uploaded to the GPU
	Bool isLoaded() const
	{
		return m_isLoaded.load() == 1;
	}

	ImageDescriptorHandle getImageDescriptorHandle() const
	{
		ANKI_ASSERT(m_imageDescHandle < kMaxU32);
		return m_imageDescHandle;
	}

	Texture& getTailChainTexture() const;

private:
	Vec4 m_avgColor = Vec4(0.0f);

	Array<TextureMemoryPoolAllocation, kImageDescriptorMaxBindlessTextures> m_texAllocations;
	Array<TexturePtr, kImageDescriptorMaxBindlessTextures> m_textures;

	ImageDescriptorHandle m_imageDescHandle = kMaxU32;

	Format m_format = Format::kNone; // Cache it

	ImageDescriptor m_imageDesc = {};

	mutable Atomic<Bool> m_isLoaded = {false};

	U8 m_textureCount = 0;
	U8 m_mipCount = 0;

	TextureType m_texType = TextureType::kCount;

	// Called only by load(). It loads the tail chain texture
	Error loadTailMipChainAsync(ImageLoader& loader) const;

	// Called by loadNonTailMips(). It runs in the async thread and loads non-tail chain textures
	Error loadNonTailMipsAsync(U32 firstMip, U32 mipCount);

	// The opposite of loadNonTailMipsAsync
	void unloadNonTailMipsAsync(U32 firstMip, U32 mipCount);

	// Should be called from the main thread
	// Not thread-safe
	void submitLoadsOfNonTailMips(U32 firstMip, U32 mipCount);

	// Should be called from the main thread
	// Not thread-safe
	void submitUnloadsOfNonTailMips(U32 firstMip, U32 mipCount);

	PtrSize estimateSurfaceMemoryConsumption(U32 firstMip, U32 mipCount) const;
};

} // end namespace anki
