// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/gr/common/GpuFrameRingAllocator.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup opengl
/// @{

/// Manages all dynamic memory.
class DynamicMemoryManager : public NonCopyable
{
public:
	DynamicMemoryManager()
	{
	}

	~DynamicMemoryManager();

	void initMainThread(
		GenericMemoryPoolAllocator<U8> alloc, const ConfigSet& cfg);

	void initRenderThread();

	void destroyRenderThread();

	void endFrame();

	ANKI_USE_RESULT void* allocatePerFrame(
		BufferUsage usage, PtrSize size, DynamicBufferToken& handle);

	void* getBaseAddress(BufferUsage usage) const
	{
		void* addr = m_buffers[usage].m_mappedMem;
		ANKI_ASSERT(addr);
		return addr;
	}

	GLuint getGlName(BufferUsage usage) const
	{
		GLuint name = m_buffers[usage].m_name;
		ANKI_ASSERT(name);
		return name;
	}

private:
	class alignas(16) Aligned16Type
	{
		U8 _m_val[16];
	};

	// CPU or GPU buffer.
	class DynamicBuffer
	{
	public:
		PtrSize m_size = 0;
		GLuint m_name = 0;
		DynamicArray<Aligned16Type> m_cpuBuff;
		U8* m_mappedMem = nullptr;
		GpuFrameRingAllocator m_frameAlloc;
	};

	GenericMemoryPoolAllocator<U8> m_alloc;
	Array<DynamicBuffer, U(BufferUsage::COUNT)> m_buffers;
};
/// @}

} // end namespace anki