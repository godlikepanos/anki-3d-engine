// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/gr/common/GpuFrameRingAllocator.h>
#include <anki/gr/common/GpuBlockAllocator.h>
#include <anki/gr/common/Misc.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup opengl
/// @{

/// Manages all dynamic memory.
class TransientMemoryManager : public NonCopyable
{
public:
	TransientMemoryManager()
	{
	}

	~TransientMemoryManager();

	void initMainThread(
		GenericMemoryPoolAllocator<U8> alloc, const ConfigSet& cfg);

	void initRenderThread();

	void destroyRenderThread();

	void endFrame();

	void allocate(PtrSize size,
		BufferUsageBit usage,
		TransientMemoryTokenLifetime lifespan,
		TransientMemoryToken& token,
		void*& ptr,
		Error* outErr);

	void free(const TransientMemoryToken& token)
	{
		ANKI_ASSERT(
			token.m_lifetime == TransientMemoryTokenLifetime::PERSISTENT);
	}

	void* getBaseAddress(const TransientMemoryToken& token) const
	{
		void* addr;
		if(token.m_lifetime == TransientMemoryTokenLifetime::PER_FRAME)
		{
			addr = m_perFrameBuffers[bufferUsageToTransient(token.m_usage)]
					   .m_mappedMem;
		}
		else
		{
			ANKI_ASSERT(0);
		}
		ANKI_ASSERT(addr);
		return addr;
	}

	GLuint getGlName(const TransientMemoryToken& token) const
	{
		GLuint name;
		if(token.m_lifetime == TransientMemoryTokenLifetime::PER_FRAME)
		{
			name =
				m_perFrameBuffers[bufferUsageToTransient(token.m_usage)].m_name;
		}
		else
		{
			ANKI_ASSERT(0);
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

	GenericMemoryPoolAllocator<U8> m_alloc;
	Array<PerFrameBuffer, U(TransientBufferType::COUNT)> m_perFrameBuffers;
};
/// @}

} // end namespace anki
