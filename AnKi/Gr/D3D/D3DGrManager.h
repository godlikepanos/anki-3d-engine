// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

class D3DCapabilities
{
public:
	Bool m_rebar = false;
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

	ID3D12DeviceX& getDevice()
	{
		return *m_device;
	}

	ID3D12CommandQueue& getCommandQueue(GpuQueueType q)
	{
		ANKI_ASSERT(m_queues[q]);
		return *m_queues[q];
	}

	const D3DCapabilities& getD3DCapabilities() const
	{
		return m_d3dCapabilities;
	}

	U64 getTimestampFrequency() const
	{
		ANKI_ASSERT(m_timestampFrequency);
		return m_timestampFrequency;
	}

	void invokeDred() const;

	Buffer& getZeroBuffer() const
	{
		return *m_zeroBuffer;
	}

private:
	ID3D12DeviceX* m_device = nullptr;
	Array<ID3D12CommandQueue*, U32(GpuQueueType::kCount)> m_queues = {};

	DWORD m_debugMessageCallbackCookie = 0;

	Mutex m_globalMtx;

	MicroSwapchainPtr m_crntSwapchain;

	class PerFrame
	{
	public:
		GrDynamicArray<MicroFencePtr> m_fences;
		MicroFencePtr m_presentFence;

		GpuQueueType m_queueWroteToSwapchainImage = GpuQueueType::kCount;
	};

	Array<PerFrame, kMaxFramesInFlight> m_frames;
	U8 m_crntFrame = 0;

	D3DCapabilities m_d3dCapabilities;

	BufferPtr m_zeroBuffer; ///< Used in CommandBuffer::zeroBuffer

	U64 m_timestampFrequency = 0;

	Bool m_canInvokeDred = false;

	void destroy();

	TexturePtr acquireNextPresentableTextureInternal();

	void beginFrameInternal();

	void endFrameInternal();

	void submitInternal(WeakArray<CommandBuffer*> cmdbs, WeakArray<Fence*> waitFences, FencePtr* signalFence);

	void finishInternal();
};
/// @}

} // end namespace anki
