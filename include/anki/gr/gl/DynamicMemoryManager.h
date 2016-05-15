// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/gr/common/GpuFrameRingAllocator.h>
#include <anki/gr/common/GpuBlockAllocator.h>

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

	void allocate(PtrSize size,
		BufferUsage usage,
		TransientMemoryTokenLifetime lifespan,
		TransientMemoryToken& token,
		void*& ptr,
		Error* outErr);

	void free(const TransientMemoryToken& token)
	{
		ANKI_ASSERT(
			token.m_lifetime == TransientMemoryTokenLifetime::PERSISTENT);
		m_persistentBuffers[token.m_usage].m_alloc.free(token.m_offset);
	}

	void* getBaseAddress(const TransientMemoryToken& token) const
	{
		void* addr;
		if(token.m_lifetime == TransientMemoryTokenLifetime::PER_FRAME)
		{
			addr = m_perFrameBuffers[token.m_usage].m_mappedMem;
		}
		else
		{
			addr = m_persistentBuffers[token.m_usage].m_mappedMem;
		}
		ANKI_ASSERT(addr);
		return addr;
	}

	GLuint getGlName(const TransientMemoryToken& token) const
	{
		GLuint name;
		if(token.m_lifetime == TransientMemoryTokenLifetime::PER_FRAME)
		{
			name = m_perFrameBuffers[token.m_usage].m_name;
		}
		else
		{
			name = m_persistentBuffers[token.m_usage].m_name;
		}
		ANKI_ASSERT(name);
		return name;
	}

private:
	class alignas(16) Aligned16Type
	{
		U8 _m_val[16];
	};

	// CPU or GPU buffer.
	class PerFrameBuffer
	{
	public:
		PtrSize m_size = 0;
		GLuint m_name = 0;
		DynamicArray<Aligned16Type> m_cpuBuff;
		U8* m_mappedMem = nullptr;
		GpuFrameRingAllocator m_alloc;
	};

	class PersistentBuffer
	{
	public:
		PtrSize m_size = 0;
		GLuint m_name = 0;
		U32 m_alignment = 0;
		DynamicArray<Aligned16Type> m_cpuBuff;
		U8* m_mappedMem = nullptr;
		GpuBlockAllocator m_alloc;
	};

	GenericMemoryPoolAllocator<U8> m_alloc;
	Array<PerFrameBuffer, U(BufferUsage::COUNT)> m_perFrameBuffers;
	Array<PersistentBuffer, U(BufferUsage::COUNT)> m_persistentBuffers;
};
/// @}

} // end namespace anki