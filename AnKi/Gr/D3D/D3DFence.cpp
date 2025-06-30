// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

namespace anki {

void MicroFenceImpl::gpuSignal(GpuQueueType queue)
{
	ANKI_D3D_CHECKF(getGrManagerImpl().getCommandQueue(queue).Signal(m_fence, m_value.fetchAdd(1) + 1));
}

void MicroFenceImpl::gpuWait(GpuQueueType queue)
{
	ANKI_D3D_CHECKF(getGrManagerImpl().getCommandQueue(queue).Wait(m_fence, m_value.load()));
}

Fence* Fence::newInstance()
{
	return anki::newInstance<FenceImpl>(GrMemoryPool::getSingleton(), "N/A");
}

Bool Fence::clientWait(Second seconds)
{
	ANKI_D3D_SELF(FenceImpl);
	return self.m_fence->clientWait(seconds);
}

} // end namespace anki
