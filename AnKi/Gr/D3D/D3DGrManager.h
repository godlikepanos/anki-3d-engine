// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Gr/D3D/D3DTexture.h>

namespace anki {

/// @addtogroup directx
/// @{

class Limits
{
public:
	U32 m_rtvDescriptorSize = 0;
};

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

	const Limits& getLimits() const
	{
		return m_limits;
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

	class
	{
	public:
		IDXGISwapChain3* m_swapchain = nullptr;
		Array<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxFramesInFlight> m_rtvHandles = {};
		Array<ID3D12Resource*, kMaxFramesInFlight> m_rtvResources = {};
		Array<TexturePtr, kMaxFramesInFlight> m_textures;
		U32 m_backbufferIdx = 0;
	} m_swapchain;

	class PerFrame
	{
	public:
		MicroFencePtr m_presentFence;

		GpuQueueType m_queueWroteToSwapchainImage = GpuQueueType::kCount;
	};

	Array<PerFrame, kMaxFramesInFlight> m_frames;
	U8 m_crntFrame = 0;

	Limits m_limits;

	void destroy();
};
/// @}

} // end namespace anki
