// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/common/GpuFrameRingAllocator.h>
#include <anki/gr/common/Misc.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup vulkan
/// @{

/// Manages the transient memory.
class TransientMemoryManager
{
public:
	TransientMemoryManager(GrManager* gr)
		: m_manager(gr)
	{
		ANKI_ASSERT(gr);
	}

	~TransientMemoryManager()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void destroy();

	/// @note Not thread-safe.
	void endFrame();

	void allocate(PtrSize size, BufferUsageBit usage, TransientMemoryToken& token, void*& ptr, Error* outErr);

	void* getBaseAddress(const TransientMemoryToken& token) const
	{
		ANKI_ASSERT(token.m_lifetime == TransientMemoryTokenLifetime::PER_FRAME);
		const PerFrameBuffer& frame = m_perFrameBuffers[bufferUsageToTransient(token.m_usage)];
		void* addr = frame.m_mappedMem;
		ANKI_ASSERT(addr);
		return addr;
	}

	VkBuffer getBufferHandle(BufferUsageBit usage) const
	{
		const PerFrameBuffer& frame = m_perFrameBuffers[bufferUsageToTransient(usage)];
		ANKI_ASSERT(frame.m_bufferHandle);
		return frame.m_bufferHandle;
	}

private:
	class PerFrameBuffer
	{
	public:
		PtrSize m_size = 0;
		BufferImpl* m_buff = nullptr;
		U8* m_mappedMem = nullptr;
		VkBuffer m_bufferHandle = VK_NULL_HANDLE; ///< Cache it.
		GpuFrameRingAllocator m_alloc;
	};

	GrManager* m_manager = nullptr;

	Array<PerFrameBuffer, U(TransientBufferType::COUNT)> m_perFrameBuffers;
};
/// @}

} // end namespace anki
