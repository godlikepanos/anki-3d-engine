// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Util/CVarSet.h>

namespace anki {

static Error createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, U32 descriptorCount, CString name,
								  ID3D12DescriptorHeap*& heap, D3D12_CPU_DESCRIPTOR_HANDLE& cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE& gpuHeapStart,
								  U32& descriptorSize)
{

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = descriptorCount;
	heapDesc.Type = type;
	heapDesc.Flags = flags;
	ANKI_D3D_CHECK(getDevice().CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	ANKI_ASSERT(name.getLength() > 0);
	GrDynamicArray<WChar> wstr;
	wstr.resize(name.getLength() + 1);
	name.toWideChars(wstr.getBegin(), wstr.getSize());
	heap->SetName(wstr.getBegin());

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
#if ANKI_ASSERTIONS_ENABLED
	out.m_heapCpuStart = m_cpuHeapStart;
	out.m_heapGpuStart = m_gpuHeapStart;
	out.m_heapSize = m_descriptorSize * m_freeDescriptors.getSize();
#endif

	return out;
}

void PersistentDescriptorAllocator::free(DescriptorHeapHandle& handle)
{
	ANKI_ASSERT(m_descriptorSize > 0);

	if(!handle.isCreated()) [[unlikely]]
	{
		return;
	}

	handle.validate();
	ANKI_ASSERT(handle.m_heapCpuStart.ptr == m_cpuHeapStart.ptr);

	const PtrSize descriptorOffsetFromStart = handle.m_cpuHandle.ptr - m_cpuHeapStart.ptr;
	const U16 idx = U16(descriptorOffsetFromStart / m_descriptorSize);

	LockGuard lock(m_mtx);
	ANKI_ASSERT(m_freeDescriptorsHead > 0);
	--m_freeDescriptorsHead;
	m_freeDescriptors[m_freeDescriptorsHead] = idx;
	handle = {};
}

void RingDescriptorAllocator::init(D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart, D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart, U32 descriptorSize,
								   U32 descriptorCount, CString name)
{
	ANKI_ASSERT(descriptorSize > 0);
	ANKI_ASSERT(descriptorCount > 0);
	ANKI_ASSERT(cpuHeapStart.ptr != 0 || gpuHeapStart.ptr != 0);

	m_cpuHeapStart = cpuHeapStart;
	m_gpuHeapStart = gpuHeapStart;
	m_descriptorSize = descriptorSize;
	m_descriptorCount = descriptorCount;
	m_name = name;
}

DescriptorHeapHandle RingDescriptorAllocator::allocate(U32 descriptorCount)
{
	ANKI_ASSERT(descriptorCount > 0);
	ANKI_ASSERT(m_descriptorSize > 0);

	U32 firstDescriptor;
	Bool allocationPassesEnd = false;
	U64 increment;
	do
	{
		increment = m_increment.fetchAdd(descriptorCount);
		firstDescriptor = increment % m_descriptorCount;

		allocationPassesEnd = firstDescriptor + descriptorCount > m_descriptorCount;
	} while(allocationPassesEnd);

	const U64 frameIncrementEnd = m_incrementAtFrameStart.load() + m_descriptorCount / (kMaxFramesInFlight + 1);

	if(increment >= frameIncrementEnd)
	{
		ANKI_D3D_LOGW("Allocated too many descriptors from the ring buffer %s. Need to increase the limits", m_name.cstr());
	}

	DescriptorHeapHandle out;
	out.m_cpuHandle.ptr = (m_cpuHeapStart.ptr) ? (m_cpuHeapStart.ptr + firstDescriptor * m_descriptorSize) : 0;
	out.m_gpuHandle.ptr = (m_gpuHeapStart.ptr) ? (m_gpuHeapStart.ptr + firstDescriptor * m_descriptorSize) : 0;
#if ANKI_ASSERTIONS_ENABLED
	out.m_heapCpuStart = m_cpuHeapStart;
	out.m_heapGpuStart = m_gpuHeapStart;
	out.m_heapSize = m_descriptorSize * m_descriptorCount;
#endif

	return out;
}

void RingDescriptorAllocator::endFrame()
{
	m_incrementAtFrameStart.store(m_increment.load());
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
	auto createHeapAndAllocator = [this](D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, U16 descriptorCount, CString name,
										 PersistentDescriptorAllocator& alloc) -> Error {
		ID3D12DescriptorHeap* heap;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart;
		U32 descriptorSize;
		ANKI_CHECK(createDescriptorHeap(type, flags, descriptorCount, name, heap, cpuHeapStart, gpuHeapStart, descriptorSize));
		alloc.init(cpuHeapStart, gpuHeapStart, descriptorSize, descriptorCount);
		m_descriptorHeaps.emplaceBack(heap);
		return Error::kNone;
	};

	ANKI_CHECK(createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_cvarGrMaxCpuCbvSrvUavDescriptors,
									  "CPU CBV/SRV/UAV", m_cpuPersistent.m_cbvSrvUav));
	ANKI_CHECK(createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_cvarGrMaxCpuSamplerDescriptors,
									  "CPU samplers", m_cpuPersistent.m_sampler));
	ANKI_CHECK(createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_cvarGrMaxRtvDescriptors, "CPU RTV",
									  m_cpuPersistent.m_rtv));
	ANKI_CHECK(createHeapAndAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, g_cvarGrMaxDsvDescriptors, "CPU DSV",
									  m_cpuPersistent.m_dsv));

	// Init GPU visible heaps
	ID3D12DescriptorHeap* heap;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart;
	U32 descriptorSize;
	ANKI_CHECK(createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
									g_cvarGrMaxGpuCbvSrvUavDescriptors + g_cvarGrMaxBindlessSampledTextureCount, "GPU CBV/SRV/UAV", heap,
									cpuHeapStart, gpuHeapStart, descriptorSize));
	m_descriptorHeaps.emplaceBack(heap);

	m_gpuPersistent.m_cbvSrvUav.init(cpuHeapStart, gpuHeapStart, descriptorSize, U16(g_cvarGrMaxBindlessSampledTextureCount));

	cpuHeapStart.ptr += descriptorSize * g_cvarGrMaxBindlessSampledTextureCount;
	gpuHeapStart.ptr += descriptorSize * g_cvarGrMaxBindlessSampledTextureCount;
	m_gpuRing.m_cbvSrvUav.init(cpuHeapStart, gpuHeapStart, descriptorSize, g_cvarGrMaxGpuCbvSrvUavDescriptors, "CBV/SRV/UAV");

	ANKI_CHECK(createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, g_cvarGrMaxGpuSamplerDescriptors,
									"GPU samplers", heap, cpuHeapStart, gpuHeapStart, descriptorSize));
	m_descriptorHeaps.emplaceBack(heap);
	m_gpuRing.m_sampler.init(cpuHeapStart, gpuHeapStart, descriptorSize, g_cvarGrMaxGpuSamplerDescriptors, "Samplers");

	// Misc
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
	const U64 hash = computeHash(&refl.m_descriptor, sizeof(refl.m_descriptor));

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
	Array<GrDynamicArray<D3D12_DESCRIPTOR_RANGE1>, kMaxBindingsPerRegisterSpace * 2> tableRanges; // One per descriptor table
	U32 tableRangesCount = 0;
	for(U32 space = 0; space < kMaxRegisterSpaces; ++space)
	{
		if(refl.m_descriptor.m_bindingCounts[space] == 0)
		{
			continue;
		}

		GrDynamicArray<D3D12_DESCRIPTOR_RANGE1>& nonSamplerRanges = tableRanges[tableRangesCount++];
		GrDynamicArray<D3D12_DESCRIPTOR_RANGE1>& samplerRanges = tableRanges[tableRangesCount++];

		// Create the ranges
		for(U32 bindingIdx = 0; bindingIdx < refl.m_descriptor.m_bindingCounts[space]; ++bindingIdx)
		{
			const ShaderReflectionBinding& akBinding = refl.m_descriptor.m_bindings[space][bindingIdx];
			const Bool isSampler = (akBinding.m_type == DescriptorType::kSampler);
			akBinding.validate();
			D3D12_DESCRIPTOR_RANGE1& range = (isSampler) ? *samplerRanges.emplaceBack() : *nonSamplerRanges.emplaceBack();
			const HlslResourceType hlslRsrcType = descriptorTypeToHlslResourceType(akBinding.m_type);

			range = {};
			range.NumDescriptors = akBinding.m_arraySize;
			range.BaseShaderRegister = akBinding.m_registerBindingPoint;
			range.RegisterSpace = space;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			range.Flags = {};

			if(!isSampler)
			{
				range.Flags |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
			}

			if(hlslRsrcType == HlslResourceType::kCbv)
			{
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			}
			else if(hlslRsrcType == HlslResourceType::kSampler)
			{
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			}
			else if(hlslRsrcType == HlslResourceType::kSrv)
			{
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			}
			else
			{
				ANKI_ASSERT(hlslRsrcType == HlslResourceType::kUav);
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			}
		}

		// Add the root parameter
		if(nonSamplerRanges.getSize())
		{
			D3D12_ROOT_PARAMETER1& table = *rootParameters.emplaceBack();
			table = {};
			table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			table.DescriptorTable.NumDescriptorRanges = nonSamplerRanges.getSize();
			table.DescriptorTable.pDescriptorRanges = nonSamplerRanges.getBegin();
			table.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		// Add the root parameter
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
	if(refl.m_descriptor.m_fastConstantsSize)
	{
		D3D12_ROOT_PARAMETER1& rootParam = *rootParameters.emplaceBack();
		rootParam = {};
		rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		ANKI_ASSERT((refl.m_descriptor.m_fastConstantsSize % 4) == 0);
		rootParam.Constants.Num32BitValues = refl.m_descriptor.m_fastConstantsSize / 4;
		rootParam.Constants.RegisterSpace = ANKI_D3D_FAST_CONSTANTS_SPACE;
		rootParam.Constants.ShaderRegister = 0;
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	// DrawID
	U32 drawIdRootParam = kMaxU32;
	if(refl.m_descriptor.m_d3dHasDrawId)
	{
		drawIdRootParam = rootParameters.getSize();

		D3D12_ROOT_PARAMETER1& rootParam = *rootParameters.emplaceBack();
		rootParam = {};
		rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParam.Constants.Num32BitValues = 1;
		rootParam.Constants.RegisterSpace = ANKI_D3D_DRAW_ID_CONSTANT_SPACE;
		rootParam.Constants.ShaderRegister = 0;
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	}

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC verSigDesc = {};
	verSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

	D3D12_ROOT_SIGNATURE_DESC1& sigDesc = verSigDesc.Desc_1_1;
	sigDesc.NumParameters = rootParameters.getSize();
	sigDesc.pParameters = rootParameters.getBegin();
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	const HRESULT ret = D3D12SerializeVersionedRootSignature(&verSigDesc, &signatureBlob, &errorBlob);
	if(ret != S_OK)
	{
		const Char* errc = reinterpret_cast<const Char*>(errorBlob->GetBufferPointer());
		ANKI_D3D_LOGE("D3D12SerializeVersionedRootSignature() failed: %s", errc);
	}

	ID3D12RootSignature* dxRootSig;
	ANKI_D3D_CHECK(getDevice().CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&dxRootSig)));

	// Create the signature
	signature = newInstance<RootSignature>(GrMemoryPool::getSingleton());
	signature->m_hash = hash;
	signature->m_rootSignature = dxRootSig;
	signature->m_drawIdRootParamIdx = drawIdRootParam;

	U8 rootParameterIdx = 0;
	for(U32 spaceIdx = 0; spaceIdx < kMaxRegisterSpaces; ++spaceIdx)
	{
		if(refl.m_descriptor.m_bindingCounts[spaceIdx] == 0)
		{
			continue;
		}

		RootSignature::Space& outSpace = signature->m_spaces[spaceIdx];

		for(U32 bindingIdx = 0; bindingIdx < refl.m_descriptor.m_bindingCounts[spaceIdx]; ++bindingIdx)
		{
			const ShaderReflectionBinding& inBinding = refl.m_descriptor.m_bindings[spaceIdx][bindingIdx];
			inBinding.validate();
			const HlslResourceType hlslResourceType = descriptorTypeToHlslResourceType(inBinding.m_type);

			if(hlslResourceType < HlslResourceType::kSampler)
			{
				outSpace.m_cbvSrvUavCount += inBinding.m_arraySize;
			}
			else
			{
				outSpace.m_samplerCount += inBinding.m_arraySize;
			}

			for(U32 arrayIdx = 0; arrayIdx < inBinding.m_arraySize; ++arrayIdx)
			{
				const U32 idxInDescriptorsArr = inBinding.m_registerBindingPoint + arrayIdx;
				if(idxInDescriptorsArr >= signature->m_spaces[spaceIdx].m_descriptors[hlslResourceType].getSize())
				{
					signature->m_spaces[spaceIdx].m_descriptors[hlslResourceType].resize(idxInDescriptorsArr + 1); // Account for holes
				}

				RootSignature::Descriptor& outDescriptor = signature->m_spaces[spaceIdx].m_descriptors[hlslResourceType][idxInDescriptorsArr];
				outDescriptor.m_type = inBinding.m_type;
				if(outDescriptor.m_type == DescriptorType::kSrvStructuredBuffer || outDescriptor.m_type == DescriptorType::kUavStructuredBuffer)
				{
					ANKI_ASSERT(inBinding.m_d3dStructuredBufferStride < kMaxU16);
					outDescriptor.m_structuredBufferStride = inBinding.m_d3dStructuredBufferStride;
				}
				else if(outDescriptor.m_type == DescriptorType::kSrvByteAddressBuffer
						|| outDescriptor.m_type == DescriptorType::kUavByteAddressBuffer)
				{
					ANKI_ASSERT(inBinding.m_d3dStructuredBufferStride == sizeof(U32));
					outDescriptor.m_structuredBufferStride = sizeof(U32);
				}
			}
		}

		if(outSpace.m_cbvSrvUavCount)
		{
			outSpace.m_cbvSrvUavDescriptorTableRootParameterIdx = rootParameterIdx++;
		}

		if(outSpace.m_samplerCount)
		{
			outSpace.m_samplersDescriptorTableRootParameterIdx = rootParameterIdx++;
		}
	}

	signature->m_rootConstantsSize = refl.m_descriptor.m_fastConstantsSize;
	if(signature->m_rootConstantsSize)
	{
		signature->m_rootConstantsParameterIdx = rootParameterIdx++;
	}

	m_signatures.emplaceBack(signature);

	return Error::kNone;
}

Error RootSignatureFactory::getOrCreateLocalRootSignature(const ShaderReflection& refl, RootSignature*& signature)
{
	ANKI_ASSERT(refl.m_descriptor.m_d3dShaderBindingTableRecordConstantsSize > 0
				&& (refl.m_descriptor.m_d3dShaderBindingTableRecordConstantsSize % 4) == 0);

	// Compute the hash
	const U64 hash = computeHash(&refl.m_descriptor.m_d3dShaderBindingTableRecordConstantsSize,
								 sizeof(refl.m_descriptor.m_d3dShaderBindingTableRecordConstantsSize));

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

	D3D12_ROOT_PARAMETER1 rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam.Constants.Num32BitValues = refl.m_descriptor.m_d3dShaderBindingTableRecordConstantsSize / 4;
	rootParam.Constants.RegisterSpace = ANKI_D3D_SHADER_RECORD_CONSTANTS_SPACE;
	rootParam.Constants.ShaderRegister = 0;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC verSigDesc = {};
	verSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

	D3D12_ROOT_SIGNATURE_DESC1& sigDesc = verSigDesc.Desc_1_1;
	sigDesc.NumParameters = 1;
	sigDesc.pParameters = &rootParam;
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	const HRESULT ret = D3D12SerializeVersionedRootSignature(&verSigDesc, &signatureBlob, &errorBlob);
	if(ret != S_OK)
	{
		const Char* errc = reinterpret_cast<const Char*>(errorBlob->GetBufferPointer());
		ANKI_D3D_LOGE("D3D12SerializeVersionedRootSignature() failed: %s", errc);
	}

	ID3D12RootSignature* dxRootSig;
	ANKI_D3D_CHECK(getDevice().CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&dxRootSig)));

	// Create the signature
	signature = newInstance<RootSignature>(GrMemoryPool::getSingleton());
	signature->m_hash = hash;
	signature->m_rootSignature = dxRootSig;

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

	if(m_rootSignature && rootSignature->m_hash == m_rootSignature->m_hash)
	{
		ANKI_ASSERT(m_rootSignatureNeedsRebinding == false);
		return;
	}

	for(U32 space = 0; space < kMaxRegisterSpaces; ++space)
	{
		if(!rootSignature->m_spaces[space].isEmpty())
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
	for(U32 spaceIdx = 0; spaceIdx < kMaxRegisterSpaces; ++spaceIdx)
	{
		const RootSignature::Space& rootSignatureSpace = m_rootSignature->m_spaces[spaceIdx];
		if(rootSignatureSpace.isEmpty())
		{
			continue;
		}

		Space& stateSpace = m_spaces[spaceIdx];

		// Allocate descriptor memory (doesn't include holes)
		{
			Bool skip = true;

			if(rootSignatureSpace.m_cbvSrvUavCount && (stateSpace.m_cbvSrvUavDirty || rootSignatureNeedsRebinding))
			{
				skip = false;
			}

			if(rootSignatureSpace.m_samplerCount && (stateSpace.m_samplersDirty || rootSignatureNeedsRebinding))
			{
				skip = false;
			}

			if(skip)
			{
				continue;
			}

			if(rootSignatureSpace.m_cbvSrvUavCount)
			{
				stateSpace.m_cbvSrvUavHeapHandle = DescriptorFactory::getSingleton().allocateTransient(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
																									   rootSignatureSpace.m_cbvSrvUavCount, true);
			}

			if(rootSignatureSpace.m_samplerCount)
			{
				stateSpace.m_samplerHeapHandle =
					DescriptorFactory::getSingleton().allocateTransient(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, rootSignatureSpace.m_samplerCount, true);
			}
		}

		// Populate descriptors

		DescriptorHeapHandle cbvSrvUavHeapOffset = stateSpace.m_cbvSrvUavHeapHandle;
		DescriptorHeapHandle samplerHeapOffset = stateSpace.m_samplerHeapHandle;

		for(HlslResourceType hlslResourceType : EnumIterable<HlslResourceType>())
		{
			for(U32 registerBinding = 0; registerBinding < rootSignatureSpace.m_descriptors[hlslResourceType].getSize(); ++registerBinding)
			{
				const RootSignature::Descriptor& inDescriptor = rootSignatureSpace.m_descriptors[hlslResourceType][registerBinding];

				if(inDescriptor.m_type == DescriptorType::kCount)
				{
					// Skip a hole
					continue;
				}

				const Descriptor& outDescriptor = stateSpace.m_descriptors[hlslResourceType][registerBinding];
				ANKI_ASSERT(
					(inDescriptor.m_type == outDescriptor.m_type
					 || (inDescriptor.m_type == DescriptorType::kSrvByteAddressBuffer && outDescriptor.m_type == DescriptorType::kSrvStructuredBuffer)
					 || (inDescriptor.m_type == DescriptorType::kUavByteAddressBuffer
						 && outDescriptor.m_type == DescriptorType::kUavStructuredBuffer))
					&& "Have bound the wrong thing");
				ANKI_ASSERT(descriptorTypeToHlslResourceType(inDescriptor.m_type) == hlslResourceType);

				if(inDescriptor.m_type == DescriptorType::kConstantBuffer)
				{
					// ConstantBuffer

					ANKI_ASSERT(!outDescriptor.m_isHandle);
					const BufferView& view = outDescriptor.m_bufferView;
					D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
					desc.BufferLocation = view.m_resource->GetGPUVirtualAddress() + view.m_offset;
					desc.SizeInBytes = U32(getAlignedRoundUp(256, view.m_range));

					getDevice().CreateConstantBufferView(&desc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else if(inDescriptor.m_type == DescriptorType::kSrvTexture || inDescriptor.m_type == DescriptorType::kUavTexture)
				{
					// Texture2D or RWTexture2D

					ANKI_ASSERT(outDescriptor.m_isHandle);
					getDevice().CopyDescriptorsSimple(1, cbvSrvUavHeapOffset.getCpuOffset(), outDescriptor.m_heapOffset,
													  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
				else if(inDescriptor.m_type == DescriptorType::kSampler)
				{
					// SamplerState

					ANKI_ASSERT(outDescriptor.m_isHandle);
					getDevice().CopyDescriptorsSimple(1, samplerHeapOffset.getCpuOffset(), outDescriptor.m_heapOffset,
													  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}
				else if(inDescriptor.m_type == DescriptorType::kUavByteAddressBuffer)
				{
					// RWByteAddressBuffer

					ANKI_ASSERT(!outDescriptor.m_isHandle);

					const BufferView& view = outDescriptor.m_bufferView;
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
					uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

					ANKI_ASSERT((view.m_offset % sizeof(U32)) == 0);
					uavDesc.Buffer.FirstElement = view.m_offset / sizeof(U32);

					ANKI_ASSERT((view.m_range % sizeof(U32)) == 0);
					uavDesc.Buffer.NumElements = U32(view.m_range / sizeof(U32));

					getDevice().CreateUnorderedAccessView(view.m_resource, nullptr, &uavDesc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else if(inDescriptor.m_type == DescriptorType::kUavStructuredBuffer)
				{
					// RWStructuredBuffer

					ANKI_ASSERT(!outDescriptor.m_isHandle);

					const BufferView& view = outDescriptor.m_bufferView;
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

					ANKI_ASSERT((view.m_offset % inDescriptor.m_structuredBufferStride) == 0);
					uavDesc.Buffer.FirstElement = view.m_offset / inDescriptor.m_structuredBufferStride;

					uavDesc.Buffer.NumElements = U32(view.m_range / inDescriptor.m_structuredBufferStride);
					ANKI_ASSERT(uavDesc.Buffer.NumElements > 0);

					uavDesc.Buffer.StructureByteStride = inDescriptor.m_structuredBufferStride;

					getDevice().CreateUnorderedAccessView(view.m_resource, nullptr, &uavDesc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else if(inDescriptor.m_type == DescriptorType::kSrvByteAddressBuffer)
				{
					// ByteAddressBuffer

					ANKI_ASSERT(!outDescriptor.m_isHandle);
					const BufferView& view = outDescriptor.m_bufferView;
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

					ANKI_ASSERT((view.m_offset % sizeof(U32)) == 0);
					srvDesc.Buffer.FirstElement = view.m_offset / sizeof(U32);

					ANKI_ASSERT((view.m_range % sizeof(U32)) == 0);
					srvDesc.Buffer.NumElements = U32(view.m_range / sizeof(U32));

					getDevice().CreateShaderResourceView(view.m_resource, &srvDesc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else if(inDescriptor.m_type == DescriptorType::kSrvStructuredBuffer)
				{
					// StructuredBuffer

					ANKI_ASSERT(!outDescriptor.m_isHandle);
					const BufferView& view = outDescriptor.m_bufferView;
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = DXGI_FORMAT_UNKNOWN;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

					ANKI_ASSERT((view.m_offset % inDescriptor.m_structuredBufferStride) == 0);
					srvDesc.Buffer.FirstElement = view.m_offset / inDescriptor.m_structuredBufferStride;

					srvDesc.Buffer.NumElements = U32(view.m_range / inDescriptor.m_structuredBufferStride);
					ANKI_ASSERT(srvDesc.Buffer.NumElements > 0);

					srvDesc.Buffer.StructureByteStride = inDescriptor.m_structuredBufferStride;

					getDevice().CreateShaderResourceView(view.m_resource, &srvDesc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else if(inDescriptor.m_type == DescriptorType::kUavTexelBuffer)
				{
					// RWBuffer

					ANKI_ASSERT(!outDescriptor.m_isHandle);
					const BufferView& view = outDescriptor.m_bufferView;
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.Format = convertFormat(view.m_format);
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

					const U32 texelSize = getFormatInfo(Format(view.m_format)).m_texelSize;
					ANKI_ASSERT((view.m_offset % texelSize) == 0);
					uavDesc.Buffer.FirstElement = view.m_offset / texelSize;

					ANKI_ASSERT((view.m_range % texelSize) == 0);
					uavDesc.Buffer.NumElements = U32(view.m_range / texelSize);

					getDevice().CreateUnorderedAccessView(view.m_resource, nullptr, &uavDesc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else if(inDescriptor.m_type == DescriptorType::kSrvTexelBuffer)
				{
					// Buffer

					ANKI_ASSERT(!outDescriptor.m_isHandle);
					const BufferView& view = outDescriptor.m_bufferView;
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

					srvDesc.Format = convertFormat(view.m_format);
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

					const U32 texelSize = getFormatInfo(Format(view.m_format)).m_texelSize;
					ANKI_ASSERT((view.m_offset % texelSize) == 0);
					srvDesc.Buffer.FirstElement = view.m_offset / texelSize;

					ANKI_ASSERT((view.m_range % texelSize) == 0);
					srvDesc.Buffer.NumElements = U32(view.m_range / texelSize);

					getDevice().CreateShaderResourceView(view.m_resource, &srvDesc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else if(inDescriptor.m_type == DescriptorType::kAccelerationStructure)
				{
					// AS

					ANKI_ASSERT(!outDescriptor.m_isHandle);
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.RaytracingAccelerationStructure.Location = outDescriptor.m_asAddress;

					getDevice().CreateShaderResourceView(nullptr, &srvDesc, cbvSrvUavHeapOffset.getCpuOffset());
				}
				else
				{
					ANKI_ASSERT(!"TODO");
				}

				if(hlslResourceType == HlslResourceType::kSampler)
				{
					samplerHeapOffset.increment(1);
				}
				else
				{
					cbvSrvUavHeapOffset.increment(1);
				}
			}
		}
	}

	// Bind root parameters
	for(U32 spaceIdx = 0; spaceIdx < kMaxRegisterSpaces; ++spaceIdx)
	{
		const RootSignature::Space& rootSignatureSpace = m_rootSignature->m_spaces[spaceIdx];
		if(rootSignatureSpace.isEmpty())
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
													  stateSpace.m_cbvSrvUavHeapHandle.getGpuOffset());
			}
			else
			{
				cmdList.SetGraphicsRootDescriptorTable(rootSignatureSpace.m_cbvSrvUavDescriptorTableRootParameterIdx,
													   stateSpace.m_cbvSrvUavHeapHandle.getGpuOffset());
			}
			stateSpace.m_cbvSrvUavDirty = false;
		}

		if(rootSignatureSpace.m_descriptors[HlslResourceType::kSampler].getSize() && (stateSpace.m_samplersDirty || rootSignatureNeedsRebinding))
		{
			ANKI_ASSERT(rootSignatureSpace.m_samplersDescriptorTableRootParameterIdx < kMaxU8);

			if(m_rootSignatureIsCompute)
			{
				cmdList.SetComputeRootDescriptorTable(rootSignatureSpace.m_samplersDescriptorTableRootParameterIdx,
													  stateSpace.m_samplerHeapHandle.getGpuOffset());
			}
			else
			{
				cmdList.SetGraphicsRootDescriptorTable(rootSignatureSpace.m_samplersDescriptorTableRootParameterIdx,
													   stateSpace.m_samplerHeapHandle.getGpuOffset());
			}
			stateSpace.m_samplersDirty = false;
		}
	}

	// Set root constants
	if(m_rootSignature->m_rootConstantsSize && (m_rootConstsDirty || rootSignatureNeedsRebinding))
	{
		ANKI_ASSERT(m_rootSignature->m_rootConstantsParameterIdx != kMaxU8);

		if(m_rootSignatureIsCompute)
		{
			cmdList.SetComputeRoot32BitConstants(m_rootSignature->m_rootConstantsParameterIdx, m_rootConstSize / sizeof(U32), m_rootConsts.getBegin(),
												 0);
		}
		else
		{
			cmdList.SetGraphicsRoot32BitConstants(m_rootSignature->m_rootConstantsParameterIdx, m_rootConstSize / sizeof(U32),
												  m_rootConsts.getBegin(), 0);
		}

		m_rootConstsDirty = false;
	}
}

} // end namespace anki
