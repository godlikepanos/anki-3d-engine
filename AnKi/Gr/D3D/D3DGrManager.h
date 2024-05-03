// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Gr/D3D/D3DSwapchainFactory.h>

namespace anki {

/// @addtogroup directx
/// @{

/// DX implementation of GrManager.
class GrManagerImpl : public GrManager
{
	friend class GrManager;

public:
	GrManagerImpl()
	{
	}

	~GrManagerImpl();

	Error initInternal(const GrManagerInitInfo& cfg);

	ID3D12Device& getDevice()
	{
		return *m_device;
	}

	ID3D12CommandQueue& getCommandQueue(GpuQueueType q)
	{
		ANKI_ASSERT(m_queues[q]);
		return *m_queues[q];
	}

private:
	ID3D12Device* m_device = nullptr;
	Array<ID3D12CommandQueue*, U32(GpuQueueType::kCount)> m_queues = {};

	DWORD m_debugMessageCallbackCookie = 0;

	Mutex m_globalMtx;

	MicroSwapchainPtr m_crntSwapchain;

	class PerFrame
	{
	public:
		MicroFencePtr m_presentFence;

		GpuQueueType m_queueWroteToSwapchainImage = GpuQueueType::kCount;
	};

	Array<PerFrame, kMaxFramesInFlight> m_frames;
	U8 m_crntFrame = 0;

	void destroy();
};
/// @}

} // end namespace anki
