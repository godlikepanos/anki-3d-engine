// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/GrManager.h>

namespace anki
{

//==============================================================================
inline void CommandBuffer::uploadTextureSurfaceData(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	void*& data,
	PtrSize& dataSize)
{
	PtrSize allocationSize;
	BufferUsageBit usage;
	getManager().getTextureUploadInfo(tex, surf, allocationSize, usage);
	ANKI_ASSERT(dataSize <= allocationSize);

	TransientMemoryToken token;
	data =
		getManager().allocateFrameTransientMemory(allocationSize, usage, token);

	uploadTextureSurface(tex, surf, token);
}

//==============================================================================
inline void CommandBuffer::tryUploadTextureSurfaceData(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	void*& data,
	PtrSize& dataSize)
{
	PtrSize allocationSize;
	BufferUsageBit usage;
	getManager().getTextureUploadInfo(tex, surf, allocationSize, usage);
	ANKI_ASSERT(dataSize <= allocationSize);

	TransientMemoryToken token;
	data = getManager().tryAllocateFrameTransientMemory(
		allocationSize, usage, token);

	if(data)
	{
		uploadTextureSurface(tex, surf, token);
	}
}

//==============================================================================
inline void CommandBuffer::uploadTextureSurfaceCopyData(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	void* data,
	PtrSize dataSize)
{
	PtrSize allocationSize;
	BufferUsageBit usage;
	getManager().getTextureUploadInfo(tex, surf, allocationSize, usage);
	ANKI_ASSERT(dataSize <= allocationSize);

	TransientMemoryToken token;
	void* ptr =
		getManager().allocateFrameTransientMemory(allocationSize, usage, token);
	memcpy(ptr, data, dataSize);

	uploadTextureSurface(tex, surf, token);
}

} // end namespace anki
