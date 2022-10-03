// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/MemoryPool.h>

namespace anki {

/// @addtogroup core
/// @{

/// The HW counters.
/// @memberof MaliHwCounters
class MaliHwCountersOut
{
public:
	U64 m_gpuActive = 0;
	PtrSize m_readBandwidth = 0; ///< In bytes.
	PtrSize m_writeBandwidth = 0; ///< In bytes.
};

/// Sample HW counters for Mali GPUs.
class MaliHwCounters
{
public:
	MaliHwCounters(BaseMemoryPool* pool);

	MaliHwCounters(const MaliHwCounters&) = delete; // Non-copyable

	~MaliHwCounters();

	MaliHwCounters& operator=(const MaliHwCounters&) = delete; // Non-copyable

	void sample(MaliHwCountersOut& out);

private:
	BaseMemoryPool* m_pool = nullptr;
	void* m_impl = nullptr;
};
/// @}

} // end namespace anki
