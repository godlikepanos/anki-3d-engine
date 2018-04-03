// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

BufferImpl::~BufferImpl()
{
	ANKI_ASSERT(!m_mapped);

	if(m_handle)
	{
		vkDestroyBuffer(getDevice(), m_handle, nullptr);
	}

	if(m_memHandle)
	{
		getGrManagerImpl().getGpuMemoryManager().freeMemory(m_memHandle);
	}
}

Error BufferImpl::init(const BufferInitInfo& inf)
{
	ANKI_ASSERT(!isCreated());

	PtrSize size = inf.m_size;
	BufferMapAccessBit access = inf.m_access;
	BufferUsageBit usage = inf.m_usage;

	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(usage != BufferUsageBit::NONE);

	// Align the size to satisfy fill buffer
	alignRoundUp(4, size);

	// Create the buffer
	VkBufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.size = size;
	ci.usage = convertBufferUsageBit(usage);
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.queueFamilyIndexCount = 1;
	U32 queueIdx = getGrManagerImpl().getGraphicsQueueFamily();
	ci.pQueueFamilyIndices = &queueIdx;
	ANKI_VK_CHECK(vkCreateBuffer(getDevice(), &ci, nullptr, &m_handle));
	getGrManagerImpl().trySetVulkanHandleName(inf.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, m_handle);

	// Get mem requirements
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(getDevice(), m_handle, &req);
	U memIdx = MAX_U32;

	if(access == BufferMapAccessBit::WRITE)
	{
		// Only write

		// Device & host & coherent but not cached
		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

		// Fallback: host & coherent and not cached
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
		}

		// Fallback: just host
		if(memIdx == MAX_U32)
		{
			ANKI_VK_LOGW("Using a fallback mode for write-only buffer");
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
		}
	}
	else if((access & BufferMapAccessBit::READ) != BufferMapAccessBit::NONE)
	{
		// Read or read/write

		// Cached & coherent
		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			0);

		// Fallback: Just cached
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, 0);
		}

		// Fallback: Just host
		if(memIdx == MAX_U32)
		{
			ANKI_VK_LOGW("Using a fallback mode for read/write buffer");
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
		}
	}
	else
	{
		// Not mapped

		ANKI_ASSERT(access == BufferMapAccessBit::NONE);

		// Device only
		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
			req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Fallback: Device with anything else
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}
	}

	ANKI_ASSERT(memIdx != MAX_U32);

	const VkPhysicalDeviceMemoryProperties& props = getGrManagerImpl().getMemoryProperties();
	m_memoryFlags = props.memoryTypes[memIdx].propertyFlags;

	// Allocate
	getGrManagerImpl().getGpuMemoryManager().allocateMemory(memIdx, req.size, req.alignment, true, m_memHandle);

	// Bind mem to buffer
	ANKI_TRACE_START_EVENT(VK_BIND_OBJECT);
	ANKI_VK_CHECK(vkBindBufferMemory(getDevice(), m_handle, m_memHandle.m_memory, m_memHandle.m_offset));
	ANKI_TRACE_STOP_EVENT(VK_BIND_OBJECT);

	m_access = access;
	m_size = inf.m_size;
	m_actualSize = size;
	m_usage = usage;
	return Error::NONE;
}

void* BufferImpl::map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(access != BufferMapAccessBit::NONE);
	ANKI_ASSERT((access & m_access) != BufferMapAccessBit::NONE);
	ANKI_ASSERT(!m_mapped);
	ANKI_ASSERT(offset + range <= m_size);

	void* ptr = getGrManagerImpl().getGpuMemoryManager().getMappedAddress(m_memHandle);
	ANKI_ASSERT(ptr);

#if ANKI_EXTRA_CHECKS
	m_mapped = true;
#endif

	// TODO Flush or invalidate caches

	return static_cast<void*>(static_cast<U8*>(ptr) + offset);
}

VkPipelineStageFlags BufferImpl::computePplineStage(BufferUsageBit usage)
{
	VkPipelineStageFlags stageMask = 0;

	if(!!(usage & (BufferUsageBit::UNIFORM_VERTEX | BufferUsageBit::STORAGE_VERTEX_READ_WRITE)))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}

	if(!!(usage
		   & (BufferUsageBit::UNIFORM_TESSELLATION_EVALUATION
				 | BufferUsageBit::STORAGE_TESSELLATION_EVALUATION_READ_WRITE)))
	{
		stageMask |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}

	if(!!(usage
		   & (BufferUsageBit::UNIFORM_TESSELLATION_CONTROL | BufferUsageBit::STORAGE_TESSELLATION_CONTROL_READ_WRITE)))
	{
		stageMask |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::UNIFORM_GEOMETRY | BufferUsageBit::STORAGE_GEOMETRY_READ_WRITE)))
	{
		stageMask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::STORAGE_FRAGMENT_READ_WRITE)))
	{
		stageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::UNIFORM_COMPUTE | BufferUsageBit::STORAGE_COMPUTE_READ_WRITE)))
	{
		stageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::INDEX | BufferUsageBit::VERTEX)))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}

	if(!!(usage & BufferUsageBit::INDIRECT))
	{
		stageMask |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}

	if(!!(usage & (BufferUsageBit::TRANSFER_ALL)))
	{
		stageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	if(!!(usage & (BufferUsageBit::QUERY_RESULT)))
	{
		stageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	if(!stageMask)
	{
		stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	ANKI_ASSERT(stageMask);
	return stageMask;
}

VkAccessFlags BufferImpl::computeAccessMask(BufferUsageBit usage)
{
	VkAccessFlags mask = 0;

	const BufferUsageBit SHADER_READ =
		BufferUsageBit::STORAGE_VERTEX_READ | BufferUsageBit::STORAGE_TESSELLATION_CONTROL_READ
		| BufferUsageBit::STORAGE_TESSELLATION_EVALUATION_READ | BufferUsageBit::STORAGE_GEOMETRY_READ
		| BufferUsageBit::STORAGE_FRAGMENT_READ | BufferUsageBit::STORAGE_COMPUTE_READ;

	const BufferUsageBit SHADER_WRITE =
		BufferUsageBit::STORAGE_VERTEX_WRITE | BufferUsageBit::STORAGE_TESSELLATION_CONTROL_WRITE
		| BufferUsageBit::STORAGE_TESSELLATION_EVALUATION_WRITE | BufferUsageBit::STORAGE_GEOMETRY_WRITE
		| BufferUsageBit::STORAGE_FRAGMENT_WRITE | BufferUsageBit::STORAGE_COMPUTE_WRITE;

	if(!!(usage & BufferUsageBit::UNIFORM_ALL))
	{
		mask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(!!(usage & SHADER_READ))
	{
		mask |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(usage & SHADER_WRITE))
	{
		mask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(usage & BufferUsageBit::INDEX))
	{
		mask |= VK_ACCESS_INDEX_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::VERTEX))
	{
		mask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::INDIRECT))
	{
		mask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}

	if(!!(usage & (BufferUsageBit::FILL | BufferUsageBit::BUFFER_UPLOAD_DESTINATION)))
	{
		mask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(!!(usage & (BufferUsageBit::BUFFER_UPLOAD_SOURCE | BufferUsageBit::TEXTURE_UPLOAD_SOURCE)))
	{
		mask |= VK_ACCESS_TRANSFER_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::QUERY_RESULT))
	{
		mask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	return mask;
}

void BufferImpl::computeBarrierInfo(BufferUsageBit before,
	BufferUsageBit after,
	VkPipelineStageFlags& srcStages,
	VkAccessFlags& srcAccesses,
	VkPipelineStageFlags& dstStages,
	VkAccessFlags& dstAccesses) const
{
	ANKI_ASSERT(usageValid(before) && usageValid(after));
	ANKI_ASSERT(!!after);

	srcStages = computePplineStage(before);
	dstStages = computePplineStage(after);
	srcAccesses = computeAccessMask(before);
	dstAccesses = computeAccessMask(after);
}

} // end namespace anki
