// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkDescriptorSetFactory.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

static StatCounter g_descriptorSetsAllocated(StatCategory::kMisc, "DescriptorSets allocated this frame", StatFlag::kZeroEveryFrame);

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

DSAllocator::~DSAllocator()
{
	destroy();
}

void DSAllocator::createNewBlock()
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
	static_assert(DescriptorType::kAccelerationStructure == DescriptorType::kCount - 1, "Needs to be the last for the bellow to work");
	inf.poolSizeCount = rtEnabled ? U32(DescriptorType::kCount) : U32(DescriptorType::kCount) - 1;
	inf.pPoolSizes = poolSizes.getBegin();

	VkDescriptorPool handle;
	ANKI_VK_CHECKF(vkCreateDescriptorPool(getVkDevice(), &inf, nullptr, &handle));

	Block& block = *m_blocks.emplaceBack();
	block.m_pool = handle;

	g_descriptorSetsAllocated.increment(1);
}

void DSAllocator::allocate(const DSLayout& layout, VkDescriptorSet& set)
{
	ANKI_TRACE_SCOPED_EVENT(GrAllocateDescriptorSet);

	ANKI_ASSERT(layout.m_hash != 0 && "Can't allocate using the bindless DS layout");

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
		if(m_blocks[m_activeBlock].m_dsetsAllocatedCount > g_dsAllocatorConsts.m_maxSets * powu(kDescriptorSetGrowScale, m_activeBlock) * 2)
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
			ci.pSetLayouts = &layout.getHandle();

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

void DSAllocator::reset()
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
		vkResetDescriptorPool(getVkDevice(), b.m_pool, 0);
		b.m_dsetsAllocatedCount = 0;
	}

	m_activeBlock = 0;
}

DSBindless::~DSBindless()
{
	ANKI_ASSERT(m_freeTexIndexCount == m_freeTexIndices.getSize() && "Forgot to unbind some textures");
	ANKI_ASSERT(m_freeTexelBufferIndexCount == m_freeTexelBufferIndices.getSize() && "Forgot to unbind some texel buffers");

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

Error DSBindless::init(U32 bindlessTextureCount, U32 bindlessTextureBuffers)
{
	// Create the layout
	{
		Array<VkDescriptorSetLayoutBinding, 2> bindings = {};
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
		bindings[0].descriptorCount = bindlessTextureCount;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
		bindings[1].descriptorCount = bindlessTextureBuffers;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;

		Array<VkDescriptorBindingFlagsEXT, 2> bindingFlags = {};
		bindingFlags[0] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT
						  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
		bindingFlags[1] = bindingFlags[0];

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
		Array<VkDescriptorPoolSize, 2> sizes = {};
		sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		sizes[0].descriptorCount = bindlessTextureCount;
		sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		sizes[1].descriptorCount = bindlessTextureBuffers;

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

		m_freeTexelBufferIndices.resize(bindlessTextureBuffers);
		m_freeTexelBufferIndexCount = U16(m_freeTexelBufferIndices.getSize());

		for(U32 i = 0; i < m_freeTexelBufferIndices.getSize(); ++i)
		{
			m_freeTexelBufferIndices[i] = U16(m_freeTexelBufferIndices.getSize() - i - 1);
		}
	}

	return Error::kNone;
}

U32 DSBindless::bindTexture(const VkImageView view, const VkImageLayout layout)
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

U32 DSBindless::bindUniformTexelBuffer(VkBufferView view)
{
	ANKI_ASSERT(view);
	LockGuard<Mutex> lock(m_mtx);
	ANKI_ASSERT(m_freeTexelBufferIndexCount > 0 && "Out of indices");

	// Pop the index
	--m_freeTexelBufferIndexCount;
	const U16 idx = m_freeTexelBufferIndices[m_freeTexelBufferIndexCount];
	ANKI_ASSERT(idx < m_freeTexelBufferIndices.getSize());

	// Update the set
	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = m_dset;
	write.dstBinding = 1;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	write.dstArrayElement = idx;
	write.pTexelBufferView = &view;

	vkUpdateDescriptorSets(getVkDevice(), 1, &write, 0, nullptr);

	return idx;
}

void DSBindless::unbindCommon(U32 idx, GrDynamicArray<U16>& freeIndices, U16& freeIndexCount)
{
	LockGuard<Mutex> lock(m_mtx);

	ANKI_ASSERT(idx < freeIndices.getSize());
	ANKI_ASSERT(freeIndexCount < freeIndices.getSize());

	freeIndices[freeIndexCount] = U16(idx);
	++freeIndexCount;

	// Sort the free indices to minimize fragmentation
	std::sort(&freeIndices[0], &freeIndices[0] + freeIndexCount, std::greater<U16>());

	// Make sure there are no duplicates
	for(U32 i = 1; i < freeIndexCount; ++i)
	{
		ANKI_ASSERT(freeIndices[i] != freeIndices[i - 1]);
	}
}

void DSStateTracker::setBinding(U32 bindingIdx, U32 arrayIdx, const Binding& b)
{
	ANKI_ASSERT(bindingIdx < kMaxBindingsPerDescriptorSet);

	BindingExtended& extended = m_bindings[bindingIdx];
	Binding* out;

	const Bool extendedInitializedBefore = m_bindingIsSetMask.get(bindingIdx);

	if(!extendedInitializedBefore)
	{
		m_bindingIsSetMask.set(bindingIdx);
		zeroMemory(extended.m_single);
		extended.m_count = 1;
	}

	if(arrayIdx == 0) [[likely]]
	{
		// Array idx is zero, most common case
		out = (extended.m_count == 1) ? &extended.m_single : &extended.m_array[0];
	}
	else if(arrayIdx < extended.m_count)
	{
		// It's (or was) an array and there enough space in thar array
		ANKI_ASSERT(arrayIdx > 0);
		out = &extended.m_array[arrayIdx];
	}
	else
	{
		// Need to grow
		ANKI_ASSERT(arrayIdx > 0);
		const U32 newSize = max(extended.m_count * 2, arrayIdx + 1);
		Binding* newArr = newArray<Binding>(m_writeInfos.getMemoryPool(), newSize);

		if(!extendedInitializedBefore)
		{
			memset(newArr, 0, newSize * sizeof(Binding));
		}
		else if(extended.m_count == 1)
		{
			newArr[0] = extended.m_single;
		}
		else
		{
			// Copy old to new
			memcpy(newArr, extended.m_array, sizeof(Binding) * extended.m_count);

			// Zero the rest
			memset(newArr + extended.m_count, 0, sizeof(Binding) * (newSize - extended.m_count));
		}

		extended.m_count = newSize;
		extended.m_array = newArr;

		out = &extended.m_array[arrayIdx];
	}

	unbindBindlessDSet();
	m_bindingDirtyMask.set(bindingIdx);
	*out = b;
}

Bool DSStateTracker::flush(DSAllocator& allocator, VkDescriptorSet& dsHandle)
{
	ANKI_TRACE_SCOPED_EVENT(GrFlushDescrStateTracker);
	ANKI_ASSERT(m_layout);

	dsHandle = VK_NULL_HANDLE;

	if(m_bindlessDSBound)
	{
		ANKI_ASSERT((m_layout->m_hash == 0 || m_layout->m_descriptorCount == 0)
					&& "Need to have bound the bindless layout or a layout with no descriptors");
	}

	const Bool reallyBindless = m_bindlessDSBound && m_layout->m_hash == 0;
	if(reallyBindless)
	{
		if(m_bindlessDSDirty || m_layoutDirty)
		{
			dsHandle = DSBindless::getSingleton().m_dset;
			m_bindlessDSDirty = false;
			m_layoutDirty = false;
		}
		return dsHandle != VK_NULL_HANDLE;
	}

	const Bool dirty = m_layoutDirty || (m_layout->m_activeBindings & m_bindingDirtyMask);
	if(!dirty)
	{
		return false;
	}

	m_layoutDirty = false;
	m_bindingDirtyMask = m_bindingDirtyMask & ~m_layout->m_activeBindings;

	if(m_writeInfos.getSize() < m_layout->m_descriptorCount)
	{
		m_writeInfos.resize(m_layout->m_descriptorCount);
	}

	// Allocate the DS
	allocator.allocate(*m_layout, dsHandle);

	// Write the DS
	VkWriteDescriptorSet writeTemplate = {};
	writeTemplate.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeTemplate.pNext = nullptr;
	writeTemplate.dstSet = dsHandle;
	writeTemplate.descriptorCount = 1;

	U32 writeInfoIdx = 0;
	for(U32 bindingIdx = m_layout->m_minBinding; bindingIdx <= m_layout->m_maxBinding; ++bindingIdx)
	{
		if(m_layout->m_activeBindings.get(bindingIdx))
		{
			for(U32 arrIdx = 0; arrIdx < m_layout->m_bindingArraySize[bindingIdx]; ++arrIdx)
			{
				VkWriteDescriptorSet& writeInfo = m_writeInfos[writeInfoIdx++];
				ANKI_ASSERT(m_bindings[bindingIdx].m_count < kMaxU32);
				const Binding& b = (m_bindings[bindingIdx].m_count == 1) ? m_bindings[bindingIdx].m_single : m_bindings[bindingIdx].m_array[arrIdx];

				ANKI_ASSERT(b.m_type == m_layout->m_bindingDsType[bindingIdx] && "Bound the wrong type");

				writeInfo = writeTemplate;
				writeInfo.descriptorType = b.m_type;
				writeInfo.dstArrayElement = arrIdx;
				writeInfo.dstBinding = bindingIdx;

				switch(b.m_type)
				{
				case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				case VK_DESCRIPTOR_TYPE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				{
					writeInfo.pImageInfo = &b.m_image;
					break;
				}
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				{
					writeInfo.pBufferInfo = &b.m_buffer;
					break;
				}
				case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				{
					writeInfo.pTexelBufferView = &b.m_bufferView;
					break;
				}
				case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
				{
					writeInfo.pNext = &b.m_as;
					break;
				}
				default:
					ANKI_ASSERT(0);
				}
			}
		}
	}

	if(writeInfoIdx > 0)
	{
		vkUpdateDescriptorSets(getVkDevice(), writeInfoIdx, m_writeInfos.getBegin(), 0, nullptr);
	}

	return true;
}

DSLayoutFactory::DSLayoutFactory()
{
	DSLayout* pLayout = newInstance<DSLayout>(GrMemoryPool::getSingleton());
	pLayout->m_handle = DSBindless::getSingleton().m_layout;
	pLayout->m_hash = 0;
	// The rest we don't care

	m_layouts.emplaceBack(pLayout);
}

DSLayoutFactory::~DSLayoutFactory()
{
	for(DSLayout* l : m_layouts)
	{
		if(l->m_hash != 0)
		{
			vkDestroyDescriptorSetLayout(getVkDevice(), l->m_handle, nullptr);
		}

		deleteInstance(GrMemoryPool::getSingleton(), l);
	}
}

Error DSLayoutFactory::getOrCreateDescriptorSetLayout(const WeakArray<DSBinding>& bindings_, const DSLayout*& layout_)
{
	DSLayout* layout = nullptr;

	// Compute the hash for the layout
	Array<DSBinding, kMaxBindingsPerDescriptorSet> bindings;
	const U32 bindingCount = bindings_.getSize();
	U64 hash;

	if(bindingCount > 0)
	{
		// Copy to a new place because we want to sort
		memcpy(bindings.getBegin(), bindings_.getBegin(), bindings_.getSizeInBytes());
		std::sort(bindings.getBegin(), bindings.getBegin() + bindingCount, [](const DSBinding& a, const DSBinding& b) {
			return a.m_binding < b.m_binding;
		});

		hash = computeHash(&bindings[0], bindings_.getSizeInBytes());
		ANKI_ASSERT(hash != 1);
	}
	else
	{
		hash = 1;
	}

	ANKI_ASSERT(hash != 0);

	// Identify if the DS is the bindless one. It is if there is at least one binding that matches the criteria
	Bool isBindless = false;
	if(bindingCount > 0)
	{
		isBindless = true;
		for(U32 i = 0; i < bindingCount; ++i)
		{
			const DSBinding& binding = bindings[i];
			if(binding.m_binding == 0 && binding.m_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
			   && binding.m_arraySize == DSBindless::getSingleton().getMaxTextureCount())
			{
				// All good
			}
			else if(binding.m_binding == 1 && binding.m_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
					&& binding.m_arraySize == DSBindless::getSingleton().getMaxTexelBufferCount())
			{
				// All good
			}
			else
			{
				isBindless = false;
				break;
			}
		}
	}

	LockGuard lock(m_mtx);

	if(isBindless)
	{
		layout = *m_layouts.getBegin();
	}
	else
	{
		// Search the cache or create it

		for(DSLayout* it : m_layouts)
		{
			if(it->m_hash == hash)
			{
				layout = it;
				break;
			}
		}

		if(layout == nullptr)
		{
			// Create it

			layout = newInstance<DSLayout>(GrMemoryPool::getSingleton());
			m_layouts.emplaceBack(layout);
			layout->m_hash = hash;

			Array<VkDescriptorSetLayoutBinding, kMaxBindingsPerDescriptorSet> vkBindings;
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

				ANKI_ASSERT(layout->m_activeBindings.get(ak.m_binding) == false);
				layout->m_activeBindings.set(ak.m_binding);
				layout->m_bindingDsType[ak.m_binding] = ak.m_type;
				layout->m_bindingArraySize[ak.m_binding] = ak.m_arraySize;
				layout->m_minBinding = min<U32>(layout->m_minBinding, ak.m_binding);
				layout->m_maxBinding = max<U32>(layout->m_maxBinding, ak.m_binding);
				layout->m_descriptorCount += ak.m_arraySize;
			}

			ci.bindingCount = bindingCount;
			ci.pBindings = &vkBindings[0];

			ANKI_VK_CHECK(vkCreateDescriptorSetLayout(getVkDevice(), &ci, nullptr, &layout->m_handle));
		}
	}

	layout_ = layout;

	return Error::kNone;
}

} // end namespace anki
