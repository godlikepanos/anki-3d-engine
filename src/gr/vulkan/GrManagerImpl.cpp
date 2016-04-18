// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/Pipeline.h>
#include <anki/util/HashMap.h>
#include <anki/util/Hash.h>

namespace anki
{

//==============================================================================
// GrManagerImpl::CompatibleRenderPassHashMap                                  =
//==============================================================================

class RenderPassKey
{
public:
	Array<PixelFormat, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	PixelFormat m_depthStencilAttachment;

	RenderPassKey()
	{
		// Zero because we compute hashes
		memset(this, 0, sizeof(*this));
	}

	RenderPassKey& operator=(const RenderPassKey& b) = default;
};

class RenderPassHasher
{
public:
	U64 operator()(const RenderPassKey& b) const
	{
		return computeHash(&b, sizeof(b));
	}
};

class RenderPassCompare
{
public:
	Bool operator()(const RenderPassKey& a, const RenderPassKey& b) const
	{
		for(U i = 0; i < a.m_colorAttachments.getSize(); ++i)
		{
			if(a.m_colorAttachments[i] != b.m_colorAttachments[i])
			{
				return false;
			}
		}

		return a.m_depthStencilAttachment == b.m_depthStencilAttachment;
	}
};

class GrManagerImpl::CompatibleRenderPassHashMap
{
public:
	Mutex m_mtx;
	HashMap<RenderPassKey, VkRenderPass, RenderPassHasher, RenderPassCompare>
		m_hashmap;
};

//==============================================================================
// GrManagerImpl                                                               =
//==============================================================================

//==============================================================================
Error GrManagerImpl::init()
{
	initGlobalDsetLayout();
	initGlobalPplineLayout();
	initMemory();

	return ErrorCode::NONE;
}

//==============================================================================
void GrManagerImpl::initGlobalDsetLayout()
{
	VkDescriptorSetLayoutCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;

	const U BINDING_COUNT = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS
		+ MAX_STORAGE_BUFFER_BINDINGS;
	ci.bindingCount = BINDING_COUNT;

	Array<VkDescriptorSetLayoutBinding, BINDING_COUNT> bindings;
	ci.pBindings = &bindings[0];

	U count = 0;

	// Combined image samplers
	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[count];
		binding.binding = count;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL;
		binding.pImmutableSamplers = nullptr;

		++count;
	}

	// Uniform buffers
	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[count];
		binding.binding = count;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL;
		binding.pImmutableSamplers = nullptr;

		++count;
	}

	// Storage buffers
	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[count];
		binding.binding = count;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL;
		binding.pImmutableSamplers = nullptr;

		++count;
	}

	ANKI_ASSERT(count == BINDING_COUNT);

	ANKI_VK_CHECK(vkCreateDescriptorSetLayout(
		m_device, &ci, nullptr, &m_globalDescriptorSetLayout));
}

//==============================================================================
void GrManagerImpl::initGlobalPplineLayout()
{
	Array<VkDescriptorSetLayout, MAX_RESOURCE_GROUPS> sets = {
		{m_globalDescriptorSetLayout, m_globalDescriptorSetLayout}};

	VkPipelineLayoutCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.setLayoutCount = MAX_RESOURCE_GROUPS;
	ci.pSetLayouts = &sets[0];
	ci.pushConstantRangeCount = 0;
	ci.pPushConstantRanges = nullptr;

	ANKI_VK_CHECK(vkCreatePipelineLayout(
		m_device, &ci, nullptr, &m_globalPipelineLayout));
}

//==============================================================================
VkRenderPass GrManagerImpl::getOrCreateCompatibleRenderPass(
	const PipelineInitInfo& init)
{
	VkRenderPass out;
	// Create the key
	RenderPassKey key;
	for(U i = 0; i < init.m_color.m_attachmentCount; ++i)
	{
		key.m_colorAttachments[i] = init.m_color.m_attachments[i].m_format;
	}
	key.m_depthStencilAttachment = init.m_depthStencil.m_format;

	// Lock
	LockGuard<Mutex> lock(m_renderPasses->m_mtx);

	auto it = m_renderPasses->m_hashmap.find(key);
	if(it != m_renderPasses->m_hashmap.getEnd())
	{
		// Found the key
		out = *it;
	}
	else
	{
		// Not found, create one
		VkRenderPassCreateInfo ci;
		ANKI_VK_MEMSET_DBG(ci);
		ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;

		Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1>
			attachmentDescriptions;
		Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS> references;
		for(U i = 0; i < init.m_color.m_attachmentCount; ++i)
		{
			// We only care about samples and format
			VkAttachmentDescription& desc = attachmentDescriptions[i];
			ANKI_VK_MEMSET_DBG(desc);
			desc.flags = 0;
			desc.format = convertFormat(init.m_color.m_attachments[i].m_format);
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			references[i].attachment = i;
			references[i].layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		ci.attachmentCount = init.m_color.m_attachmentCount;

		Bool hasDepthStencil =
			init.m_depthStencil.m_format.m_components != ComponentFormat::NONE;
		VkAttachmentReference dsReference = {0, VK_IMAGE_LAYOUT_UNDEFINED};
		if(hasDepthStencil)
		{
			VkAttachmentDescription& desc =
				attachmentDescriptions[ci.attachmentCount];
			ANKI_VK_MEMSET_DBG(desc);
			desc.flags = 0;
			desc.format = convertFormat(init.m_depthStencil.m_format);
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			dsReference.attachment = ci.attachmentCount;
			dsReference.layout = VK_IMAGE_LAYOUT_UNDEFINED;

			++ci.attachmentCount;
		}

		VkSubpassDescription desc;
		ANKI_VK_MEMSET_DBG(desc);
		desc.flags = 0;
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.inputAttachmentCount = 0;
		desc.pInputAttachments = nullptr;
		desc.colorAttachmentCount = init.m_color.m_attachmentCount;
		desc.pColorAttachments =
			(init.m_color.m_attachmentCount) ? &references[0] : nullptr;
		desc.pResolveAttachments = nullptr;
		desc.pDepthStencilAttachment =
			(hasDepthStencil) ? &dsReference : nullptr;
		desc.preserveAttachmentCount = 0;
		desc.pPreserveAttachments = nullptr;

		ANKI_ASSERT(ci.attachmentCount);
		ci.pAttachments = &attachmentDescriptions[0];
		ci.subpassCount = 1;
		ci.pSubpasses = &desc;
		ci.dependencyCount = 0;
		ci.pDependencies = nullptr;

		VkRenderPass rpass;
		ANKI_VK_CHECK(vkCreateRenderPass(m_device, &ci, nullptr, &rpass));

		m_renderPasses->m_hashmap.pushBack(m_alloc, key, rpass);
	}

	return out;
}

//==============================================================================
void GrManagerImpl::initMemory()
{
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

	m_gpuMemAllocs.create(m_alloc, m_memoryProperties.memoryTypeCount);
	U idx = 0;
	for(GpuMemoryAllocator& alloc : m_gpuMemAllocs)
	{
		alloc.init(m_alloc, m_device, idx++, 50 * 1024 * 1024, 1.0, 0);
	}
}

//==============================================================================
U GrManagerImpl::findMemoryType(U resourceMemTypeBits,
	VkMemoryPropertyFlags preferFlags,
	VkMemoryPropertyFlags avoidFlags) const
{
	U preferedHigh = MAX_U32;
	U preferedMed = MAX_U32;

	// Iterate all mem types
	for(U i = 0; i < m_memoryProperties.memoryTypeCount; i++)
	{
		if(resourceMemTypeBits & (1u << i))
		{
			VkMemoryPropertyFlags flags =
				m_memoryProperties.memoryTypes[i].propertyFlags;

			if((flags & preferFlags) == preferFlags)
			{
				preferedMed = i;

				if((flags & avoidFlags) != avoidFlags)
				{
					preferedHigh = i;
				}
			}
		}
	}

	if(preferedHigh < MAX_U32)
	{
		return preferedHigh;
	}
	else
	{
		ANKI_ASSERT(preferedMed < MAX_U32);
		return preferedMed;
	}
}

} // end namespace anki
