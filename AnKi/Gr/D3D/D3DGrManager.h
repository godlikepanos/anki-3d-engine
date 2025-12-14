// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Gr/D3D/D3DSwapchainFactory.h>
#include <AnKi/Util/Tracer.h>

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

	/// @note It's thread-safe.
	void releaseObject(GrObject* object)
	{
		ANKI_ASSERT(object);
		LockGuard lock(m_globalMtx);
		m_frames[m_crntFrame].m_objectsMarkedForDeletion.emplaceBack(object);
	}

	void releaseObjectDeleteLoop(GrObject* object)
	{
		ANKI_ASSERT(object);
		m_frames[m_crntFrame].m_objectsMarkedForDeletion.emplaceBack(object);
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

		GpuQueueType m_queueWroteToSwapchainImage = GpuQueueType::kCount;

		GrDynamicArray<GrObject*> m_objectsMarkedForDeletion;
	};

	Array<PerFrame, kMaxFramesInFlight> m_frames;
	U8 m_crntFrame = 0;

	D3DCapabilities m_d3dCapabilities;

	BufferInternalPtr m_zeroBuffer; ///< Used in CommandBuffer::zeroBuffer

	U64 m_timestampFrequency = 0;

	Bool m_canInvokeDred = false;

	void destroy();

	TexturePtr acquireNextPresentableTextureInternal();

	void beginFrameInternal();

	void endFrameInternal();

	void submitInternal(WeakArray<CommandBuffer*> cmdbs, WeakArray<Fence*> waitFences, FencePtr* signalFence);

	void finishInternal();

	void deleteObjectsMarkedForDeletion()
	{
		ANKI_TRACE_FUNCTION();
		PerFrame& frame = m_frames[m_crntFrame];
		while(!frame.m_objectsMarkedForDeletion.isEmpty())
		{
			GrDynamicArray<GrObject*> objects = std::move(frame.m_objectsMarkedForDeletion);

			for(GrObject* obj : objects)
			{
				deleteInstance(GrMemoryPool::getSingleton(), obj);
			}
		}
	}
};
/// @}

} // end namespace anki
