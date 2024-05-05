// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommandBufferFactory.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

MicroCommandBuffer::~MicroCommandBuffer()
{
	m_fastPool.destroy();

	safeRelease(m_cmdList);
	safeRelease(m_cmdAllocator);
}

Error MicroCommandBuffer::init(CommandBufferFlag flags)
{
	const D3D12_COMMAND_LIST_TYPE cmdListType =
		!!(flags & CommandBufferFlag::kGeneralWork) ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE;

	ANKI_D3D_CHECK(getDevice().CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&m_cmdAllocator)));

	ComPtr<ID3D12GraphicsCommandList> cmdList;
	ANKI_D3D_CHECK(getDevice().CreateCommandList(0, cmdListType, m_cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList)));
	ANKI_D3D_CHECK(cmdList->QueryInterface(IID_PPV_ARGS(&m_cmdList)));

	return Error::kNone;
}

void MicroCommandBuffer::reset()
{
	ANKI_TRACE_SCOPED_EVENT(D3DCommandBufferReset);

	ANKI_ASSERT(m_refcount.load() == 0);
	ANKI_ASSERT(!m_fence.isCreated());

	for(GrObjectType type : EnumIterable<GrObjectType>())
	{
		m_objectRefs[type].destroy();
	}

	m_fastPool.reset();

	// Command list should already be reset on submit

	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator, nullptr);

	m_isSmallBatch = true;
}

void MicroCommandBufferPtrDeleter::operator()(MicroCommandBuffer* cmdb)
{
	if(cmdb)
	{
		CommandBufferFactory::getSingleton().deleteCommandBuffer(cmdb);
	}
}

CommandBufferFactory::~CommandBufferFactory()
{
	for(U32 smallBatch = 0; smallBatch < 2; ++smallBatch)
	{
		for(GpuQueueType queue : EnumIterable<GpuQueueType>())
		{
			m_recyclers[smallBatch][queue].destroy();
		}
	}
}

void CommandBufferFactory::deleteCommandBuffer(MicroCommandBuffer* cmdb)
{
	ANKI_ASSERT(cmdb);

	const Bool smallBatch = cmdb->m_isSmallBatch;
	const GpuQueueType queue = (cmdb->m_cmdList->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT) ? GpuQueueType::kCompute : GpuQueueType::kCompute;

	m_recyclers[smallBatch][queue].recycle(cmdb);
}

Error CommandBufferFactory::newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr)
{
	const Bool smallBatch = !!(cmdbFlags & CommandBufferFlag::kSmallBatch);
	const GpuQueueType queue = !!(cmdbFlags & CommandBufferFlag::kGeneralWork) ? GpuQueueType::kCompute : GpuQueueType::kCompute;

	MicroCommandBuffer* cmdb = m_recyclers[smallBatch][queue].findToReuse();

	if(cmdb == nullptr)
	{
		cmdb = newInstance<MicroCommandBuffer>(GrMemoryPool::getSingleton());
		const Error err = cmdb->init(cmdbFlags);
		if(err)
		{
			deleteInstance(GrMemoryPool::getSingleton(), cmdb);
			cmdb = nullptr;
			return err;
		}
	}

	ptr.reset(cmdb);
	return Error::kNone;
}

} // end namespace anki
