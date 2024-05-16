// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DBuffer.h>

namespace anki {

/// @addtogroup directx
/// @{

class DescriptorHeapHandle
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {};

	[[nodiscard]] Bool isCreated() const
	{
		return m_cpuHandle.ptr != 0 || m_gpuHandle.ptr != 0;
	}
};

/// Descriptor allocator with persistent allocations.
class PersistentDescriptorAllocator
{
public:
	PersistentDescriptorAllocator() = default;

	~PersistentDescriptorAllocator()
	{
		ANKI_ASSERT(m_freeDescriptorsHead == 0 && "Forgot to free");
	}

	void init(D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart, U32 descriptorSize, U16 descriptorCount);

	/// @note Thread-safe.
	DescriptorHeapHandle allocate();

	/// @note Thread-safe.
	void free(DescriptorHeapHandle& handle);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHeapStart = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHeapStart = {};

	U32 m_descriptorSize = 0;

	GrDynamicArray<U16> m_freeDescriptors;
	U16 m_freeDescriptorsHead = 0;

	Mutex m_mtx;
};

/// Ring-buffer-like descriptor allocator.
class RingDescriptorAllocator
{
public:
	void init(D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart, U32 descriptorSize, U32 descriptorCount);

	/// Allocate for this frame. Memory will be reclaimed a few frames in the future.
	/// @note Thread-safe.
	DescriptorHeapHandle allocate(U32 descriptorCount);

	/// @note Thread-safe with allocate().
	void endFrame();

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHeapStart = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHeapStart = {};

	U32 m_descriptorSize = 0;
	U32 m_descriptorCount = 0;

	Atomic<U64> m_increment = 0;

	U64 m_incrementAtFrameStart = 0;
};

/// A container of all descriptor heaps.
class DescriptorFactory : public MakeSingleton<DescriptorFactory>
{
public:
	~DescriptorFactory();

	Error init();

	/// @note Thread-safe.
	DescriptorHeapHandle allocateCpuVisiblePersistent(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		return getCpuPersistentAllocator(type).allocate();
	}

	/// @note Thread-safe.
	void free(D3D12_DESCRIPTOR_HEAP_TYPE type, DescriptorHeapHandle& handle)
	{
		getCpuPersistentAllocator(type).free(handle);
	}

	/// @note Thread-safe.
	DescriptorHeapHandle allocateCpuGpuVisibleTransient(D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorCount)
	{
		ANKI_ASSERT(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		RingDescriptorAllocator& alloc = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? m_gpuRing.m_cbvSrvUav : m_gpuRing.m_sampler;
		return alloc.allocate(descriptorCount);
	}

	U32 getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		ANKI_ASSERT(m_descriptorHandleIncrementSizes[type]);
		return m_descriptorHandleIncrementSizes[type];
	}

	Array<ID3D12DescriptorHeap*, 2> getCpuGpuVisibleTransientHeaps() const
	{
		return {m_descriptorHeaps[4], m_descriptorHeaps[5]};
	}

private:
	class
	{
	public:
		PersistentDescriptorAllocator m_cbvSrvUav; ///< Holds a CPU view that will be copied into a GPU heap at runtime.
		PersistentDescriptorAllocator m_sampler; ///< Same as m_cbvSrvUav
		PersistentDescriptorAllocator m_rtv; ///< Holds RTVs that the application needs.
		PersistentDescriptorAllocator m_dsv; ///< Holds DSVs that the application needs.
	} m_cpuPersistent;

	class
	{
	public:
		RingDescriptorAllocator m_cbvSrvUav;
		RingDescriptorAllocator m_sampler;
	} m_gpuRing;

	GrDynamicArray<ID3D12DescriptorHeap*> m_descriptorHeaps;

	Array<U32, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_descriptorHandleIncrementSizes = {};

	PersistentDescriptorAllocator& getCpuPersistentAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		PersistentDescriptorAllocator* alloc = nullptr;
		switch(type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			alloc = &m_cpuPersistent.m_cbvSrvUav;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			alloc = &m_cpuPersistent.m_sampler;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			alloc = &m_cpuPersistent.m_rtv;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			alloc = &m_cpuPersistent.m_dsv;
			break;
		default:
			ANKI_ASSERT(0);
		}
		return *alloc;
	}
};

/// @memberof RootSignatureFactory
class RootSignature
{
	friend class RootSignatureFactory;
	friend class DescriptorState;

public:
	ID3D12RootSignature& getD3DRootSignature() const
	{
		return *m_rootSignature;
	}

private:
	class Descriptor
	{
	public:
		DescriptorType m_type = DescriptorType::kCount;
		DescriptorFlag m_flags = DescriptorFlag::kNone;
		U16 m_structuredBufferStride = kMaxU8;
	};

	class Space
	{
	public:
		U32 m_cbvSrvUavCount = 0;

		U8 m_cbvSrvUavDescriptorTableRootParameterIdx = kMaxU8; ///< The index of the root parameter of the CBV/SRV/UAV table.
		U8 m_samplersDescriptorTableRootParameterIdx = kMaxU8; ///< The index of the root parameter of the sampler table.

		Array<GrDynamicArray<Descriptor>, U32(HlslResourceType::kCount)> m_descriptors; ///< Arrays are unrolled.
	};

	ID3D12RootSignature* m_rootSignature = nullptr;

	Array<Space, kMaxDescriptorSets> m_spaces;

	U32 m_rootConstantsSize = kMaxU32;
	U8 m_rootConstantsParameterIdx = kMaxU8;

	U64 m_hash = 0;
};

/// Creator of root signatures.
class RootSignatureFactory : public MakeSingleton<RootSignatureFactory>
{
public:
	~RootSignatureFactory();

	Error getOrCreateRootSignature(const ShaderReflection& refl, RootSignature*& signature);

private:
	GrDynamicArray<RootSignature*> m_signatures;
	Mutex m_mtx;
};

/// Part of the command buffer that deals with descriptors.
class DescriptorState
{
public:
	void init(StackMemoryPool* tempPool);

	void bindRootSignature(const RootSignature* rootSignature, Bool isCompute);

	void bindUav(U32 space, U32 registerBinding, ID3D12Resource* resource, PtrSize offset, PtrSize range)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kUav, space, registerBinding);
		descriptor.m_bufferView.m_resource = resource;
		descriptor.m_bufferView.m_offset = offset;
		descriptor.m_bufferView.m_range = range;
		descriptor.m_bufferView.m_format = DXGI_FORMAT_UNKNOWN;
		descriptor.m_isHandle = false;
#if ANKI_ASSERTIONS_ENABLED
		descriptor.m_type = DescriptorType::kStorageBuffer;
		descriptor.m_flags = DescriptorFlag::kReadWrite;
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void bindSrv(U32 space, U32 registerBinding, ID3D12Resource* resource, PtrSize offset, PtrSize range)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		descriptor.m_bufferView.m_resource = resource;
		descriptor.m_bufferView.m_offset = offset;
		descriptor.m_bufferView.m_range = range;
		descriptor.m_bufferView.m_format = DXGI_FORMAT_UNKNOWN;
		descriptor.m_isHandle = false;
#if ANKI_ASSERTIONS_ENABLED
		descriptor.m_type = DescriptorType::kStorageBuffer;
		descriptor.m_flags = DescriptorFlag::kRead;
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void flush(ID3D12GraphicsCommandList& cmdList);

private:
	class BufferView
	{
	public:
		ID3D12Resource* m_resource;
		PtrSize m_offset;
		PtrSize m_range;
		DXGI_FORMAT m_format;
	};

	class Descriptor
	{
	public:
		union
		{
			BufferView m_bufferView; ///< For buffers
			D3D12_CPU_DESCRIPTOR_HANDLE m_heapOffset; ///< For samplers and texture SRVs/UAVs
		};

		Bool m_isHandle;
#if ANKI_ASSERTIONS_ENABLED
		DescriptorType m_type = DescriptorType::kCount;
		DescriptorFlag m_flags = DescriptorFlag::kNone;
#endif

		Descriptor()
		{
			// No init for perf reasons
		}

		Descriptor& operator=(const Descriptor& b)
		{
			if(m_isHandle)
			{
				m_heapOffset = b.m_heapOffset;
			}
			else
			{
				m_bufferView = b.m_bufferView;
			}
			m_isHandle = b.m_isHandle;
#if ANKI_ASSERTIONS_ENABLED
			m_type = b.m_type;
			m_flags = b.m_flags;
#endif
			return *this;
		}
	};

	class Space
	{
	public:
		Array<DynamicArray<Descriptor, MemoryPoolPtrWrapper<StackMemoryPool>>, U32(HlslResourceType::kCount)> m_descriptors;
		DescriptorHeapHandle m_cbvSrvUavHeapHandle;
		DescriptorHeapHandle m_samplerHeapHandle;

		Bool m_cbvSrvUavDirty = true;
		Bool m_samplersDirty = true;
	};

	const RootSignature* m_rootSignature = nullptr;
	Array<Space, kMaxDescriptorSets> m_spaces;

	Bool m_rootSignatureNeedsRebinding = true;
	Bool m_rootSignatureIsCompute = false;

	Descriptor& getDescriptor(HlslResourceType svv, U32 space, U32 registerBinding)
	{
		if(registerBinding >= m_spaces[space].m_descriptors[svv].getSize())
		{
			m_spaces[space].m_descriptors[svv].resize(registerBinding + 1);
		}
		return m_spaces[space].m_descriptors[svv][registerBinding];
	}
};
/// @}

} // end namespace anki
