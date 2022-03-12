// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/BufferImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

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

#if ANKI_EXTRA_CHECKS
	if(m_needsFlush && m_flushCount.load() == 0)
	{
		ANKI_VK_LOGW("Buffer needed flushing but you never flushed: %s", getName().cstr());
	}

	if(m_needsInvalidate && m_invalidateCount.load() == 0)
	{
		ANKI_VK_LOGW("Buffer needed invalidation but you never invalidated: %s", getName().cstr());
	}
#endif
}

Error BufferImpl::init(const BufferInitInfo& inf)
{
	ANKI_ASSERT(!isCreated());
	const Bool exposeGpuAddress = !!(getGrManagerImpl().getExtensions() & VulkanExtensions::KHR_BUFFER_DEVICE_ADDRESS)
								  && !!(inf.m_usage & ~BufferUsageBit::ALL_TRANSFER);

	PtrSize size = inf.m_size;
	BufferMapAccessBit access = inf.m_mapAccess;
	BufferUsageBit usage = inf.m_usage;

	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(usage != BufferUsageBit::NONE);

	m_mappedMemoryRangeAlignment = getGrManagerImpl().getPhysicalDeviceProperties().limits.nonCoherentAtomSize;

	// Align the size to satisfy fill buffer
	alignRoundUp(4, size);

	// Align to satisfy the flush and invalidate
	alignRoundUp(m_mappedMemoryRangeAlignment, size);

	// Create the buffer
	VkBufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.size = size;
	ci.usage = convertBufferUsageBit(usage);
	if(exposeGpuAddress)
	{
		ci.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
	}
	ci.queueFamilyIndexCount = getGrManagerImpl().getQueueFamilies().getSize();
	ci.pQueueFamilyIndices = &getGrManagerImpl().getQueueFamilies()[0];
	ci.sharingMode = (ci.queueFamilyIndexCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	ANKI_VK_CHECK(vkCreateBuffer(getDevice(), &ci, nullptr, &m_handle));
	getGrManagerImpl().trySetVulkanHandleName(inf.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, m_handle);

	// Get mem requirements
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(getDevice(), m_handle, &req);
	U32 memIdx = MAX_U32;
	const Bool isDiscreteGpu = getGrManagerImpl().getDeviceCapabilities().m_discreteGpu;

	if(access == BufferMapAccessBit::WRITE)
	{
		// Only write, probably for uploads

		// 1st try: Device & host & coherent but not cached
		VkMemoryPropertyFlags prefer = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VkMemoryPropertyFlags avoid = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

		if(isDiscreteGpu)
		{
			if((usage & (~BufferUsageBit::ALL_TRANSFER)) != BufferUsageBit::NONE)
			{
				// Will be used for something other than transfer, try to put it in the device
				prefer |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			}
			else
			{
				// Will be used only for transfers, don't want it in the device
				avoid |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			}
		}

		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits, prefer, avoid);

		// 2nd try: host & coherent
		if(memIdx == MAX_U32)
		{
			prefer = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			avoid = 0;

			if(isDiscreteGpu)
			{
				ANKI_VK_LOGW("Using a fallback mode for write-only buffer");

				if((usage & (~BufferUsageBit::ALL_TRANSFER)) == BufferUsageBit::NONE)
				{
					// Will be used only for transfers, don't want it in the device
					avoid |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				}
			}

			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits, prefer, avoid);
		}
	}
	else if(!!(access & BufferMapAccessBit::READ))
	{
		// Read or read/write

		// Cached & coherent
		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits,
																		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
																			 | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
																			 | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
																		 0);

		// Fallback: Just cached
		if(memIdx == MAX_U32)
		{
			if(isDiscreteGpu)
			{
				ANKI_VK_LOGW("Using a fallback mode for read/write buffer");
			}

			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, 0);
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
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits,
																			 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}
	}

	if(memIdx == MAX_U32)
	{
		ANKI_VK_LOGE("Failed to find appropriate memory type for buffer: %s", getName().cstr());
		return Error::FUNCTION_FAILED;
	}

	const VkPhysicalDeviceMemoryProperties& props = getGrManagerImpl().getMemoryProperties();
	m_memoryFlags = props.memoryTypes[memIdx].propertyFlags;

	if(!!(access & BufferMapAccessBit::READ) && !(m_memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		m_needsInvalidate = true;
	}

	if(!!(access & BufferMapAccessBit::WRITE) && !(m_memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		m_needsFlush = true;
	}

	// Allocate
	const U32 alignment = U32(max(m_mappedMemoryRangeAlignment, req.alignment));
	getGrManagerImpl().getGpuMemoryManager().allocateMemory(memIdx, req.size, alignment, true, m_memHandle);

	// Bind mem to buffer
	{
		ANKI_TRACE_SCOPED_EVENT(VK_BIND_OBJECT);
		ANKI_VK_CHECK(vkBindBufferMemory(getDevice(), m_handle, m_memHandle.m_memory, m_memHandle.m_offset));
	}

	// Get GPU buffer address
	if(exposeGpuAddress)
	{
		VkBufferDeviceAddressInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
		info.buffer = m_handle;
		m_gpuAddress = vkGetBufferDeviceAddressKHR(getDevice(), &info);

		if(m_gpuAddress == 0)
		{
			ANKI_VK_LOGE("vkGetBufferDeviceAddressKHR() failed");
			return Error::FUNCTION_FAILED;
		}
	}

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
	ANKI_ASSERT(offset < m_size);
	if(range == MAX_PTR_SIZE)
	{
		range = m_size - offset;
	}
	ANKI_ASSERT(offset + range <= m_size);

	void* ptr = getGrManagerImpl().getGpuMemoryManager().getMappedAddress(m_memHandle);
	ANKI_ASSERT(ptr);

#if ANKI_EXTRA_CHECKS
	m_mapped = true;
#endif

	return static_cast<void*>(static_cast<U8*>(ptr) + offset);
}

VkPipelineStageFlags BufferImpl::computePplineStage(BufferUsageBit usage)
{
	VkPipelineStageFlags stageMask = 0;

	if(!!(usage & BufferUsageBit::ALL_INDIRECT))
	{
		stageMask |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}

	if(!!(usage & (BufferUsageBit::INDEX | BufferUsageBit::VERTEX)))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	}

	if(!!(usage & BufferUsageBit::ALL_GEOMETRY))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}

	if(!!(usage & BufferUsageBit::ALL_FRAGMENT))
	{
		stageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::ALL_COMPUTE & ~BufferUsageBit::INDIRECT_COMPUTE)))
	{
		stageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	if(!!(usage & BufferUsageBit::ACCELERATION_STRUCTURE_BUILD))
	{
		stageMask |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	}

	if(!!(usage & (BufferUsageBit::ALL_TRACE_RAYS & ~BufferUsageBit::INDIRECT_TRACE_RAYS)))
	{
		stageMask |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
	}

	if(!!(usage & BufferUsageBit::ALL_TRANSFER))
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

	constexpr BufferUsageBit SHADER_READ =
		BufferUsageBit::STORAGE_GEOMETRY_READ | BufferUsageBit::STORAGE_FRAGMENT_READ
		| BufferUsageBit::STORAGE_COMPUTE_READ | BufferUsageBit::STORAGE_TRACE_RAYS_READ
		| BufferUsageBit::TEXTURE_GEOMETRY_READ | BufferUsageBit::TEXTURE_FRAGMENT_READ
		| BufferUsageBit::TEXTURE_COMPUTE_READ | BufferUsageBit::TEXTURE_TRACE_RAYS_READ;

	constexpr BufferUsageBit SHADER_WRITE =
		BufferUsageBit::STORAGE_GEOMETRY_WRITE | BufferUsageBit::STORAGE_FRAGMENT_WRITE
		| BufferUsageBit::STORAGE_COMPUTE_WRITE | BufferUsageBit::STORAGE_TRACE_RAYS_WRITE
		| BufferUsageBit::TEXTURE_GEOMETRY_WRITE | BufferUsageBit::TEXTURE_FRAGMENT_WRITE
		| BufferUsageBit::TEXTURE_COMPUTE_WRITE | BufferUsageBit::TEXTURE_TRACE_RAYS_WRITE;

	if(!!(usage & BufferUsageBit::ALL_UNIFORM))
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

	if(!!(usage & BufferUsageBit::ALL_INDIRECT))
	{
		mask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::TRANSFER_DESTINATION))
	{
		mask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(!!(usage & BufferUsageBit::TRANSFER_SOURCE))
	{
		mask |= VK_ACCESS_TRANSFER_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::ACCELERATION_STRUCTURE_BUILD))
	{
		mask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	return mask;
}

void BufferImpl::computeBarrierInfo(BufferUsageBit before, BufferUsageBit after, VkPipelineStageFlags& srcStages,
									VkAccessFlags& srcAccesses, VkPipelineStageFlags& dstStages,
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
