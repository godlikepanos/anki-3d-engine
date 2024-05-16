// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Core/CVarSet.h>

namespace anki {

static NumericCVar<U16> g_maxRtvDescriptors(CVarSubsystem::kGr, "MaxRvtDescriptors", 128, 8, kMaxU16, "Max number of RTVs");
static NumericCVar<U16> g_maxDsvDescriptors(CVarSubsystem::kGr, "MaxDsvDescriptors", 128, 8, kMaxU16, "Max number of DSVs");
static NumericCVar<U16> g_maxCpuCbvSrvUavDescriptors(CVarSubsystem::kGr, "MaxCpuCbvSrvUavDescriptors", 1024, 8, kMaxU16,
													 "Max number of CBV/SRV/UAV descriptors");
static NumericCVar<U16> g_maxCpuSamplerDescriptors(CVarSubsystem::kGr, "MaxCpuSamplerDescriptors", 64, 8, kMaxU16,
												   "Max number of sampler descriptors");
static NumericCVar<U16> g_maxGpuCbvSrvUavDescriptors(CVarSubsystem::kGr, "MaxGpuCbvSrvUavDescriptors", 2 * 1024, 8, kMaxU16,
													 "Max number of CBV/SRV/UAV descriptors");
static NumericCVar<U16> g_maxGpuSamplerDescriptors(CVarSubsystem::kGr, "MaxGpuSamplerDescriptors", 128, 8, kMaxU16,
												   "Max number of sampler descriptors");

static Error createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, U32 descriptorCount,
								  ID3D12DescriptorHeap*& heap, D3D12_CPU_DESCRIPTOR_HANDLE& cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE& gpuHeapStart,
								  U32& descriptorSize)
{

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = descriptorCount;
	heapDesc.Type = type;
	heapDesc.Flags = flags;
	ANKI_D3D_CHECK(getDevice().CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	cpuHeapStart = heap->GetCPUDescriptorHandleForHeapStart();
	if(flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		gpuHeapStart = heap->GetGPUDescriptorHandleForHeapStart();
	}
	else
	{
		gpuHeapStart = {};
	}
	ANKI_ASSERT(cpuHeapStart.ptr != 0 || gpuHeapStart.ptr != 0);

	descriptorSize = getDevice().GetDescriptorHandleIncrementSize(type);

	return Error::kNone;
}

void PersistentDescriptorAllocator::init(D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart, U32 descriptorSize,
										 U16 descriptorCount)
{
	ANKI_ASSERT(descriptorSize > 0);
	ANKI_ASSERT(descriptorCount > 0);
	ANKI_ASSERT(cpuHeapStart.ptr != 0 || gpuHeapStart.ptr != 0);

	m_cpuHeapStart = cpuHeapStart;
	m_gpuHeapStart = gpuHeapStart;
	m_descriptorSize = descriptorSize;

	m_freeDescriptors.resize(descriptorCount);
	for(U16 i = 0; i < descriptorCount; ++i)
	{
		m_freeDescriptors[i] = i;
	}

	m_freeDescriptorsHead = 0;
}

DescriptorHeapHandle PersistentDescriptorAllocator::allocate()
{
	ANKI_ASSERT(m_descriptorSize > 0);

	U16 idx;
	{
		LockGuard lock(m_mtx);
		if(m_freeDescriptorsHead == m_freeDescriptors.getSize())
		{
			ANKI_D3D_LOGF("Out of descriptors");
		}
		idx = m_freeDescriptorsHead;
		++m_freeDescriptorsHead;
	}

	idx = m_freeDescriptors[idx];

	DescriptorHeapHandle out;
	out.m_cpuHandle.ptr = (m_cpuHeapStart.ptr) ? (m_cpuHeapStart.ptr + PtrSize(idx) * m_descriptorSize) : 0;
	out.m_gpuHandle.ptr = (m_gpuHeapStart.ptr) ? (m_gpuHeapStart.ptr + PtrSize(idx) * m_descriptorSize) : 0;

	return out;
}

void PersistentDescriptorAllocator::free(DescriptorHeapHandle& handle)
{
	ANKI_ASSERT(m_descriptorSize > 0);

	if(!handle.isCreated()) [[unlikely]]
	{
		return;
	}

	PtrSize descriptorOffsetFromStart;
	if(handle.m_cpuHandle.ptr)
	{
		ANKI_ASSERT(handle.m_cpuHandle.ptr >= m_cpuHeapStart.ptr
					&& handle.m_cpuHandle.ptr < m_cpuHeapStart.ptr + m_descriptorSize * m_freeDescriptors.getSize());
		descriptorOffsetFromStart = handle.m_cpuHandle.ptr - m_cpuHeapStart.ptr;
	}
	else
	{
		ANKI_ASSERT(handle.m_gpuHandle.ptr >= m_gpuHeapStart.ptr
					&& handle.m_gpuHandle.ptr < m_gpuHeapStart.ptr + m_descriptorSize * m_freeDescriptors.getSize());
		descriptorOffsetFromStart = handle.m_gpuHandle.ptr - m_gpuHeapStart.ptr;
	}

	const U16 idx = U16(descriptorOffsetFromStart / m_descriptorSize);

	LockGuard lock(m_mtx);
	ANKI_ASSERT(m_freeDescriptorsHead > 0);
	--m_freeDescriptorsHead;
	m_freeDescriptors[m_freeDescriptorsHead] = idx;
	handle = {};
}

void RingDescriptorAllocator::init(D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart, U32 descriptorSize,
								   U32 descriptorCount)
{
	ANKI_ASSERT(descriptorSize > 0);
	ANKI_ASSERT(descriptorCount > 0);
	ANKI_ASSERT(cpuHeapStart.ptr != 0 || gpuHeapStart.ptr != 0);

	m_cpuHeapStart = cpuHeapStart;
	m_gpuHeapStart = gpuHeapStart;
	m_descriptorSize = descriptorSize;
	m_descriptorCount = descriptorCount;
}

DescriptorHeapHandle RingDescriptorAllocator::allocate(U32 descriptorCount)
{
	ANKI_ASSERT(m_descriptorSize > 0);

	U32 firstDescriptor;
	Bool allocationPassesEnd = false;
	do
	{
		firstDescriptor = m_increment.fetchAdd(descriptorCount) % m_descriptorCount;

		allocationPassesEnd = firstDescriptor + descriptorCount > m_descriptorCount;
	} while(allocationPassesEnd);

	DescriptorHeapHandle out;
	out.m_cpuHandle.ptr = (m_cpuHeapStart.ptr) ? (m_cpuHeapStart.ptr + PtrSize(firstDescriptor) * m_descriptorSize) : 0;
	out.m_gpuHandle.ptr = (m_gpuHeapStart.ptr) ? (m_gpuHeapStart.ptr + PtrSize(firstDescriptor) * m_descriptorSize) : 0;

	return out;
}

void RingDescriptorAllocator::endFrame()
{
	const U64 crntIncrement = m_increment.load();

	const U32 descriptorsAllocatedThisFrame = U32(crntIncrement - m_incrementAtFrameStart);

	const U32 maxFramesInFlight = kMaxFramesInFlight + 1; // Be very conservative
	if(descriptorsAllocatedThisFrame > m_descriptorCount / maxFramesInFlight)
	{
		ANKI_D3D_LOGW("Allocated too many descriptors this frame");
	}

	m_incrementAtFrameStart = crntIncrement;
}

DescriptorFactory::~DescriptorFactory()
{
	for(ID3D12DescriptorHeap* h : m_descriptorHeaps)
	{
		safeRelease(h);
	}
}

Error DescriptorFactory::init()
{
	// Init CPU descriptors first
	auto createHeapAndAllocator = [this](D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, U16 descriptorCount,
										 PersistentDescriptorAllocator& alloc) -> Error {
		ID3D12DescriptorHeap* heap;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart;
		U32 descriptorSize;
		ANKI_CHECK(createDescriptorHeap(type, flags, descriptorCount, heap, cpuHeapStart, gpuHeapStart, descriptorSize));
		alloc.init(cpuHeapStart, gpuHeapStart, descriptorSize, descriptorCount);
		m_descriptorHeaps.emplaceBack(heap);
		return Error::kNone;
	};

	ANKI_CHECK(createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_maxCpuCbvSrvUavDescriptors.get(),
									  m_cpuPersistent.m_cbvSrvUav));
	ANKI_CHECK(createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_maxCpuSamplerDescriptors.get(),
									  m_cpuPersistent.m_sampler));
	ANKI_CHECK(
		createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_maxRtvDescriptors.get(), m_cpuPersistent.m_rtv));
	ANKI_CHECK(
		createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_maxDsvDescriptors.get(), m_cpuPersistent.m_dsv));

	// Init GPU descriptors
	auto createHeapAndAllocator2 = [this](D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, U16 descriptorCount,
										  RingDescriptorAllocator& alloc) -> Error {
		ID3D12DescriptorHeap* heap;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart;
		U32 descriptorSize;
		ANKI_CHECK(createDescriptorHeap(type, flags, descriptorCount, heap, cpuHeapStart, gpuHeapStart, descriptorSize));
		alloc.init(cpuHeapStart, gpuHeapStart, descriptorSize, descriptorCount);
		m_descriptorHeaps.emplaceBack(heap);
		return Error::kNone;
	};

	ANKI_CHECK(createHeapAndAllocator2(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
									   g_maxGpuCbvSrvUavDescriptors.get(), m_gpuRing.m_cbvSrvUav));
	ANKI_CHECK(createHeapAndAllocator2(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
									   g_maxGpuSamplerDescriptors.get(), m_gpuRing.m_sampler));

	for(D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE(0); type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		type = D3D12_DESCRIPTOR_HEAP_TYPE(type + 1))
	{
		m_descriptorHandleIncrementSizes[type] = getDevice().GetDescriptorHandleIncrementSize(type);
	}

	return Error::kNone;
}

RootSignatureFactory::~RootSignatureFactory()
{
	for(auto sig : m_signatures)
	{
		safeRelease(sig->m_rootSignature);
		deleteInstance(GrMemoryPool::getSingleton(), sig);
	}
}

Error RootSignatureFactory::getOrCreateRootSignature(const ShaderReflection& refl, RootSignature*& signature)
{
	// Compute the hash
	// TODO
	U64 hash = 0;

	// Search if exists
	LockGuard lock(m_mtx);

	for(RootSignature* s : m_signatures)
	{
		if(s->m_hash == hash)
		{
			signature = s;
			return Error::kNone;
		}
	}

	// Not found, create one

	GrDynamicArray<D3D12_ROOT_PARAMETER1> rootParameters;

	// Create the tables
	Array<GrDynamicArray<D3D12_DESCRIPTOR_RANGE1>, kMaxBindingsPerDescriptorSet * 2> tableRanges; // One per descriptor table
	U32 tableRangesCount = 0;
	for(U32 space = 0; space < kMaxDescriptorSets; ++space)
	{
		if(!refl.m_descriptorSetMask.get(space))
		{
			continue;
		}

		GrDynamicArray<D3D12_DESCRIPTOR_RANGE1>& nonSamplerRanges = tableRanges[tableRangesCount++];
		GrDynamicArray<D3D12_DESCRIPTOR_RANGE1>& samplerRanges = tableRanges[tableRangesCount++];

		// Create the ranges
		for(U32 bindingIdx = 0; bindingIdx < refl.m_bindingCounts[space]; ++bindingIdx)
		{
			const ShaderReflectionBinding& akBinding = refl.m_bindings[space][bindingIdx];
			akBinding.validate();
			D3D12_DESCRIPTOR_RANGE1& range =
				(akBinding.m_type == DescriptorType::kSampler) ? *samplerRanges.emplaceBack() : *nonSamplerRanges.emplaceBack();

			range = {};
			range.NumDescriptors = akBinding.m_arraySize;
			range.BaseShaderRegister = akBinding.m_registerBindingPoint;
			range.RegisterSpace = space;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;

			if(akBinding.m_type == DescriptorType::kUniformBuffer)
			{
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			}
			else if(akBinding.m_type == DescriptorType::kSampler)
			{
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			}
			else if(!(akBinding.m_flags & DescriptorFlag::kWrite))
			{
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			}
			else
			{
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			}
		}

		if(nonSamplerRanges.getSize())
		{
			D3D12_ROOT_PARAMETER1& table = *rootParameters.emplaceBack();
			table = {};
			table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			table.DescriptorTable.NumDescriptorRanges = nonSamplerRanges.getSize();
			table.DescriptorTable.pDescriptorRanges = nonSamplerRanges.getBegin();
			table.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		if(samplerRanges.getSize())
		{
			D3D12_ROOT_PARAMETER1& table = *rootParameters.emplaceBack();
			table = {};
			table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			table.DescriptorTable.NumDescriptorRanges = samplerRanges.getSize();
			table.DescriptorTable.pDescriptorRanges = samplerRanges.getBegin();
			table.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}
	}

	// Root constants
	if(refl.m_pushConstantsSize)
	{
		D3D12_ROOT_PARAMETER1& rootParam = *rootParameters.emplaceBack();
		rootParam = {};
		rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		ANKI_ASSERT((refl.m_pushConstantsSize % 4) == 0);
		rootParam.Constants.Num32BitValues = refl.m_pushConstantsSize / 4;
		rootParam.Constants.RegisterSpace = kPushConstantsRegisterSpace;
		rootParam.Constants.ShaderRegister = kPushConstantsRegisterBindPoint;
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC verSigDesc = {};
	verSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

	D3D12_ROOT_SIGNATURE_DESC1& sigDesc = verSigDesc.Desc_1_1;
	sigDesc.NumParameters = rootParameters.getSize();
	sigDesc.pParameters = rootParameters.getBegin();

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ANKI_D3D_CHECK(D3D12SerializeVersionedRootSignature(&verSigDesc, &signatureBlob, &errorBlob));

	ID3D12RootSignature* dxRootSig;
	ANKI_D3D_CHECK(getDevice().CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&dxRootSig)));

	// Create the signature
	signature = newInstance<RootSignature>(GrMemoryPool::getSingleton());
	signature->m_hash = hash;
	signature->m_rootSignature = dxRootSig;

	U8 rootParameterIdx = 0;
	for(U32 spaceIdx = 0; spaceIdx < kMaxDescriptorSets; ++spaceIdx)
	{
		if(!refl.m_descriptorSetMask.get(spaceIdx))
		{
			continue;
		}

		RootSignature::Space& outSpace = signature->m_spaces[spaceIdx];

		for(U32 bindingIdx = 0; bindingIdx < refl.m_bindingCounts[spaceIdx]; ++bindingIdx)
		{
			const ShaderReflectionBinding& inBinding = refl.m_bindings[spaceIdx][bindingIdx];
			inBinding.validate();
			const HlslResourceType hlslResourceType = descriptorTypeToHlslResourceType(inBinding.m_type, inBinding.m_flags);

			if(hlslResourceType < HlslResourceType::kSampler)
			{
				++outSpace.m_cbvSrvUavCount;
			}

			for(U32 arrayIdx = 0; arrayIdx < inBinding.m_arraySize; ++arrayIdx)
			{
				RootSignature::Descriptor& outDescriptor = *signature->m_spaces[spaceIdx].m_descriptors[hlslResourceType].emplaceBack();
				outDescriptor.m_flags = inBinding.m_flags;
				outDescriptor.m_type = inBinding.m_type;
				if(outDescriptor.m_type == DescriptorType::kStorageBuffer)
				{
					outDescriptor.m_structuredBufferStride = inBinding.m_d3dStructuredBufferStride;
				}
			}
		}

		if(outSpace.m_cbvSrvUavCount)
		{
			outSpace.m_cbvSrvUavDescriptorTableRootParameterIdx = rootParameterIdx++;
		}

		if(outSpace.m_descriptors[HlslResourceType::kSampler].getSize())
		{
			outSpace.m_samplersDescriptorTableRootParameterIdx = rootParameterIdx++;
		}
	}

	signature->m_rootConstantsSize = refl.m_pushConstantsSize;
	if(signature->m_rootConstantsSize)
	{
		signature->m_rootConstantsParameterIdx = rootParameterIdx++;
	}

	m_signatures.emplaceBack(signature);

	return Error::kNone;
}

void DescriptorState::init(StackMemoryPool* tempPool)
{
	for(auto& s : m_spaces)
	{
		for(auto& d : s.m_descriptors)
		{
			d = DynamicArray<Descriptor, MemoryPoolPtrWrapper<StackMemoryPool>>(tempPool);
		}
	}
}

void DescriptorState::bindRootSignature(const RootSignature* rootSignature, Bool isCompute)
{
	ANKI_ASSERT(rootSignature);

	if(rootSignature == m_rootSignature)
	{
		ANKI_ASSERT(m_rootSignatureNeedsRebinding == false);
		return;
	}

	for(U32 space = 0; space < kMaxDescriptorSets; ++space)
	{
		if(rootSignature->m_spaces[space].m_descriptors.getSize())
		{
			m_spaces[space].m_cbvSrvUavDirty = true;
			m_spaces[space].m_samplersDirty = true;
		}
	}

	m_rootSignature = rootSignature;
	m_rootSignatureNeedsRebinding = true;
	m_rootSignatureIsCompute = isCompute;
}

void DescriptorState::flush(ID3D12GraphicsCommandList& cmdList)
{
	// Rebind the root signature
	ANKI_ASSERT(m_rootSignature);
	const Bool rootSignatureNeedsRebinding = m_rootSignatureNeedsRebinding;
	if(m_rootSignatureNeedsRebinding)
	{
		if(m_rootSignatureIsCompute)
		{
			cmdList.SetComputeRootSignature(m_rootSignature->m_rootSignature);
		}
		else
		{
			cmdList.SetGraphicsRootSignature(m_rootSignature->m_rootSignature);
		}

		m_rootSignatureNeedsRebinding = false;
	}

	// Populate the heaps
	for(U32 spaceIdx = 0; spaceIdx < kMaxDescriptorSets; ++spaceIdx)
	{
		const RootSignature::Space& rootSignatureSpace = m_rootSignature->m_spaces[spaceIdx];
		if(rootSignatureSpace.m_descriptors.getSize() == 0)
		{
			continue;
		}

		Space& stateSpace = m_spaces[spaceIdx];

		// Allocate descriptor memory
		if(stateSpace.m_cbvSrvUavDirty && rootSignatureSpace.m_cbvSrvUavDescriptorTableRootParameterIdx != kMaxU8)
		{
			stateSpace.m_cbvSrvUavHeapHandle = DescriptorFactory::getSingleton().allocateCpuGpuVisibleTransient(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, rootSignatureSpace.m_cbvSrvUavCount);
		}

		if(stateSpace.m_samplersDirty && rootSignatureSpace.m_descriptors[HlslResourceType::kSampler].getSize())
		{
			stateSpace.m_samplerHeapHandle = DescriptorFactory::getSingleton().allocateCpuGpuVisibleTransient(
				D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, rootSignatureSpace.m_descriptors[HlslResourceType::kSampler].getSize());
		}

		// Populate descriptors
		if(stateSpace.m_cbvSrvUavDirty && rootSignatureSpace.m_cbvSrvUavCount)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapOffset = stateSpace.m_cbvSrvUavHeapHandle.m_cpuHandle;

			for(HlslResourceType hlslResourceType : EnumIterable<HlslResourceType>())
			{
				for(U32 registerBinding = 0; registerBinding < rootSignatureSpace.m_descriptors[hlslResourceType].getSize(); ++registerBinding)
				{
					const RootSignature::Descriptor& inDescriptor = rootSignatureSpace.m_descriptors[hlslResourceType][registerBinding];
					const Descriptor& outDescriptor = stateSpace.m_descriptors[hlslResourceType][registerBinding];
					ANKI_ASSERT(inDescriptor.m_flags == outDescriptor.m_flags && inDescriptor.m_type == outDescriptor.m_type
								&& "Have bound the wront thing");

					if(outDescriptor.m_isHandle)
					{
						ANKI_ASSERT(!"TODO");
					}
					else
					{
						const BufferView& view = outDescriptor.m_bufferView;

						if(inDescriptor.m_type == DescriptorType::kStorageBuffer && !!(inDescriptor.m_flags & DescriptorFlag::kWrite))
						{
							// RWStructuredBuffer
							D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
							uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
							ANKI_ASSERT((view.m_offset % inDescriptor.m_structuredBufferStride) == 0);
							uavDesc.Buffer.FirstElement = view.m_offset / inDescriptor.m_structuredBufferStride;
							ANKI_ASSERT((view.m_range % inDescriptor.m_structuredBufferStride) == 0);
							uavDesc.Buffer.NumElements = U32(view.m_range / inDescriptor.m_structuredBufferStride);
							uavDesc.Buffer.StructureByteStride = inDescriptor.m_structuredBufferStride;

							getDevice().CreateUnorderedAccessView(view.m_resource, nullptr, &uavDesc, cpuHeapOffset);
						}
						else if(inDescriptor.m_type == DescriptorType::kStorageBuffer && inDescriptor.m_flags == DescriptorFlag::kRead)
						{
							// StructuredBuffer
							D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
							srvDesc.Format = DXGI_FORMAT_UNKNOWN;
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
							srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
								D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1,
								D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);
							ANKI_ASSERT((view.m_offset % inDescriptor.m_structuredBufferStride) == 0);
							srvDesc.Buffer.FirstElement = view.m_offset / inDescriptor.m_structuredBufferStride;
							ANKI_ASSERT((view.m_range % inDescriptor.m_structuredBufferStride) == 0);
							srvDesc.Buffer.NumElements = U32(view.m_range / inDescriptor.m_structuredBufferStride);
							srvDesc.Buffer.StructureByteStride = inDescriptor.m_structuredBufferStride;

							getDevice().CreateShaderResourceView(view.m_resource, &srvDesc, cpuHeapOffset);
						}
						else
						{
							ANKI_ASSERT(!"TODO");
						}
					}

					cpuHeapOffset.ptr += DescriptorFactory::getSingleton().getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
			}
		}
	}

	// Bind root parameters
	for(U32 spaceIdx = 0; spaceIdx < kMaxDescriptorSets; ++spaceIdx)
	{
		const RootSignature::Space& rootSignatureSpace = m_rootSignature->m_spaces[spaceIdx];
		if(rootSignatureSpace.m_descriptors.getSize() == 0)
		{
			continue;
		}

		Space& stateSpace = m_spaces[spaceIdx];

		if(rootSignatureSpace.m_cbvSrvUavCount && (stateSpace.m_cbvSrvUavDirty || rootSignatureNeedsRebinding))
		{
			ANKI_ASSERT(rootSignatureSpace.m_cbvSrvUavDescriptorTableRootParameterIdx < kMaxU8);

			if(m_rootSignatureIsCompute)
			{
				cmdList.SetComputeRootDescriptorTable(rootSignatureSpace.m_cbvSrvUavDescriptorTableRootParameterIdx,
													  stateSpace.m_cbvSrvUavHeapHandle.m_gpuHandle);
			}
			else
			{
				cmdList.SetGraphicsRootDescriptorTable(rootSignatureSpace.m_cbvSrvUavDescriptorTableRootParameterIdx,
													   stateSpace.m_cbvSrvUavHeapHandle.m_gpuHandle);
			}
			stateSpace.m_cbvSrvUavDirty = false;
		}

		if(rootSignatureSpace.m_descriptors[HlslResourceType::kSampler].getSize() && (stateSpace.m_samplersDirty || rootSignatureNeedsRebinding))
		{
			ANKI_ASSERT(rootSignatureSpace.m_samplersDescriptorTableRootParameterIdx < kMaxU8);

			if(m_rootSignatureIsCompute)
			{
				cmdList.SetComputeRootDescriptorTable(rootSignatureSpace.m_samplersDescriptorTableRootParameterIdx,
													  stateSpace.m_samplerHeapHandle.m_gpuHandle);
			}
			else
			{
				cmdList.SetGraphicsRootDescriptorTable(rootSignatureSpace.m_samplersDescriptorTableRootParameterIdx,
													   stateSpace.m_samplerHeapHandle.m_gpuHandle);
			}
			stateSpace.m_samplersDirty = false;
		}
	}

	// Set root constants
	ANKI_ASSERT(m_rootSignature->m_rootConstantsSize == 0 && "TODO");
}

} // end namespace anki
