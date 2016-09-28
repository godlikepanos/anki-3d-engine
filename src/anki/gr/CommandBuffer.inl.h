// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/GrManager.h>

namespace anki
{

inline void CommandBuffer::uploadTextureSurfaceData(
	TexturePtr tex, const TextureSurfaceInfo& surf, void*& data, PtrSize& dataSize, DepthStencilAspectMask aspect)
{
	PtrSize allocationSize;
	const BufferUsageBit usage = BufferUsageBit::TEXTURE_UPLOAD_SOURCE;
	getManager().getTextureSurfaceUploadInfo(tex, surf, allocationSize);
	ANKI_ASSERT(dataSize <= allocationSize);

	TransientMemoryToken token;
	data = getManager().allocateFrameTransientMemory(allocationSize, usage, token);

	uploadTextureSurface(tex, surf, token, aspect);
}

inline void CommandBuffer::tryUploadTextureSurfaceData(
	TexturePtr tex, const TextureSurfaceInfo& surf, void*& data, PtrSize& dataSize, DepthStencilAspectMask aspect)
{
	PtrSize allocationSize;
	const BufferUsageBit usage = BufferUsageBit::TEXTURE_UPLOAD_SOURCE;
	getManager().getTextureSurfaceUploadInfo(tex, surf, allocationSize);
	ANKI_ASSERT(dataSize <= allocationSize);

	TransientMemoryToken token;
	data = getManager().tryAllocateFrameTransientMemory(allocationSize, usage, token);

	if(data)
	{
		uploadTextureSurface(tex, surf, token, aspect);
	}
}

inline void CommandBuffer::uploadTextureSurfaceCopyData(
	TexturePtr tex, const TextureSurfaceInfo& surf, void* data, PtrSize dataSize, DepthStencilAspectMask aspect)
{
	PtrSize allocationSize;
	const BufferUsageBit usage = BufferUsageBit::TEXTURE_UPLOAD_SOURCE;
	getManager().getTextureSurfaceUploadInfo(tex, surf, allocationSize);
	ANKI_ASSERT(dataSize <= allocationSize);

	TransientMemoryToken token;
	void* ptr = getManager().allocateFrameTransientMemory(allocationSize, usage, token);
	memcpy(ptr, data, dataSize);

	uploadTextureSurface(tex, surf, token, aspect);
}

inline void CommandBuffer::uploadTextureVolumeCopyData(
	TexturePtr tex, const TextureVolumeInfo& vol, void* data, PtrSize dataSize, DepthStencilAspectMask aspect)
{
	PtrSize allocationSize;
	const BufferUsageBit usage = BufferUsageBit::TEXTURE_UPLOAD_SOURCE;
	getManager().getTextureVolumeUploadInfo(tex, vol, allocationSize);
	ANKI_ASSERT(dataSize <= allocationSize);

	TransientMemoryToken token;
	void* ptr = getManager().allocateFrameTransientMemory(allocationSize, usage, token);
	memcpy(ptr, data, dataSize);

	uploadTextureVolume(tex, vol, token, aspect);
}

} // end namespace anki
