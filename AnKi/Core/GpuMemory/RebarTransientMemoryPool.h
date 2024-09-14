// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

/// @addtogroup core
/// @{

inline NumericCVar<PtrSize> g_rebarGpuMemorySizeCvar("Core", "RebarGpuMemorySize", 24_MB, 1_MB, 1_GB, "ReBAR: always mapped GPU memory");

/// Manages staging GPU memory.
class RebarTransientMemoryPool : public MakeSingleton<RebarTransientMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	RebarTransientMemoryPool(const RebarTransientMemoryPool&) = delete; // Non-copyable

	RebarTransientMemoryPool& operator=(const RebarTransientMemoryPool&) = delete; // Non-copyable

	void init();

	void endFrame();

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the N-(kMaxFramesInFlight-1) frame.
	template<typename T>
	BufferView allocate(PtrSize size, U32 alignment, T*& mappedMem)
	{
		void* mem;
		const BufferView out = allocateInternal(size, alignment, mem);
		mappedMem = static_cast<T*>(mem);
		return out;
	}

	/// @copydoc allocate
	template<typename T>
	BufferView allocateConstantBuffer(T*& mappedMem)
	{
		return allocate(sizeof(T), GrManager::getSingleton().getDeviceCapabilities().m_constantBufferBindOffsetAlignment, mappedMem);
	}

	/// @copydoc allocate
	template<typename T>
	BufferView allocateStructuredBuffer(U32 count, WeakArray<T>& arr)
	{
		T* mem;
		const U32 alignment = (m_structuredBufferAlignment == kMaxU32) ? sizeof(T) : m_structuredBufferAlignment;
		const BufferView out = allocate(count * sizeof(T), alignment, mem);
		arr = {mem, count};
		return out;
	}

	/// @copydoc allocate
	template<typename T>
	BufferView allocateCopyBuffer(U32 count, WeakArray<T>& arr)
	{
		T* mem;
		const U32 alignment = sizeof(U32);
		const BufferView out = allocate(sizeof(T) * count, alignment, mem);
		arr = {mem, count};
		return out;
	}

	ANKI_PURE Buffer& getBuffer() const
	{
		return *m_buffer;
	}

	U8* getBufferMappedAddress()
	{
		return m_mappedMem;
	}

private:
	BufferPtr m_buffer;
	U8* m_mappedMem = nullptr; ///< Cache it.
	PtrSize m_bufferSize = 0; ///< Cache it.
	Atomic<PtrSize> m_offset = {0};
	PtrSize m_previousFrameEndOffset = 0;
	U32 m_structuredBufferAlignment = kMaxU32;

	RebarTransientMemoryPool() = default;

	~RebarTransientMemoryPool();

	BufferView allocateInternal(PtrSize size, U32 alignment, void*& mappedMem);
};
/// @}

} // end namespace anki
