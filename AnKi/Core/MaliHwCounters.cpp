// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/MaliHwCounters.h>

#define ANKI_HWCPIPE_ENABLE (ANKI_OS_ANDROID == 1)

#if ANKI_HWCPIPE_ENABLE
#	include <ThirdParty/HwcPipe/hwcpipe.h>
#endif

namespace anki {

MaliHwCounters::MaliHwCounters(BaseMemoryPool* pool)
	: m_pool(pool)
{
	ANKI_ASSERT(pool);
#if ANKI_HWCPIPE_ENABLE
	hwcpipe::HWCPipe* hwc = newInstance<hwcpipe::HWCPipe>(*m_pool);
	hwc->set_enabled_gpu_counters({hwcpipe::GpuCounter::GpuCycles, hwcpipe::GpuCounter::ExternalMemoryWriteBytes,
								   hwcpipe::GpuCounter::ExternalMemoryReadBytes});

	hwc->run();

	m_impl = hwc;
#else
	(void)m_impl; // Shut up the compiler
#endif
}

MaliHwCounters::~MaliHwCounters()
{
#if ANKI_HWCPIPE_ENABLE
	hwcpipe::HWCPipe* hwc = static_cast<hwcpipe::HWCPipe*>(m_impl);
	hwc->stop();
	deleteInstance(*m_pool, hwc);
	m_impl = nullptr;
#endif
}

void MaliHwCounters::sample(MaliHwCountersOut& out)
{
	out = {};

#if ANKI_HWCPIPE_ENABLE
	hwcpipe::HWCPipe* hwc = static_cast<hwcpipe::HWCPipe*>(m_impl);

	const hwcpipe::Measurements m = hwc->sample();

	if(m.gpu)
	{
		auto readCounter = [&](hwcpipe::GpuCounter counter) -> U64 {
			auto it = m.gpu->find(counter);
			ANKI_ASSERT(it != m.gpu->end());
			const hwcpipe::Value val = it->second;
			return val.get<U64>();
		};

		out.m_gpuActive = readCounter(hwcpipe::GpuCounter::GpuCycles);
		out.m_readBandwidth = readCounter(hwcpipe::GpuCounter::ExternalMemoryReadBytes);
		out.m_writeBandwidth = readCounter(hwcpipe::GpuCounter::ExternalMemoryWriteBytes);
	}
#endif
}

} // end namespace anki
