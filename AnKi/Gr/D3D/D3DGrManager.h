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
#include <AnKi/Util/BlockArray.h>

namespace anki {

class D3DCapabilities
{
public:
	Bool m_rebar = false;
};

// DX implementation of GrManager.
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

	// Node: It's thread-safe.
	void releaseObject(GrObject* object)
	{
		ANKI_ASSERT(object);
		LockGuard lock(m_globalMtx);
		releaseObjectDeleteLoop(object);
	}

	void releaseObjectDeleteLoop(GrObject* object);

private:
	// Contains objects that are marked for deletion
	class CleanupGroup
	{
	public:
		Array<MicroFencePtr, U32(GpuQueueType::kCount)> m_fences;
		GrDynamicArray<GrObject*> m_objectsMarkedForDeletion;
		Bool m_finalized = false;

		~CleanupGroup()
		{
			ANKI_ASSERT(m_objectsMarkedForDeletion.getSize() == 0);
		}

		Bool canDelete() const
		{
			if(m_fences[GpuQueueType::kGeneral] && !m_fences[GpuQueueType::kGeneral]->signaled())
			{
				return false;
			}

			if(m_fences[GpuQueueType::kCompute] && !m_fences[GpuQueueType::kCompute]->signaled())
			{
				return false;
			}

			return true;
		}
	};

	GrBlockArray<CleanupGroup> m_cleanupGroups;
	U32 m_activeCleanupGroup = kMaxU32;

	Array<MicroFencePtr, U32(GpuQueueType::kCount) + 1> m_crntFences;

	MicroSwapchainPtr m_crntSwapchain;

	ID3D12DeviceX* m_device = nullptr;
	Array<ID3D12CommandQueue*, U32(GpuQueueType::kCount)> m_queues = {};

	DWORD m_debugMessageCallbackCookie = 0;

	Mutex m_globalMtx;

	D3DCapabilities m_d3dCapabilities;

	BufferInternalPtr m_zeroBuffer; // Used in CommandBuffer::zeroBuffer

	U64 m_timestampFrequency = 0;

	Bool m_canInvokeDred = false;

	void destroy();

	TexturePtr acquireNextPresentableTextureInternal();

	void beginFrameInternal();

	void endFrameInternal();

	void submitInternal(WeakArray<CommandBuffer*> cmdbs, WeakArray<Fence*> waitFences, FencePtr* signalFence);

	void finishInternal();

	void deleteObjectsMarkedForDeletion();
};

} // end namespace anki
