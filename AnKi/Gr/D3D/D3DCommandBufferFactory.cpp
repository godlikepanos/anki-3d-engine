// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommandBufferFactory.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

static StatCounter g_commandBufferCountStatVar(StatCategory::kMisc, "CommandBufferCount", StatFlag::kNone);

MicroCommandBuffer::~MicroCommandBuffer()
{
	m_fastPool.destroy();

	safeRelease(m_cmdList);
	safeRelease(m_cmdAllocator);

	g_commandBufferCountStatVar.decrement(1);
}

Error MicroCommandBuffer::init(CommandBufferFlag flags)
{
	const D3D12_COMMAND_LIST_TYPE cmdListType =
		!!(flags & CommandBufferFlag::kGeneralWork) ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE;

	ANKI_D3D_CHECK(getDevice().CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&m_cmdAllocator)));

	ComPtr<ID3D12GraphicsCommandList> cmdList;
	ANKI_D3D_CHECK(getDevice().CreateCommandList(0, cmdListType, m_cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList)));
	ANKI_D3D_CHECK(cmdList->QueryInterface(IID_PPV_ARGS(&m_cmdList)));

	g_commandBufferCountStatVar.increment(1);

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
	for(GpuQueueType queue : EnumIterable<GpuQueueType>())
	{
		m_recyclers[queue].destroy();
	}
}

void CommandBufferFactory::deleteCommandBuffer(MicroCommandBuffer* cmdb)
{
	ANKI_ASSERT(cmdb);

	const GpuQueueType queue = (cmdb->m_cmdList->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT) ? GpuQueueType::kGeneral : GpuQueueType::kCompute;

	m_recyclers[queue].recycle(cmdb);
}

Error CommandBufferFactory::newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr)
{
	const GpuQueueType queue = !!(cmdbFlags & CommandBufferFlag::kGeneralWork) ? GpuQueueType::kGeneral : GpuQueueType::kCompute;

	MicroCommandBuffer* cmdb = m_recyclers[queue].findToReuse();

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
