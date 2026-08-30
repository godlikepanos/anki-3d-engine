// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/GpuMemory/TextureMemoryPool.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Shaders/ImageStreaming.h>
#include <AnKi/Util/DynamicBitSet.h>

namespace anki {

ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageSize2, kImageDescriptorMaxTextureSize, kImageDescriptorSmallestMipmapSize, kImageDescriptorMaxTextureSize,
		  "Max image size to load")
ANKI_CVAR(NumericCVar<U32>, Rsrc, MaxImageDescriptors, 5 * 1024, 32, kMaxU32, "The size of the ImageDescriptor structured buffer")

// XXX
class StreamingImageResourceManager : public MakeSingleton<StreamingImageResourceManager>
{
public:
	Error init();

	U32 newImageDescriptor();

	void freeImageDescriptor(U32 index);

	void uploadImageDescriptor(U32 index, const ImageDescriptor& desc);

private:
	GpuSceneBufferAllocation m_imageDescriptorsBuff; // Use the GPU scene for it's conventient upload functionality

	ResourceDynamicBitSet<U32> m_freeDecriptorMask;
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

private:
	class LoadingContext;
	class TexUploadTask;

	Array<TextureMemoryPoolAllocation, kImageDescriptorMaxBindlessTextures> m_texAllocations;
	Array<TexturePtr, kImageDescriptorMaxBindlessTextures> m_textures;

	U32 m_imageDescriptorIdx = kMaxU32;

	Vec4 m_avgColor = Vec4(0.0f);

	Error loadAsync(LoadingContext& ctx) const;
};

} // end namespace anki
