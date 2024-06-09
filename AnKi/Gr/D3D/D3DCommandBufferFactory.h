// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
#include <AnKi/Util/List.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Recycled object.
class MicroCommandBuffer
{
	friend class CommandBufferFactory;

public:
	MicroCommandBuffer()
	{
		m_fastPool.init(GrMemoryPool::getSingleton().getAllocationCallback(), GrMemoryPool::getSingleton().getAllocationCallbackUserData(), 256_KB,
						2.0f);

		for(DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>& arr : m_objectRefs)
		{
			arr = DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>(&m_fastPool);
		}
	}

	~MicroCommandBuffer();

	Error init(CommandBufferFlag flags);

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	/// Interface method.
	void onFenceDone()
	{
		reset();
	}

	void setFence(MicroFence* fence)
	{
		m_fence.reset(fence);
	}

	/// Interface method.
	MicroFence* getFence() const
	{
		return m_fence.tryGet();
	}

	template<typename T>
	void pushObjectRef(T* x)
	{
		ANKI_ASSERT(T::kClassType != GrObjectType::kTexture && T::kClassType != GrObjectType::kBuffer
					&& "No need to push references of buffers and textures");
		pushToArray(m_objectRefs[T::kClassType], x);
	}

	D3D12GraphicsCommandListX& getCmdList() const
	{
		ANKI_ASSERT(m_cmdList);
		return *m_cmdList;
	}

	StackMemoryPool& getFastMemoryPool()
	{
		return m_fastPool;
	}

	GpuQueueType getQueueType() const
	{
		return (m_cmdList->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE) ? GpuQueueType::kCompute : GpuQueueType::kGeneral;
	}

private:
	static constexpr U32 kMaxRefObjectSearch = 16;

	mutable Atomic<I32> m_refcount = {0};

	MicroFencePtr m_fence;
	Array<DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>, U(GrObjectType::kCount)> m_objectRefs;

	StackMemoryPool m_fastPool;

	ID3D12CommandAllocator* m_cmdAllocator = nullptr;
	D3D12GraphicsCommandListX* m_cmdList = nullptr;

	void reset();

	static void pushToArray(DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>& arr, GrObject* grobj)
	{
		ANKI_ASSERT(grobj);

		// Search the temp cache to avoid setting the ref again
		if(arr.getSize() >= kMaxRefObjectSearch)
		{
			for(U32 i = arr.getSize() - kMaxRefObjectSearch; i < arr.getSize(); ++i)
			{
				if(arr[i].get() == grobj)
				{
					return;
				}
			}
		}

		// Not found in the temp cache, add it
		arr.emplaceBack(grobj);
	}
};

/// Deleter.
class MicroCommandBufferPtrDeleter
{
public:
	void operator()(MicroCommandBuffer* cmdb);
};

/// Micro command buffer pointer.
using MicroCommandBufferPtr = IntrusivePtr<MicroCommandBuffer, MicroCommandBufferPtrDeleter>;

/// Command bufffer object recycler.
class CommandBufferFactory : public MakeSingleton<CommandBufferFactory>
{
	friend class MicroCommandBufferPtrDeleter;

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

	void deleteCommandBuffer(MicroCommandBuffer* cmdb);
};

/// Creates command signatures.
class IndirectCommandSignatureFactory : public MakeSingleton<IndirectCommandSignatureFactory>
{
public:
	~IndirectCommandSignatureFactory();

	Error init();

	/// @note Thread-safe.
	Error getOrCreateSignature(D3D12_INDIRECT_ARGUMENT_TYPE type, U32 stride, ID3D12CommandSignature*& signature)
	{
		return getOrCreateSignatureInternal(true, type, stride, signature);
	}

private:
	enum class IndirectCommandSignatureType
	{
		kDraw,
		kDrawIndexed,
		kDispatch,
		kDispatchMesh,

		kCount,
		kFirst = 0
	};

	class Signature
	{
	public:
		ID3D12CommandSignature* m_d3dSignature;
		U32 m_stride;
	};

	static constexpr Array<U32, U32(IndirectCommandSignatureType::kCount)> kCommonStrides = {
		sizeof(DrawIndirectArgs), sizeof(DrawIndexedIndirectArgs), sizeof(DispatchIndirectArgs), sizeof(DispatchIndirectArgs)};

	static constexpr Array<D3D12_INDIRECT_ARGUMENT_TYPE, U32(IndirectCommandSignatureType::kCount)> kAnkiToD3D = {
		D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH,
		D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH};

	Array<GrDynamicArray<Signature>, U32(IndirectCommandSignatureType::kCount)> m_arrays;

	Array<RWMutex, U32(IndirectCommandSignatureType::kCount)> m_mutexes;

	Error getOrCreateSignatureInternal(Bool takeFastPath, D3D12_INDIRECT_ARGUMENT_TYPE type, U32 stride, ID3D12CommandSignature*& signature);
};
/// @}

} // end namespace anki
