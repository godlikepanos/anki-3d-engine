// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
#include <AnKi/Util/List.h>

namespace anki {

class RootSignature;

/// @addtogroup directx
/// @{

/// Recycled object.
class MicroCommandBuffer
{
	friend class CommandBufferFactory;

public:
	MicroCommandBuffer() = default;

	~MicroCommandBuffer();

	Error init(CommandBufferFlag flags);

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	void release()
	{
		if(m_refcount.fetchSub(1) == 1)
		{
			releaseInternal();
		}
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	D3D12GraphicsCommandListX& getCmdList() const
	{
		ANKI_ASSERT(m_cmdList);
		return *m_cmdList;
	}

	GpuQueueType getQueueType() const
	{
		return (m_cmdList->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE) ? GpuQueueType::kCompute : GpuQueueType::kGeneral;
	}

	/// Reset for reuse.
	void reset()
	{
		m_cmdAllocator->Reset();
		m_cmdList->Reset(m_cmdAllocator, nullptr);
	}

private:
	mutable Atomic<I32> m_refcount = {0};

	ID3D12CommandAllocator* m_cmdAllocator = nullptr;
	D3D12GraphicsCommandListX* m_cmdList = nullptr;

	void releaseInternal();
};

/// Micro command buffer pointer.
using MicroCommandBufferPtr = IntrusiveNoDelPtr<MicroCommandBuffer>;

/// Command bufffer object recycler.
class CommandBufferFactory : public MakeSingleton<CommandBufferFactory>
{
	friend class MicroCommandBuffer;

public:
	CommandBufferFactory()
	{
	}

	CommandBufferFactory(const CommandBufferFactory&) = delete; // Non-copyable

	~CommandBufferFactory();

	CommandBufferFactory& operator=(const CommandBufferFactory&) = delete; // Non-copyable

	/// Request a new command buffer.
	/// @note Thread-safe.
	Error newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr);

private:
	Array<MicroObjectRecycler<MicroCommandBuffer>, U(GpuQueueType::kCount)> m_recyclers;

	void recycleCommandBuffer(MicroCommandBuffer* cmdb);
};

/// Creates command signatures.
class IndirectCommandSignatureFactory : public MakeSingleton<IndirectCommandSignatureFactory>
{
public:
	~IndirectCommandSignatureFactory();

	Error init();

	// Thread-safe
	// rootSignature: Only useful for drawcalls with vertex shader (needed for the DrawID)
	Error getOrCreateSignature(D3D12_INDIRECT_ARGUMENT_TYPE type, U32 stride, RootSignature* rootSignature, ID3D12CommandSignature*& signature)
	{
		return getOrCreateSignatureInternal(true, type, stride, rootSignature, signature);
	}

private:
	enum class IndirectCommandSignatureType
	{
		kDraw,
		kDrawIndexed,
		kDispatch,
		kDispatchMesh,
		kDispatchRays,

		kCount,
		kFirst = 0
	};

	class Signature
	{
	public:
		ID3D12CommandSignature* m_d3dSignature;
		ID3D12RootSignature* m_d3dRootSignature;
		U32 m_stride;
	};

	static constexpr Array<U32, U32(IndirectCommandSignatureType::kCount)> kCommonStrides = {
		sizeof(DrawIndirectArgs), sizeof(DrawIndexedIndirectArgs), sizeof(DispatchIndirectArgs), sizeof(DispatchIndirectArgs),
		sizeof(D3D12_DISPATCH_RAYS_DESC)};

	static constexpr Array<D3D12_INDIRECT_ARGUMENT_TYPE, U32(IndirectCommandSignatureType::kCount)> kAnkiToD3D = {
		D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH,
		D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH, D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS};

	Array<GrDynamicArray<Signature>, U32(IndirectCommandSignatureType::kCount)> m_arrays;

	Array<RWMutex, U32(IndirectCommandSignatureType::kCount)> m_mutexes;

	Error getOrCreateSignatureInternal(Bool tryThePreCreatedRootSignaturesFirst, D3D12_INDIRECT_ARGUMENT_TYPE type, U32 stride,
									   RootSignature* rootSignature, ID3D12CommandSignature*& signature);
};
/// @}

} // end namespace anki
