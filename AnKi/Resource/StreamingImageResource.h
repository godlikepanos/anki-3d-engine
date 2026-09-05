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

ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageSize2, kImageDescriptorMaxTextureSize, kImageDescriptorSmallestMipmapSize, kImageDescriptorMaxTextureSize,
		  "Max image size to load")
ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageDescriptors, 5 * 1024, 32, kMaxU32, "The size of the ImageDescriptor structured buffer")

using ImageDescriptorHandle = U32;

// A system that handles some GPU memory required for streaming images
class StreamingImageResourceManager : public MakeSingleton<StreamingImageResourceManager>
{
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

	ImageDescriptorHandle newImageDescriptor();

	void freeImageDescriptor(ImageDescriptorHandle index);

	void uploadImageDescriptor(ImageDescriptorHandle index, const ImageDescriptor& desc) const;

	void endFrame(Fence* fence);

private:
	class Garbage
	{
	public:
		ResourceDynamicBitSet<U32> m_freedDecriptorMask;
		FencePtr m_fence;
	};

	TextureMemoryPoolAllocation m_imageDescriptorsBuff;

	ResourceDynamicBitSet<U32> m_freeDescriptorMask;

	ResourceDynamicArray<Garbage> m_garbage;

	mutable Mutex m_mtx;

	void init();

	void collectGarbage(Bool waitForFences);
};

// Image resource class. It loads or creates an image and then loads it in the GPU. It supports compressed and uncompressed TGAs, PNGs, JPEG and
// AnKi's image format.
class StreamingImageResource : public ResourceObject
{
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

	Bool isLoaded() const
	{
		return m_isLoaded.load() == 1;
	}

	U32 getImageDescriptorIndex() const
	{
		ANKI_ASSERT(m_imageDescHandle < kMaxU32);
		return m_imageDescHandle;
	}

	Texture& getTexture(U32 mipmap) const;

private:
	class LoadingContext;
	class TexUploadTask;

	Vec4 m_avgColor = Vec4(0.0f);

	Array<TextureMemoryPoolAllocation, kImageDescriptorMaxBindlessTextures> m_texAllocations;
	Array<TexturePtr, kImageDescriptorMaxBindlessTextures> m_textures;

	ImageDescriptorHandle m_imageDescHandle = kMaxU32;

	mutable Atomic<U32> m_isLoaded = {0};

	U32 m_textureCount = 0;

	Error loadAsync(LoadingContext& ctx) const;
};

} // end namespace anki
