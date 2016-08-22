// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

//==============================================================================
BufferImpl::~BufferImpl()
{
	ANKI_ASSERT(!m_mapped);

	if(m_handle)
	{
		vkDestroyBuffer(getDevice(), m_handle, nullptr);
	}

	if(m_memHandle)
	{
		getGrManagerImpl().getGpuMemoryAllocator().freeMemory(m_memHandle);
	}
}

//==============================================================================
Error BufferImpl::init(
	PtrSize size, BufferUsageBit usage, BufferMapAccessBit access)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(usage != BufferUsageBit::NONE);

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

	// Get mem requirements
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(getDevice(), m_handle, &req);
	U memIdx = MAX_U32;

	if(access == BufferMapAccessBit::WRITE)
	{
		// Only write

		// Device & host but not coherent
		memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
			req.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Fallback: host and not coherent
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
				req.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		// Fallback: any host
		if(memIdx == MAX_U32)
		{
			ANKI_LOGW("Vulkan: Using a fallback mode for write-only buffer");
			memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
		}
	}
	else if((access & BufferMapAccessBit::READ) != BufferMapAccessBit::NONE)
	{
		// Read or read/write

		// Cached & coherent
		memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
			req.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_CACHED_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			0);

		// Fallback: Just cached
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
				req.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
					| VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
				0);
		}

		// Fallback: Just host
		if(memIdx == MAX_U32)
		{
			ANKI_LOGW("Vulkan: Using a fallback mode for read/write buffer");
			memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
		}
	}
	else
	{
		// Not mapped

		ANKI_ASSERT(access == BufferMapAccessBit::NONE);

		// Device only
		memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
			req.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Fallback: Device with anything else
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryAllocator().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}
	}

	ANKI_ASSERT(memIdx != MAX_U32);

	const VkPhysicalDeviceMemoryProperties& props =
		getGrManagerImpl().getMemoryProperties();
	m_memoryFlags = props.memoryTypes[memIdx].propertyFlags;

	// Allocate
	getGrManagerImpl().getGpuMemoryAllocator().allocateMemory(
		memIdx, req.size, req.alignment, true, m_memHandle);

	// Bind mem to buffer
	ANKI_VK_CHECK(vkBindBufferMemory(
		getDevice(), m_handle, m_memHandle.m_memory, m_memHandle.m_offset));

	m_access = access;
	m_size = size;
	m_usage = usage;
	return ErrorCode::NONE;
}

//==============================================================================
void* BufferImpl::map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(access != BufferMapAccessBit::NONE);
	ANKI_ASSERT((access & m_access) != BufferMapAccessBit::NONE);
	ANKI_ASSERT(!m_mapped);
	ANKI_ASSERT(offset + range <= m_size);

	void* ptr = getGrManagerImpl().getGpuMemoryAllocator().getMappedAddress(
		m_memHandle);
	ANKI_ASSERT(ptr);

#if ANKI_ASSERTIONS
	m_mapped = true;
#endif

	// TODO Flush or invalidate caches

	return static_cast<void*>(static_cast<U8*>(ptr) + offset);
}

} // end namespace anki
