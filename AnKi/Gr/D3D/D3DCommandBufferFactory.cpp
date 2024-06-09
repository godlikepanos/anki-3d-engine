// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

IndirectCommandSignatureFactory::~IndirectCommandSignatureFactory()
{
	for(IndirectCommandSignatureType i : EnumIterable<IndirectCommandSignatureType>())
	{
		for(Signature& sig : m_arrays[i])
		{
			safeRelease(sig.m_d3dSignature);
		}
	}
}

Error IndirectCommandSignatureFactory::init()
{
	// Initialize some common strides
	for(IndirectCommandSignatureType i : EnumIterable<IndirectCommandSignatureType>())
	{
		ID3D12CommandSignature* sig;
		ANKI_CHECK(getOrCreateSignatureInternal(false, kAnkiToD3D[i], kCommonStrides[i], sig));
	}
	return Error::kNone;
}

Error IndirectCommandSignatureFactory::getOrCreateSignatureInternal(Bool takeFastPath, D3D12_INDIRECT_ARGUMENT_TYPE type, U32 stride,
																	ID3D12CommandSignature*& signature)
{
	signature = nullptr;

	IndirectCommandSignatureType akType = IndirectCommandSignatureType::kCount;
	switch(type)
	{
	case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
		akType = IndirectCommandSignatureType::kDraw;
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
		akType = IndirectCommandSignatureType::kDrawIndexed;
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH:
		akType = IndirectCommandSignatureType::kDispatch;
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH:
		akType = IndirectCommandSignatureType::kDispatchMesh;
		break;
	default:
		ANKI_ASSERT(!"Unsupported");
	}

	// Check if it's a common stride
	if(takeFastPath && stride == kCommonStrides[akType])
	{
		signature = m_arrays[akType][0].m_d3dSignature;
		return Error::kNone;
	}

	{
		RLockGuard lock(m_mutexes[akType]);

		for(const Signature& sig : m_arrays[akType])
		{
			if(sig.m_stride == stride)
			{
				signature = sig.m_d3dSignature;
				break;
			}
		}
	}

	if(signature == nullptr) [[unlikely]]
	{
		// Proactively create it without locking

		const D3D12_INDIRECT_ARGUMENT_DESC arg = {.Type = type};
		const D3D12_COMMAND_SIGNATURE_DESC desc = {.ByteStride = stride, .NumArgumentDescs = 1, .pArgumentDescs = &arg, .NodeMask = 0};

		ANKI_D3D_CHECK(getDevice().CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&signature)));

		// Try to do bookkeeping

		{
			WLockGuard lock(m_mutexes[akType]);

			ID3D12CommandSignature* oldSignature = nullptr;
			for(const Signature& sig : m_arrays[akType])
			{
				if(sig.m_stride == stride)
				{
					oldSignature = sig.m_d3dSignature;
					break;
				}
			}

			if(oldSignature)
			{
				// Someone else created the signature, remove what we proactively created
				safeRelease(signature);
				signature = oldSignature;
			}
			else
			{
				// New signature, do bookkeeping
				const Signature sig = {.m_d3dSignature = signature, .m_stride = stride};
				m_arrays[akType].emplaceBack(sig);
			}
		}
	}

	ANKI_ASSERT(signature);

	return Error::kNone;
}

} // end namespace anki
