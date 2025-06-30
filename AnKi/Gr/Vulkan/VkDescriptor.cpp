// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkDescriptor.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static StatCounter g_descriptorSetsAllocatedStatVar(StatCategory::kGr, "DescriptorSets allocated this frame", StatFlag::kZeroEveryFrame);
static StatCounter g_descriptorSetsWrittenStatVar(StatCategory::kGr, "DescriptorSets written this frame", StatFlag::kZeroEveryFrame);

/// Contains some constants. It's a class to avoid bugs initializing arrays (m_descriptorCount).
class DSAllocatorConstants
{
public:
	Array<std::pair<VkDescriptorType, U32>, 8> m_descriptorCount;

	U32 m_maxSets;

	DSAllocatorConstants()
	{
		m_descriptorCount[0] = {VK_DESCRIPTOR_TYPE_SAMPLER, 8};
		m_descriptorCount[1] = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 64};
		m_descriptorCount[2] = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 8};
		m_descriptorCount[3] = {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 32};
		m_descriptorCount[4] = {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 8};
		m_descriptorCount[5] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8};
		m_descriptorCount[6] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 64};
		m_descriptorCount[7] = {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 4};

		m_maxSets = 64;
	}
};

static const DSAllocatorConstants g_dsAllocatorConsts;

static U32 powu(U32 base, U32 exponent)
{
	U32 out = 1;
	for(U32 i = 1; i <= exponent; i++)
	{
		out *= base;
	}
	return out;
}

DescriptorAllocator::~DescriptorAllocator()
{
	destroy();
}

void DescriptorAllocator::createNewBlock()
{
	ANKI_TRACE_SCOPED_EVENT(GrDescriptorSetCreate);

	const Bool rtEnabled = GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled;

	Array<VkDescriptorPoolSize, g_dsAllocatorConsts.m_descriptorCount.getSize()> poolSizes;

	for(U32 i = 0; i < g_dsAllocatorConsts.m_descriptorCount.getSize(); ++i)
	{
		VkDescriptorPoolSize& size = poolSizes[i];

		size.descriptorCount = g_dsAllocatorConsts.m_descriptorCount[i].second * powu(kDescriptorSetGrowScale, m_blocks.getSize());
		size.type = g_dsAllocatorConsts.m_descriptorCount[i].first;
	}

	VkDescriptorPoolCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	inf.flags = 0;
	inf.maxSets = g_dsAllocatorConsts.m_maxSets * powu(kDescriptorSetGrowScale, m_blocks.getSize());
	ANKI_ASSERT(g_dsAllocatorConsts.m_descriptorCount.getBack().first == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
				&& "Needs to be the last for the bellow to work");
	inf.poolSizeCount = rtEnabled ? g_dsAllocatorConsts.m_descriptorCount.getSize() : g_dsAllocatorConsts.m_descriptorCount.getSize() - 1;
	inf.pPoolSizes = poolSizes.getBegin();

	VkDescriptorPool handle;
	ANKI_VK_CHECKF(vkCreateDescriptorPool(getVkDevice(), &inf, nullptr, &handle));

	Block& block = *m_blocks.emplaceBack();
	block.m_pool = handle;
	block.m_maxDsets = inf.maxSets;

	g_descriptorSetsAllocatedStatVar.increment(1);
}

void DescriptorAllocator::allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set)
{
	ANKI_TRACE_SCOPED_EVENT(GrAllocateDescriptorSet);

	// Lazy init
	if(m_blocks.getSize() == 0)
	{
		createNewBlock();
		ANKI_ASSERT(m_activeBlock == 0);
	}

	U32 iterationCount = 0;
	do
	{
		VkResult res;
		if(m_blocks[m_activeBlock].m_dsetsAllocatedCount > m_blocks[m_activeBlock].m_maxDsets * 2)
		{
			// The driver doesn't respect VkDescriptorPoolCreateInfo::maxSets. It should have thrown OoM already. To avoid growing the same DS forever
			// force OoM
			res = VK_ERROR_OUT_OF_POOL_MEMORY;
		}
		else
		{

			VkDescriptorSetAllocateInfo ci = {};
			ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			ci.descriptorPool = m_blocks[m_activeBlock].m_pool;
			ci.descriptorSetCount = 1;
			ci.pSetLayouts = &layout;

			res = vkAllocateDescriptorSets(getVkDevice(), &ci, &set);
		}

		if(res == VK_SUCCESS)
		{
			++m_blocks[m_activeBlock].m_dsetsAllocatedCount;
			break;
		}
		else if(res == VK_ERROR_OUT_OF_POOL_MEMORY)
		{
			++m_activeBlock;
			if(m_activeBlock >= m_blocks.getSize())
			{
				createNewBlock();
			}
		}
		else
		{
			ANKI_VK_CHECKF(res);
		}

		++iterationCount;
	} while(iterationCount < 10);

	if(iterationCount == 10)
	{
		ANKI_VK_LOGF("Failed to allocate descriptor set");
	}
}

void DescriptorAllocator::reset()
{
	// Trim blocks that were not used last time
	const U32 blocksInUse = m_activeBlock + 1;
	if(blocksInUse < m_blocks.getSize())
	{
		for(U32 i = blocksInUse; i < m_blocks.getSize(); ++i)
		{
			vkDestroyDescriptorPool(getVkDevice(), m_blocks[i].m_pool, nullptr);
		}

		m_blocks.resize(blocksInUse);
	}

	// Reset the remaining pools
	for(Block& b : m_blocks)
	{
		if(b.m_dsetsAllocatedCount > 0)
		{
			vkResetDescriptorPool(getVkDevice(), b.m_pool, 0);
		}
		b.m_dsetsAllocatedCount = 0;
	}

	m_activeBlock = 0;
}

BindlessDescriptorSet::~BindlessDescriptorSet()
{
	ANKI_ASSERT(m_freeTexIndexCount == m_freeTexIndices.getSize() && "Forgot to unbind some textures");

	if(m_dsPool)
	{
		vkDestroyDescriptorPool(getVkDevice(), m_dsPool, nullptr);
		m_dsPool = VK_NULL_HANDLE;
		m_dset = VK_NULL_HANDLE;
	}

	if(m_layout)
	{
		vkDestroyDescriptorSetLayout(getVkDevice(), m_layout, nullptr);
		m_layout = VK_NULL_HANDLE;
	}
}

Error BindlessDescriptorSet::init()
{
	const U32 bindlessTextureCount = g_maxBindlessSampledTextureCountCVar;

	// Create the layout
	{
		Array<VkDescriptorSetLayoutBinding, 1> bindings = {};
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
		bindings[0].descriptorCount = bindlessTextureCount;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

		Array<VkDescriptorBindingFlagsEXT, 1> bindingFlags = {};
		bindingFlags[0] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT
						  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extraInfos = {};
		extraInfos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		extraInfos.bindingCount = bindingFlags.getSize();
		extraInfos.pBindingFlags = &bindingFlags[0];

		VkDescriptorSetLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
		ci.bindingCount = bindings.getSize();
		ci.pBindings = &bindings[0];
		ci.pNext = &extraInfos;

		ANKI_VK_CHECK(vkCreateDescriptorSetLayout(getVkDevice(), &ci, nullptr, &m_layout));
	}

	// Create the pool
	{
		Array<VkDescriptorPoolSize, 1> sizes = {};
		sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		sizes[0].descriptorCount = bindlessTextureCount;

		VkDescriptorPoolCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		ci.maxSets = 1;
		ci.poolSizeCount = sizes.getSize();
		ci.pPoolSizes = &sizes[0];
		ci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;

		ANKI_VK_CHECK(vkCreateDescriptorPool(getVkDevice(), &ci, nullptr, &m_dsPool));
	}

	// Create the descriptor set
	{
		VkDescriptorSetAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ci.descriptorPool = m_dsPool;
		ci.descriptorSetCount = 1;
		ci.pSetLayouts = &m_layout;

		ANKI_VK_CHECK(vkAllocateDescriptorSets(getVkDevice(), &ci, &m_dset));
	}

	// Init the free arrays
	{
		m_freeTexIndices.resize(bindlessTextureCount);
		m_freeTexIndexCount = U16(m_freeTexIndices.getSize());

		for(U32 i = 0; i < m_freeTexIndices.getSize(); ++i)
		{
			m_freeTexIndices[i] = U16(m_freeTexIndices.getSize() - i - 1);
		}
	}

	return Error::kNone;
}

U32 BindlessDescriptorSet::bindTexture(const VkImageView view, const VkImageLayout layout)
{
	ANKI_ASSERT(layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	ANKI_ASSERT(view);
	LockGuard<Mutex> lock(m_mtx);
	ANKI_ASSERT(m_freeTexIndexCount > 0 && "Out of indices");

	// Pop the index
	--m_freeTexIndexCount;
	const U16 idx = m_freeTexIndices[m_freeTexIndexCount];
	ANKI_ASSERT(idx < m_freeTexIndices.getSize());

	// Update the set
	VkDescriptorImageInfo imageInf = {};
	imageInf.imageView = view;
	imageInf.imageLayout = layout;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = m_dset;
	write.dstBinding = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	write.dstArrayElement = idx;
	write.pImageInfo = &imageInf;

	vkUpdateDescriptorSets(getVkDevice(), 1, &write, 0, nullptr);

	return idx;
}

void BindlessDescriptorSet::unbindTexture(U32 idx)
{
	LockGuard<Mutex> lock(m_mtx);

	ANKI_ASSERT(idx < m_freeTexIndices.getSize());
	ANKI_ASSERT(m_freeTexIndexCount < m_freeTexIndices.getSize());

	m_freeTexIndices[m_freeTexIndexCount] = U16(idx);
	++m_freeTexIndexCount;

	// Sort the free indices to minimize fragmentation
	std::sort(&m_freeTexIndices[0], &m_freeTexIndices[0] + m_freeTexIndexCount, std::greater<U16>());

	// Make sure there are no duplicates
	for(U32 i = 1; i < m_freeTexIndexCount; ++i)
	{
		ANKI_ASSERT(m_freeTexIndices[i] != m_freeTexIndices[i - 1]);
	}
}

PipelineLayoutFactory2::~PipelineLayoutFactory2()
{
	for(auto it : m_pplLayouts)
	{
		vkDestroyPipelineLayout(getVkDevice(), it->m_handle, nullptr);
		deleteInstance(GrMemoryPool::getSingleton(), it);
	}

	for(auto it : m_dsLayouts)
	{
		vkDestroyDescriptorSetLayout(getVkDevice(), it->m_handle, nullptr);
		deleteInstance(GrMemoryPool::getSingleton(), it);
	}
}

Error PipelineLayoutFactory2::getOrCreateDescriptorSetLayout(ConstWeakArray<ShaderReflectionBinding> reflBindings, DescriptorSetLayout*& layout)
{
	ANKI_BEGIN_PACKED_STRUCT
	class DSBinding
	{
	public:
		VkDescriptorType m_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		U16 m_arraySize = 0;
		U16 m_binding = kMaxU8;
	};
	ANKI_END_PACKED_STRUCT

	// Compute the hash for the layout
	Array<DSBinding, kMaxBindingsPerRegisterSpace> bindings;
	U64 hash;

	if(reflBindings.getSize())
	{
		// Copy to a new place because we want to sort
		U32 count = 0;
		for(const ShaderReflectionBinding& reflBinding : reflBindings)
		{
			bindings[count].m_type = convertDescriptorType(reflBinding.m_type);
			bindings[count].m_arraySize = reflBinding.m_arraySize;
			bindings[count].m_binding = reflBinding.m_vkBinding;
			++count;
		}

		std::sort(bindings.getBegin(), bindings.getBegin() + count, [](const DSBinding& a, const DSBinding& b) {
			return a.m_binding < b.m_binding;
		});

		hash = computeHash(&bindings[0], sizeof(bindings[0]) * count);
		ANKI_ASSERT(hash != 1);
	}
	else
	{
		hash = 1;
	}

	// Search the cache or create it
	auto it = m_dsLayouts.find(hash);
	if(it != m_dsLayouts.getEnd())
	{
		layout = *it;
	}
	else
	{
		const U32 bindingCount = reflBindings.getSize();

		layout = newInstance<DescriptorSetLayout>(GrMemoryPool::getSingleton());
		m_dsLayouts.emplace(hash, layout);

		Array<VkDescriptorSetLayoutBinding, kMaxBindingsPerRegisterSpace> vkBindings;
		VkDescriptorSetLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		for(U i = 0; i < bindingCount; ++i)
		{
			VkDescriptorSetLayoutBinding& vk = vkBindings[i];
			const DSBinding& ak = bindings[i];

			vk.binding = ak.m_binding;
			vk.descriptorCount = ak.m_arraySize;
			vk.descriptorType = ak.m_type;
			vk.pImmutableSamplers = nullptr;
			vk.stageFlags = VK_SHADER_STAGE_ALL;
		}

		ci.bindingCount = bindingCount;
		ci.pBindings = &vkBindings[0];

		ANKI_VK_CHECK(vkCreateDescriptorSetLayout(getVkDevice(), &ci, nullptr, &layout->m_handle));
	}

	return Error::kNone;
}

Error PipelineLayoutFactory2::getOrCreatePipelineLayout(const ShaderReflectionDescriptorRelated& refl, PipelineLayout2*& layout)
{
	// Compute the hash
	const U64 hash = computeHash(&refl, sizeof(refl));

	LockGuard lock(m_mtx);

	auto it = m_pplLayouts.find(hash);
	if(it != m_pplLayouts.getEnd())
	{
		layout = *it;
	}
	else
	{
		// Create new

		layout = newInstance<PipelineLayout2>(GrMemoryPool::getSingleton());
		m_pplLayouts.emplace(hash, layout);
		layout->m_refl = refl;

		// Find dset count
		layout->m_dsetCount = 0;
		for(U8 iset = 0; iset < kMaxRegisterSpaces; ++iset)
		{
			if(refl.m_bindingCounts[iset])
			{
				layout->m_dsetCount = max<U8>(iset + 1u, layout->m_dsetCount);

				for(U32 i = 0; i < refl.m_bindingCounts[iset]; ++i)
				{
					layout->m_descriptorCounts[iset] += refl.m_bindings[iset][i].m_arraySize;
				}
			}
		}

		if(refl.m_vkBindlessDescriptorSet != kMaxU8)
		{
			layout->m_dsetCount = max<U8>(refl.m_vkBindlessDescriptorSet + 1, layout->m_dsetCount);
		}

		// Create the DS layouts
		for(U32 iset = 0; iset < layout->m_dsetCount; ++iset)
		{
			if(refl.m_vkBindlessDescriptorSet == iset)
			{
				layout->m_dsetLayouts[iset] = BindlessDescriptorSet::getSingleton().m_layout;
			}
			else
			{
				DescriptorSetLayout* dlayout;
				ANKI_CHECK(getOrCreateDescriptorSetLayout({refl.m_bindings[iset].getBegin(), refl.m_bindingCounts[iset]}, dlayout));

				layout->m_dsetLayouts[iset] = dlayout->m_handle;
			}
		}

		VkPipelineLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		ci.pSetLayouts = &layout->m_dsetLayouts[0];
		ci.setLayoutCount = layout->m_dsetCount;

		VkPushConstantRange pushConstantRange;
		if(refl.m_fastConstantsSize > 0)
		{
			pushConstantRange.offset = 0;
			pushConstantRange.size = refl.m_fastConstantsSize;
			pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
			ci.pushConstantRangeCount = 1;
			ci.pPushConstantRanges = &pushConstantRange;
		}

		ANKI_VK_CHECK(vkCreatePipelineLayout(getVkDevice(), &ci, nullptr, &layout->m_handle));
	}

	return Error::kNone;
}

void DescriptorState::flush(VkCommandBuffer cmdb, DescriptorAllocator& dalloc)
{
	ANKI_ASSERT(m_pipelineLayout);

	const ShaderReflectionDescriptorRelated& refl = m_pipelineLayout->getReflection();

	// Small opt to bind the high frequency sets as little as possible
	BitSet<kMaxRegisterSpaces> dirtySets(false);

	for(U32 iset = 0; iset < m_pipelineLayout->m_dsetCount; ++iset)
	{
		DescriptorSet& set = m_sets[iset];

		if(iset == refl.m_vkBindlessDescriptorSet)
		{
			if(m_vkDsets[iset] != BindlessDescriptorSet::getSingleton().m_dset)
			{
				dirtySets.set(iset);

				m_vkDsets[iset] = BindlessDescriptorSet::getSingleton().m_dset;
			}
		}
		else if(m_sets[iset].m_dirty)
		{
			// Need to allocate and populate a new DS

			dirtySets.set(iset);

			set.m_dirty = false;

			if(set.m_writeInfos.getSize() < m_pipelineLayout->m_descriptorCounts[iset])
			{
				set.m_writeInfos.resize(m_pipelineLayout->m_descriptorCounts[iset]);
			}

			dalloc.allocate(m_pipelineLayout->m_dsetLayouts[iset], m_vkDsets[iset]);

			// Write the DS
			VkWriteDescriptorSet writeTemplate = {};
			writeTemplate.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeTemplate.pNext = nullptr;
			writeTemplate.dstSet = m_vkDsets[iset];
			writeTemplate.descriptorCount = 1;

			U32 writeInfoCount = 0;

			for(U32 ibinding = 0; ibinding < refl.m_bindingCounts[iset]; ++ibinding)
			{
				const ShaderReflectionBinding& binding = refl.m_bindings[iset][ibinding];
				for(U32 arrayIdx = 0; arrayIdx < binding.m_arraySize; ++arrayIdx)
				{
					VkWriteDescriptorSet& writeInfo = set.m_writeInfos[writeInfoCount++];
					const HlslResourceType hlslType = descriptorTypeToHlslResourceType(binding.m_type);
					ANKI_ASSERT(binding.m_registerBindingPoint + arrayIdx < set.m_descriptors[hlslType].getSize() && "Forgot to bind something");
					const Descriptor& desc = set.m_descriptors[hlslType][binding.m_registerBindingPoint + arrayIdx];

					ANKI_ASSERT(desc.m_type == binding.m_type && "Have bound the wrong type");

					writeInfo = writeTemplate;
					writeInfo.descriptorType = convertDescriptorType(binding.m_type);
					writeInfo.dstArrayElement = arrayIdx;
					writeInfo.dstBinding = binding.m_vkBinding;

					switch(writeInfo.descriptorType)
					{
					case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
					case VK_DESCRIPTOR_TYPE_SAMPLER:
					case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
					{
						writeInfo.pImageInfo = &desc.m_image;
						break;
					}
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					{
						writeInfo.pBufferInfo = &desc.m_buffer;
						break;
					}
					case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
					{
						writeInfo.pTexelBufferView = &desc.m_bufferView;
						break;
					}
					case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
					{
						writeInfo.pNext = &desc.m_as;
						break;
					}
					default:
						ANKI_ASSERT(0);
					}
				}
			}

			if(writeInfoCount > 0)
			{
				vkUpdateDescriptorSets(getVkDevice(), writeInfoCount, set.m_writeInfos.getBegin(), 0, nullptr);
				g_descriptorSetsWrittenStatVar.increment(1);
			}
		}
		else
		{
			// Do nothing
		}

		ANKI_ASSERT(m_vkDsets[iset] != VK_NULL_HANDLE);
	}

	// Bind the descriptor sets
	if(dirtySets.getAnySet())
	{
		const U32 minSetThatNeedsRebind = dirtySets.getLeastSignificantBit();
		const U32 dsetCount = m_pipelineLayout->m_dsetCount - minSetThatNeedsRebind;
		ANKI_ASSERT(dsetCount <= m_pipelineLayout->m_dsetCount);
		vkCmdBindDescriptorSets(cmdb, m_pipelineBindPoint, m_pipelineLayout->m_handle, minSetThatNeedsRebind, dsetCount,
								&m_vkDsets[minSetThatNeedsRebind], 0, nullptr);
	}

	// Set push consts
	if(refl.m_fastConstantsSize)
	{
		ANKI_ASSERT(refl.m_fastConstantsSize == m_pushConstSize && "Possibly forgot to set push constants");

		if(m_pushConstantsDirty)
		{
			vkCmdPushConstants(cmdb, m_pipelineLayout->m_handle, VK_SHADER_STAGE_ALL, 0, m_pushConstSize, m_pushConsts.getBegin());
			m_pushConstantsDirty = false;
		}
	}
}

} // end namespace anki
