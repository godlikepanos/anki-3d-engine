// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	friend class PersistentDescriptorAllocator;
	friend class RingDescriptorAllocator;
	friend class DescriptorFactory;

public:
	[[nodiscard]] Bool isCreated() const
	{
		return m_cpuHandle.ptr != 0 || m_gpuHandle.ptr != 0;
	}

	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE getCpuOffset() const
	{
		validate();
		return m_cpuHandle;
	}

	[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE getGpuOffset() const
	{
		validate();
		ANKI_ASSERT(m_gpuHandle.ptr);
		return m_gpuHandle;
	}

	void increment(U32 descriptorCount);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {};

	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;

#if ANKI_ASSERTIONS_ENABLED
	D3D12_CPU_DESCRIPTOR_HANDLE m_heapCpuStart = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_heapGpuStart = {};
	PtrSize m_heapSize = 0;
#endif

	void validate() const
	{
		ANKI_ASSERT(m_cpuHandle.ptr);
		ANKI_ASSERT(m_heapType != D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);
		ANKI_ASSERT(m_heapCpuStart.ptr);
		ANKI_ASSERT(m_heapSize);

		ANKI_ASSERT(m_cpuHandle.ptr >= m_heapCpuStart.ptr
					&& m_cpuHandle.ptr + getDevice().GetDescriptorHandleIncrementSize(m_heapType) <= m_heapCpuStart.ptr + m_heapSize);
		if(m_gpuHandle.ptr)
		{
			ANKI_ASSERT(m_gpuHandle.ptr >= m_heapGpuStart.ptr
						&& m_gpuHandle.ptr + getDevice().GetDescriptorHandleIncrementSize(m_heapType) <= m_heapGpuStart.ptr + m_heapSize);
		}
	}
};

/// Descriptor allocator with persistent allocations.
class PersistentDescriptorAllocator
{
	friend class DescriptorFactory;

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
	void init(D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart, U32 descriptorSize, U32 descriptorCount,
			  CString name);

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

	Atomic<U64> m_incrementAtFrameStart = 0;

	GrString m_name;
};

/// A container of all descriptor heaps.
class DescriptorFactory : public MakeSingleton<DescriptorFactory>
{
public:
	~DescriptorFactory();

	Error init();

	/// @note Thread-safe.
	DescriptorHeapHandle allocatePersistent(D3D12_DESCRIPTOR_HEAP_TYPE type, Bool gpuVisible)
	{
		DescriptorHeapHandle out;
		if(!gpuVisible)
		{
			out = getCpuPersistentAllocator(type).allocate();
		}
		else
		{
			ANKI_ASSERT(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			out = m_gpuPersistent.m_cbvSrvUav.allocate();
		}
		out.m_heapType = type;
		return out;
	}

	/// @note Thread-safe.
	void freePersistent(DescriptorHeapHandle& handle)
	{
		const Bool gpuVisible = handle.m_gpuHandle.ptr != 0;
		if(gpuVisible)
		{
			ANKI_ASSERT(handle.m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_gpuPersistent.m_cbvSrvUav.free(handle);
		}
		else
		{
			getCpuPersistentAllocator(handle.m_heapType).free(handle);
		}
	}

	/// @note Thread-safe.
	DescriptorHeapHandle allocateTransient(D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorCount, Bool gpuVisible)
	{
		ANKI_ASSERT(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		ANKI_ASSERT(gpuVisible);
		RingDescriptorAllocator& alloc = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? m_gpuRing.m_cbvSrvUav : m_gpuRing.m_sampler;
		DescriptorHeapHandle out = alloc.allocate(descriptorCount);
		out.m_heapType = type;
		out.validate();
		return out;
	}

	U32 getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		ANKI_ASSERT(m_descriptorHandleIncrementSizes[type]);
		return m_descriptorHandleIncrementSizes[type];
	}

	Array<ID3D12DescriptorHeap*, 2> getCpuGpuVisibleHeaps() const
	{
		ANKI_ASSERT(m_descriptorHeaps.getSize() == 6);
		return {m_descriptorHeaps[4], m_descriptorHeaps[5]};
	}

	U32 getBindlessIndex(const DescriptorHeapHandle& handle) const
	{
		handle.validate();
		ANKI_ASSERT(handle.m_heapCpuStart.ptr == m_gpuPersistent.m_cbvSrvUav.m_cpuHeapStart.ptr);
		ANKI_ASSERT(handle.m_heapGpuStart.ptr == m_gpuPersistent.m_cbvSrvUav.m_gpuHeapStart.ptr);
		const PtrSize idx = (handle.m_cpuHandle.ptr - m_gpuPersistent.m_cbvSrvUav.m_cpuHeapStart.ptr) / m_gpuPersistent.m_cbvSrvUav.m_descriptorSize;
		return U32(idx);
	}

	/// @note Thread-safe.
	void endFrame()
	{
		m_gpuRing.m_cbvSrvUav.endFrame();
		m_gpuRing.m_sampler.endFrame();
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
		PersistentDescriptorAllocator m_cbvSrvUav; ///< For bindless resources.
	} m_gpuPersistent;

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
		U16 m_structuredBufferStride = kMaxU16;
	};

	class Space
	{
	public:
		U32 m_cbvSrvUavCount = 0; ///< Doesn't count holes.
		U32 m_samplerCount = 0; ///< Doesn't count holes.

		U8 m_cbvSrvUavDescriptorTableRootParameterIdx = kMaxU8; ///< The index of the root parameter of the CBV/SRV/UAV table.
		U8 m_samplersDescriptorTableRootParameterIdx = kMaxU8; ///< The index of the root parameter of the sampler table.

		Array<GrDynamicArray<Descriptor>, U32(HlslResourceType::kCount)> m_descriptors; ///< Arrays are unrolled. May have holes.

		Bool isEmpty() const
		{
			return m_cbvSrvUavCount == 0 && m_samplerCount == 0;
		}
	};

	ID3D12RootSignature* m_rootSignature = nullptr;

	Array<Space, kMaxRegisterSpaces> m_spaces;

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

	/// Local root signature for hit shaders.
	Error getOrCreateLocalRootSignature(const ShaderReflection& refl, RootSignature*& signature);

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

	void bindUav(U32 space, U32 registerBinding, ID3D12Resource* resource, PtrSize offset, PtrSize range, Format fmt = Format::kNone)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kUav, space, registerBinding);
		descriptor.m_bufferView.m_resource = resource;
		descriptor.m_bufferView.m_offset = offset;
		descriptor.m_bufferView.m_range = range;
		descriptor.m_bufferView.m_format = fmt;
		descriptor.m_isHandle = false;
#if ANKI_ASSERTIONS_ENABLED
		if(fmt == Format::kNone)
		{
			descriptor.m_type = DescriptorType::kUavStructuredBuffer;
		}
		else
		{
			descriptor.m_type = DescriptorType::kUavTexelBuffer;
		}
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void bindUav(U32 space, U32 registerBinding, DescriptorHeapHandle handle)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kUav, space, registerBinding);
		descriptor.m_heapOffset = handle.getCpuOffset();
		descriptor.m_isHandle = true;
#if ANKI_ASSERTIONS_ENABLED
		descriptor.m_type = DescriptorType::kUavTexture;
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void bindSrv(U32 space, U32 registerBinding, ID3D12Resource* resource, PtrSize offset, PtrSize range, Format fmt = Format::kNone)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		descriptor.m_bufferView.m_resource = resource;
		descriptor.m_bufferView.m_offset = offset;
		descriptor.m_bufferView.m_range = range;
		descriptor.m_bufferView.m_format = fmt;
		descriptor.m_isHandle = false;
#if ANKI_ASSERTIONS_ENABLED
		if(fmt == Format::kNone)
		{
			descriptor.m_type = DescriptorType::kSrvStructuredBuffer;
		}
		else
		{
			descriptor.m_type = DescriptorType::kSrvTexelBuffer;
		}
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void bindSrv(U32 space, U32 registerBinding, DescriptorHeapHandle handle)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		descriptor.m_heapOffset = handle.getCpuOffset();
		descriptor.m_isHandle = true;
#if ANKI_ASSERTIONS_ENABLED
		descriptor.m_type = DescriptorType::kSrvTexture;
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void bindSrv(U32 space, U32 registerBinding, D3D12_GPU_VIRTUAL_ADDRESS asAddress)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		descriptor.m_asAddress = asAddress;
		descriptor.m_isHandle = false;
#if ANKI_ASSERTIONS_ENABLED
		descriptor.m_type = DescriptorType::kAccelerationStructure;
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void bindCbv(U32 space, U32 registerBinding, ID3D12Resource* resource, PtrSize offset, PtrSize range)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kCbv, space, registerBinding);
		descriptor.m_bufferView.m_resource = resource;
		descriptor.m_bufferView.m_offset = offset;
		descriptor.m_bufferView.m_range = range;
		descriptor.m_bufferView.m_format = Format::kNone;
		descriptor.m_isHandle = false;
#if ANKI_ASSERTIONS_ENABLED
		descriptor.m_type = DescriptorType::kConstantBuffer;
#endif

		m_spaces[space].m_cbvSrvUavDirty = true;
	}

	void bindSampler(U32 space, U32 registerBinding, DescriptorHeapHandle handle)
	{
		Descriptor& descriptor = getDescriptor(HlslResourceType::kSampler, space, registerBinding);
		descriptor.m_heapOffset = handle.getCpuOffset();
		descriptor.m_isHandle = true;
#if ANKI_ASSERTIONS_ENABLED
		descriptor.m_type = DescriptorType::kSampler;
#endif

		m_spaces[space].m_samplersDirty = true;
	}

	void setRootConstants(const void* data, U32 dataSize)
	{
		ANKI_ASSERT(data && dataSize && dataSize <= kMaxFastConstantsSize);
		memcpy(m_rootConsts.getBegin(), data, dataSize);
		m_rootConstSize = dataSize;
		m_rootConstsDirty = true;
	}

	void flush(ID3D12GraphicsCommandList& cmdList);

private:
	class BufferView
	{
	public:
		ID3D12Resource* m_resource;
		PtrSize m_offset;
		PtrSize m_range;
		Format m_format;
	};

	class Descriptor
	{
	public:
		union
		{
			BufferView m_bufferView; ///< For buffers
			D3D12_CPU_DESCRIPTOR_HANDLE m_heapOffset; ///< For samplers and texture SRVs/UAVs
			D3D12_GPU_VIRTUAL_ADDRESS m_asAddress;
		};

		Bool m_isHandle;
#if ANKI_ASSERTIONS_ENABLED
		DescriptorType m_type = DescriptorType::kCount;
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
	Array<Space, kMaxRegisterSpaces> m_spaces;

	Array<U8, kMaxFastConstantsSize> m_rootConsts;
	U32 m_rootConstSize = 0;

	Bool m_rootSignatureNeedsRebinding = true;
	Bool m_rootSignatureIsCompute = false;
	Bool m_rootConstsDirty = true;

	Descriptor& getDescriptor(HlslResourceType svv, U32 space, U32 registerBinding)
	{
		if(registerBinding >= m_spaces[space].m_descriptors[svv].getSize())
		{
			m_spaces[space].m_descriptors[svv].resize(registerBinding + 1);
		}
		return m_spaces[space].m_descriptors[svv][registerBinding];
	}
};

inline void DescriptorHeapHandle::increment(U32 descriptorCount)
{
	m_cpuHandle.ptr += DescriptorFactory::getSingleton().getDescriptorHandleIncrementSize(m_heapType) * descriptorCount;
}
/// @}

} // end namespace anki
