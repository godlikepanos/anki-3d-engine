// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DDescriptor.h>

namespace anki {

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

	const U32 spaceCount = (refl.m_descriptorSetMask.getMostSignificantBit() < kMaxU32) ? refl.m_descriptorSetMask.getMostSignificantBit() + 1 : 0;
	GrDynamicArray<D3D12_ROOT_PARAMETER> rootParameters;

	// Create the tables
	Array<GrDynamicArray<D3D12_DESCRIPTOR_RANGE>, kMaxBindingsPerDescriptorSet * 2> tableRanges; // One per descriptor table
	U32 tableRangesCount = 0;
	for(U32 space = 0; space < spaceCount; ++space)
	{
		GrDynamicArray<D3D12_DESCRIPTOR_RANGE>& nonSamplerRanges = tableRanges[tableRangesCount++];
		GrDynamicArray<D3D12_DESCRIPTOR_RANGE>& samplerRanges = tableRanges[tableRangesCount++];

		// Create the ranges
		for(U32 bindingIdx = 0; bindingIdx < refl.m_bindingCounts[space]; ++bindingIdx)
		{
			const ShaderReflectionBinding& akBinding = refl.m_bindings[space][bindingIdx];
			akBinding.validate();
			D3D12_DESCRIPTOR_RANGE& range =
				(akBinding.m_type == DescriptorType::kSampler) ? *samplerRanges.emplaceBack() : *nonSamplerRanges.emplaceBack();

			range.NumDescriptors = akBinding.m_arraySize;
			range.BaseShaderRegister = akBinding.m_semantic.m_number;
			range.RegisterSpace = space;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			if(akBinding.m_type == DescriptorType::kUniformBuffer)
			{
				ANKI_ASSERT(akBinding.m_semantic.m_class == 'b');
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			}
			else if(akBinding.m_type == DescriptorType::kSampler)
			{
				ANKI_ASSERT(akBinding.m_semantic.m_class == 's');
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			}
			else if(!(akBinding.m_flags & DescriptorFlag::kWrite))
			{
				ANKI_ASSERT(akBinding.m_semantic.m_class == 't');
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			}
			else
			{
				ANKI_ASSERT(akBinding.m_semantic.m_class == 'u');
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			}
		}

		if(nonSamplerRanges.getSize())
		{
			D3D12_ROOT_PARAMETER& table = *rootParameters.emplaceBack();
			table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			table.DescriptorTable.NumDescriptorRanges = nonSamplerRanges.getSize();
			table.DescriptorTable.pDescriptorRanges = nonSamplerRanges.getBegin();
		}

		if(samplerRanges.getSize())
		{
			D3D12_ROOT_PARAMETER& table = *rootParameters.emplaceBack();
			table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			table.DescriptorTable.NumDescriptorRanges = samplerRanges.getSize();
			table.DescriptorTable.pDescriptorRanges = samplerRanges.getBegin();
		}
	}

	// Root constants
	if(refl.m_pushConstantsSize)
	{
		D3D12_ROOT_PARAMETER& rootParam = *rootParameters.emplaceBack();
		rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		ANKI_ASSERT((refl.m_pushConstantsSize % 4) == 0);
		rootParam.Constants.Num32BitValues = refl.m_pushConstantsSize / 4;
		rootParam.Constants.RegisterSpace = ANKI_PUSH_CONSTANTS_D3D_BINDING;
		rootParam.Constants.ShaderRegister = ANKI_PUSH_CONSTANTS_D3D_SPACE;
	}

	D3D12_ROOT_SIGNATURE_DESC sigDesc = {};

	return Error::kNone;
}

} // end namespace anki
