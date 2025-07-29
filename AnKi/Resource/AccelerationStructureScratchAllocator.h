// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

/// @addtogroup resource
/// @{

/// Ring buffer used for AS scratch memory.
class AccelerationStructureScratchAllocator
{
public:
	static constexpr PtrSize kBufferSize = 64_MB;
	static constexpr PtrSize kMaxBufferSize = 256_MB;

	AccelerationStructureScratchAllocator()
	{
		BufferInitInfo buffInit("BLAS scratch");
		buffInit.m_size = kBufferSize;
		buffInit.m_usage = BufferUsageBit::kAccelerationStructureBuildScratch;
		m_buffer = GrManager::getSingleton().newBuffer(buffInit);
	}

	BufferView allocate(PtrSize size, Bool& addBarrierBefore)
	{
		if(size > kMaxBufferSize)
		{
			ANKI_RESOURCE_LOGF("Asked for too much BLAS scratch memory: %zu", size);
		}

		if(size > m_buffer->getSize())
		{
			BufferInitInfo buffInit("BLAS scratch");
			buffInit.m_size = min(kMaxBufferSize, m_buffer->getSize() * 2u);
			buffInit.m_usage = BufferUsageBit::kAccelerationStructureBuildScratch;
			m_buffer = GrManager::getSingleton().newBuffer(buffInit);

			m_offset = 0;
		}

		if(m_offset + size > m_buffer->getSize())
		{
			m_offset = 0;
			addBarrierBefore = true;
		}
		else
		{
			addBarrierBefore = false;
		}

		const BufferView view(m_buffer.get(), m_offset, size);
		m_offset += size;

		return view;
	}

private:
	BufferPtr m_buffer;
	PtrSize m_offset = 0;
};
/// @}

} // end namespace anki
