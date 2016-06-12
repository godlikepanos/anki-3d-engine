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

	if(!m_memHandle.isEmpty())
	{
		getGrManagerImpl().freeMemory(m_memIdx, m_memHandle);
	}
}

//==============================================================================
Error BufferImpl::init(
	PtrSize size, BufferUsageBit usage, BufferAccessBit access)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(usage != BufferUsageBit::NONE);

	// Create the buffer
	VkBufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.size = size;
	ci.usage = convertBufferUsageBit(usage);
	ci.queueFamilyIndexCount = 1;
	U32 queueIdx = getGrManagerImpl().getGraphicsQueueFamily();
	ci.pQueueFamilyIndices = &queueIdx;
	ANKI_VK_CHECK(vkCreateBuffer(getDevice(), &ci, nullptr, &m_handle));

	// Get mem requirements
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(getDevice(), m_handle, &req);

	if((access & (BufferAccessBit::CLIENT_MAP_READ
					 | BufferAccessBit::CLIENT_MAP_WRITE))
		!= BufferAccessBit::NONE)
	{
		m_memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				| VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
			0);

		// Fallback
		if(m_memIdx == MAX_U32)
		{
			m_memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				0);
		}
	}
	else
	{
		m_memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Fallback
		if(m_memIdx == MAX_U32)
		{
			m_memIdx = getGrManagerImpl().findMemoryType(
				req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}
	}

	ANKI_ASSERT(m_memIdx != MAX_U32);

	// Allocate
	getGrManagerImpl().allocateMemory(
		m_memIdx, req.size, req.alignment, m_memHandle);

	// Bind mem to buffer
	ANKI_VK_CHECK(vkBindBufferMemory(
		getDevice(), m_handle, m_memHandle.m_memory, m_memHandle.m_offset));

	m_access = access;
	m_size = size;
	return ErrorCode::NONE;
}

//==============================================================================
void* BufferImpl::map(PtrSize offset, PtrSize range, BufferAccessBit access)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT((access & m_access) != BufferAccessBit::NONE);
	ANKI_ASSERT(!m_mapped);
	ANKI_ASSERT(offset + range <= m_size);

	void* ptr = getGrManagerImpl().getMappedAddress(m_memIdx, m_memHandle);
	ANKI_ASSERT(ptr);

#if ANKI_ASSERTIONS
	m_mapped = true;
#endif

	return static_cast<void*>(static_cast<U8*>(ptr) + offset);
}

} // end namespace anki