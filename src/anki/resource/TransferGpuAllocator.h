// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/gr/common/ClassGpuAllocator.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Memory handle.
class TransferGpuAllocatorHandle
{
public:
	TransferGpuAllocatorHandle() = default;

	TransferGpuAllocatorHandle(const TransferGpuAllocatorHandle&) = delete;

	TransferGpuAllocatorHandle(TransferGpuAllocatorHandle&& b)
	{
		*this = std::move(b);
	}

	~TransferGpuAllocatorHandle();

	TransferGpuAllocatorHandle& operator=(TransferGpuAllocatorHandle&& b)
	{
		m_handle = b.m_handle;
		b.m_handle = {};
		return *this;
	}

	BufferPtr getBuffer() const;

	PtrSize getOffset() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle.m_offset;
	}

	PtrSize getRange() const
	{
		ANKI_ASSERT(m_handle);
		ANKI_ASSERT(m_range != 0);
		return m_range;
	}

private:
	ClassGpuAllocatorHandle m_handle;
	PtrSize m_range = 0;
};

/// GPU memory allocator for GPU buffers used in transfer operations.
class TransferGpuAllocator
{
public:
	class ClassInf;

	ANKI_USE_RESULT Bool allocate(PtrSize size, TransferGpuAllocatorHandle& handle);

	void release(TransferGpuAllocatorHandle& handle, FencePtr fence);

private:
	class Memory;
	class Interface;

	ResourceAllocator<U8> m_alloc;
	GrManager* m_gr = nullptr;

	ClassGpuAllocatorInterface* m_interface = nullptr;
	ClassGpuAllocator m_classAlloc;
};
/// @}

} // end namespace anki
