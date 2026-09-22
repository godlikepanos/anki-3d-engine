// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/GpuMemory/TextureMemoryPool.h>
#include <AnKi/Shaders/ImageStreaming.h>
#include <AnKi/Util/BlockArray.h>
#include <AnKi/Util/DynamicBitSet.h>

namespace anki {

// Forward
class ImageLoader;

ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageDescriptors, 1024, 32, kMaxU32, "The size of the ImageDescriptor structured buffer")
ANKI_CVAR(NumericCVar<F32>, Rsrc, MaxTextureMemoryLoadFactor, 0.7f, 0.1f, 1.0f,
		  "If the texture memory pool load is above this the streaming manager stops loading mipmaps and starts freeing them")
ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxMipmapUploadsPerFrame, 32, 1, kMaxU32, "How many mips to upload per frame")
ANKI_CVAR(NumericCVar<U32>, Rsrc, FramesUntilEviction, 1 * 60, 1, 100 * 60, "An image will be evicted if not seen that many frames")

class StreamingImage; // An opaque type

// A system that handles some GPU memory required for streaming images
class StreamingImageResourceManager : public MakeSingleton<StreamingImageResourceManager>
{
	friend class StreamingImage;

public:
	StreamingImageResourceManager();

	~StreamingImageResourceManager();

	[[nodiscard]] BufferView getBuffer() const
	{
		return m_imageDescriptorsBuff;
	}

	// Thread-safe but should be called by the main thread
	void appendStreamingRequests(ConstWeakArray<ImageStreamingRequest> requests);

	// Thread-safe but should be called by the main thread
	void endFrame(Fence* fence);

	// NOT thread-safe. Can't be when loadNewImage() is called
	[[nodiscard]] U32 getMaxDescriptorIndex() const
	{
		return m_maxDescriptorIndex;
	}

	// Create a new streaming image
	// Thread-safe
	Error loadNewImage(ResourceFilename filename, U32 resourceUuid, Bool async, U32& arrayIndex, StreamingImage*& img);

	// Free a streaming image
	// Thread-safe
	void freeImage(StreamingImage* img);

	// Thread-safe
	[[nodiscard]] static Bool isImageLoaded(StreamingImage* img);

	// Can only be called if isImageLoaded() returned true
	// Thread-safe
	[[nodiscard]] static Texture& getImageTailChainTexture(StreamingImage* img);

	// Can only be called if isImageLoaded() returned true
	// Thread-safe
	[[nodiscard]] static Vec4 getImageAverageColor(StreamingImage* img);

private:
	class StreamingInfo;

	class Garbage
	{
	public:
		ResourceDynamicBitSet<U32> m_freedDecriptorMask;

		ResourceDynamicArray<TexturePtr> m_texturesToDelete;
		ResourceDynamicArray<TextureMemoryPoolAllocation> m_allocsToFree;

		FencePtr m_fence;
	};

	class LoadMipsRequest
	{
	public:
		IntrusiveNoDelPtr<StreamingImage> m_img;
		U32 m_arrayIndex;
		U8 m_firstMip;
		U8 m_mipCount;
	};

	TextureMemoryPoolAllocation m_imageDescriptorsBuff;

	ResourceBlockArray<StreamingInfo> m_streamingInfos;
	ResourceBlockArray<StreamingImage> m_imageData;

	ResourceDynamicArray<Garbage> m_garbage;

	ResourceDynamicBitSet<U64> m_pendingDeletionMask;

	ResourceDynamicBitSet<U64> m_uploadDescriptorMask;

	PtrSize m_texPoolAllocatedSizeOnPrevEviction = kMaxPtrSize;

	U32 m_framesSinceLastEviction = 0;

	U32 m_maxDescriptorIndex = 0;

	mutable Mutex m_mtx;

	U64 m_frame = 1;

	Bool isArrayIndexFree(U32 idx) const
	{
		return !m_streamingInfos.indexExists(idx);
	}

	Error loadTailMipChainAsync(StreamingImage& img, ImageLoader& loader);

	void loadOtherMips(ConstWeakArray<LoadMipsRequest> requests);
	Error loadOtherMipsAsync(ConstWeakArray<LoadMipsRequest> requests);

	void unloadOtherMips(ConstWeakArray<LoadMipsRequest> requests);
	Error unloadOtherMipsAsync(ConstWeakArray<LoadMipsRequest> requests);

	void release(U32 arrayIndex);

	void performStreaming(ResourceDynamicArray<IntrusiveNoDelPtr<StreamingImage>>& retainedImages);

	void collectGarbage(Bool forceFullCleanup);
};

} // end namespace anki
