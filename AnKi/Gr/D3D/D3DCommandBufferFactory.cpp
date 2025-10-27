// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommandBufferFactory.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

ANKI_SVAR(CommandBufferCount, StatCategory::kGr, "CommandBufferCount", StatFlag::kNone)

MicroCommandBuffer::~MicroCommandBuffer()
{
	safeRelease(m_cmdList);
	safeRelease(m_cmdAllocator);

	g_svarCommandBufferCount.decrement(1);
}

Error MicroCommandBuffer::init(CommandBufferFlag flags)
{
	const D3D12_COMMAND_LIST_TYPE cmdListType =
		!!(flags & CommandBufferFlag::kGeneralWork) ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE;

	ANKI_D3D_CHECK(getDevice().CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&m_cmdAllocator)));

	ComPtr<ID3D12GraphicsCommandList> cmdList;
	ANKI_D3D_CHECK(getDevice().CreateCommandList(0, cmdListType, m_cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList)));
	ANKI_D3D_CHECK(cmdList->QueryInterface(IID_PPV_ARGS(&m_cmdList)));

	g_svarCommandBufferCount.increment(1);

	return Error::kNone;
}

void MicroCommandBuffer::releaseInternal()
{
	CommandBufferFactory::getSingleton().recycleCommandBuffer(this);
}

CommandBufferFactory::~CommandBufferFactory()
{
	for(GpuQueueType queue : EnumIterable<GpuQueueType>())
	{
		m_recyclers[queue].destroy();
	}
}

void CommandBufferFactory::recycleCommandBuffer(MicroCommandBuffer* cmdb)
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
	else
	{
		cmdb->reset();
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
		ANKI_CHECK(getOrCreateSignatureInternal(false, kAnkiToD3D[i], kCommonStrides[i], nullptr, sig));
	}
	return Error::kNone;
}

Error IndirectCommandSignatureFactory::getOrCreateSignatureInternal(Bool tryThePreCreatedRootSignaturesFirst, D3D12_INDIRECT_ARGUMENT_TYPE type,
																	U32 stride, RootSignature* rootSignature, ID3D12CommandSignature*& signature)
{
	signature = nullptr;

	Bool hasVertexShader = false;
	IndirectCommandSignatureType akType = IndirectCommandSignatureType::kCount;
	switch(type)
	{
	case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
		akType = IndirectCommandSignatureType::kDraw;
		hasVertexShader = true;
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
		akType = IndirectCommandSignatureType::kDrawIndexed;
		hasVertexShader = true;
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH:
		akType = IndirectCommandSignatureType::kDispatch;
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH:
		akType = IndirectCommandSignatureType::kDispatchMesh;
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS:
		akType = IndirectCommandSignatureType::kDispatchRays;
		break;
	default:
		ANKI_ASSERT(!"Unsupported");
	}

	if(tryThePreCreatedRootSignaturesFirst)
	{
		ANKI_ASSERT((!rootSignature || hasVertexShader)
					&& "If signature will be used with a vert shader we need the root signature to determin the DrawID");
	}
	else
	{
		ANKI_ASSERT(rootSignature == nullptr);
	}

	const Bool needsDrawId = rootSignature && rootSignature->getDrawIdRootParamIdx() != kMaxU32;
	ID3D12RootSignature* d3dRootSignature = needsDrawId ? &rootSignature->getD3DRootSignature() : nullptr;
	const U32 drawIdParameterIdx = needsDrawId ? rootSignature->getDrawIdRootParamIdx() : kMaxU32;

	// Check if it's one of the pre-created
	if(tryThePreCreatedRootSignaturesFirst && stride == kCommonStrides[akType] && d3dRootSignature == nullptr)
	{
		signature = m_arrays[akType][0].m_d3dSignature;
		return Error::kNone;
	}

	{
		RLockGuard lock(m_mutexes[akType]);

		for(const Signature& sig : m_arrays[akType])
		{
			if(sig.m_stride == stride && sig.m_d3dRootSignature == d3dRootSignature)
			{
				signature = sig.m_d3dSignature;
				break;
			}
		}
	}

	if(signature == nullptr) [[unlikely]]
	{
		// Proactively create it without locking

		Array<D3D12_INDIRECT_ARGUMENT_DESC, 2> args = {};
		args[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INCREMENTING_CONSTANT;
		args[0].IncrementingConstant.RootParameterIndex = drawIdParameterIdx;
		args[0].IncrementingConstant.DestOffsetIn32BitValues = 0;
		args[1].Type = type;
		const D3D12_COMMAND_SIGNATURE_DESC desc = {.ByteStride = stride,
												   .NumArgumentDescs = needsDrawId ? 2u : 1u,
												   .pArgumentDescs = args.getBegin() + (needsDrawId ? 0 : 1),
												   .NodeMask = 0};

		ANKI_D3D_CHECK(getDevice().CreateCommandSignature(&desc, d3dRootSignature, IID_PPV_ARGS(&signature)));

		// Try to do bookkeeping

		{
			WLockGuard lock(m_mutexes[akType]);

			ID3D12CommandSignature* oldSignature = nullptr;
			for(const Signature& sig : m_arrays[akType])
			{
				if(sig.m_stride == stride && sig.m_d3dRootSignature == d3dRootSignature)
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
				const Signature sig = {.m_d3dSignature = signature, .m_d3dRootSignature = d3dRootSignature, .m_stride = stride};
				m_arrays[akType].emplaceBack(sig);
			}
		}
	}

	ANKI_ASSERT(signature);

	return Error::kNone;
}

} // end namespace anki
