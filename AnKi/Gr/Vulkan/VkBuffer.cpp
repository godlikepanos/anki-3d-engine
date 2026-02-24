// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkBuffer.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

namespace anki {

Buffer* Buffer::newInstance(const BufferInitInfo& init)
{
	BufferImpl* impl = anki::newInstance<BufferImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

void* Buffer::map(PtrSize offset, PtrSize range)
{
	ANKI_VK_SELF(BufferImpl);

	ANKI_ASSERT(self.isCreated());
	ANKI_ASSERT(!!m_access);
	ANKI_ASSERT(!self.m_mapped);
	ANKI_ASSERT(offset < m_size);
	if(range == kMaxPtrSize)
	{
		range = m_size - offset;
	}
	ANKI_ASSERT(offset + range <= m_size);

	void* ptr;
	ANKI_VK_CHECKF(vkMapMemory(getVkDevice(), self.m_deviceMem, offset, range, 0, &ptr));
	ANKI_ASSERT(ptr);

#if ANKI_ASSERTIONS_ENABLED
	self.m_mapped = true;
#endif

	return ptr;
}

void Buffer::unmap()
{
#if ANKI_ASSERTIONS_ENABLED
	ANKI_VK_SELF(BufferImpl);

	ANKI_ASSERT(self.isCreated());
	ANKI_ASSERT(self.m_mapped);

	self.m_mapped = false;
#endif
}

void Buffer::flush(PtrSize offset, PtrSize range) const
{
	ANKI_VK_SELF_CONST(BufferImpl);

	ANKI_ASSERT(!!(m_access & BufferMapAccessBit::kWrite) && "No need to flush when the CPU doesn't write");
	if(self.m_needsFlush)
	{
		VkMappedMemoryRange vkrange = self.setVkMappedMemoryRange(offset, range);
		ANKI_VK_CHECKF(vkFlushMappedMemoryRanges(getVkDevice(), 1, &vkrange));
#if ANKI_ASSERTIONS_ENABLED
		self.m_flushCount.fetchAdd(1);
#endif
	}
}

void Buffer::invalidate(PtrSize offset, PtrSize range) const
{
	ANKI_VK_SELF_CONST(BufferImpl);

	ANKI_ASSERT(!!(m_access & BufferMapAccessBit::kRead) && "No need to invalidate when the CPU doesn't read");
	if(self.m_needsInvalidate)
	{
		VkMappedMemoryRange vkrange = self.setVkMappedMemoryRange(offset, range);
		ANKI_VK_CHECKF(vkInvalidateMappedMemoryRanges(getVkDevice(), 1, &vkrange));
#if ANKI_ASSERTIONS_ENABLED
		self.m_invalidateCount.fetchAdd(1);
#endif
	}
}

BufferImpl::~BufferImpl()
{
	ANKI_ASSERT(!m_mapped);

#if ANKI_ASSERTIONS_ENABLED
	if(m_needsFlush && m_flushCount.load() == 0)
	{
		ANKI_VK_LOGW("Buffer needed flushing but you never flushed: %s", getName().cstr());
	}

	if(m_needsInvalidate && m_invalidateCount.load() == 0)
	{
		ANKI_VK_LOGW("Buffer needed invalidation but you never invalidated: %s", getName().cstr());
	}
#endif

	for(VkBufferView view : m_views)
	{
		vkDestroyBufferView(getVkDevice(), view, nullptr);
	}

	if(m_handle)
	{
		vkDestroyBuffer(getVkDevice(), m_handle, nullptr);
	}

	if(m_deviceMem)
	{
		vkFreeMemory(getVkDevice(), m_deviceMem, nullptr);

		if(getGrManagerImpl().getMemoryProperties().memoryTypes[m_memoryTypeIdx].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			g_svarGpuDeviceMemoryAllocated.increment(m_actualSize);
		}
		else
		{
			g_svarGpuHostMemoryAllocated.increment(m_actualSize);
		}
	}
}

Error BufferImpl::init(const BufferInitInfo& inf)
{
	ANKI_ASSERT(!isCreated());
	const Bool exposeGpuAddress = !!(inf.m_usage & ~BufferUsageBit::kAllCopy);

	PtrSize size = inf.m_size;
	BufferMapAccessBit access = inf.m_mapAccess;
	BufferUsageBit usage = inf.m_usage;

	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(usage != BufferUsageBit::kNone);

	m_mappedMemoryRangeAlignment = getGrManagerImpl().getVulkanCapabilities().m_nonCoherentAtomSize;

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
		ci.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	}
	ci.queueFamilyIndexCount = getGrManagerImpl().getQueueFamilies().getSize();
	ci.pQueueFamilyIndices = &getGrManagerImpl().getQueueFamilies()[0];
	ci.sharingMode = (ci.queueFamilyIndexCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	ANKI_VK_CHECK(vkCreateBuffer(getVkDevice(), &ci, nullptr, &m_handle));
	getGrManagerImpl().trySetVulkanHandleName(inf.getName(), VK_OBJECT_TYPE_BUFFER, m_handle);

	// Get mem requirements
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(getVkDevice(), m_handle, &req);
	U32 memIdx = kMaxU32;
	const Bool isDiscreteGpu = getGrManagerImpl().getDeviceCapabilities().m_discreteGpu;

	if(access == BufferMapAccessBit::kWrite)
	{
		// Only write, probably for uploads

		// 1st try: Device & host & coherent but not cached
		VkMemoryPropertyFlags prefer = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VkMemoryPropertyFlags avoid = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

		if(isDiscreteGpu)
		{
			if((usage & (~BufferUsageBit::kAllCopy)) != BufferUsageBit::kNone)
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

		memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits, prefer, avoid);

		// 2nd try: host & coherent
		if(memIdx == kMaxU32)
		{
			prefer = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			avoid = 0;

			if(isDiscreteGpu)
			{
				ANKI_VK_LOGW("Using a fallback mode for write-only buffer");

				if((usage & (~BufferUsageBit::kAllCopy)) == BufferUsageBit::kNone)
				{
					// Will be used only for transfers, don't want it in the device
					avoid |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				}
			}

			memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits, prefer, avoid);
		}
	}
	else if(!!(access & BufferMapAccessBit::kRead))
	{
		// Read or read/write

		// Cached & coherent
		memIdx = getGrManagerImpl().findMemoryType(
			req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);

		// Fallback: Just cached
		if(memIdx == kMaxU32)
		{
			if(isDiscreteGpu)
			{
				ANKI_VK_LOGW("Using a fallback mode for read/write buffer");
			}

			memIdx =
				getGrManagerImpl().findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, 0);
		}
	}
	else
	{
		// Not mapped

		ANKI_ASSERT(access == BufferMapAccessBit::kNone);

		// Device only
		memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Fallback: Device with anything else
		if(memIdx == kMaxU32)
		{
			memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}
	}

	if(memIdx == kMaxU32)
	{
		ANKI_VK_LOGE("Failed to find appropriate memory type for buffer: %s", getName().cstr());
		return Error::kFunctionFailed;
	}

	m_memoryTypeIdx = memIdx;

	const VkPhysicalDeviceMemoryProperties& props = getGrManagerImpl().getMemoryProperties();
	const VkMemoryPropertyFlags memoryFlags = props.memoryTypes[memIdx].propertyFlags;

	if(!!(access & BufferMapAccessBit::kRead) && !(memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		m_needsInvalidate = true;
	}

	if(!!(access & BufferMapAccessBit::kWrite) && !(memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		m_needsFlush = true;
	}

	// Allocate
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.memoryTypeIndex = memIdx;
	allocInfo.allocationSize = req.size;
	ANKI_VK_CHECK(vkAllocateMemory(getVkDevice(), &allocInfo, nullptr, &m_deviceMem));

	if(getGrManagerImpl().getMemoryProperties().memoryTypes[memIdx].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		g_svarGpuDeviceMemoryAllocated.increment(size);
	}
	else
	{
		g_svarGpuHostMemoryAllocated.increment(size);
	}

	// Bind mem to buffer
	{
		ANKI_TRACE_SCOPED_EVENT(VkBindObject);
		ANKI_VK_CHECK(vkBindBufferMemory(getVkDevice(), m_handle, m_deviceMem, 0));
	}

	// Get GPU buffer address
	if(exposeGpuAddress)
	{
		VkBufferDeviceAddressInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		info.buffer = m_handle;
		m_gpuAddress = vkGetBufferDeviceAddress(getVkDevice(), &info);

		if(m_gpuAddress == 0)
		{
			ANKI_VK_LOGE("vkGetBufferDeviceAddress() failed");
			return Error::kFunctionFailed;
		}
	}

	m_access = access;
	m_size = inf.m_size;
	m_actualSize = size;
	m_usage = usage;
	return Error::kNone;
}

VkPipelineStageFlags BufferImpl::computePplineStage(BufferUsageBit usage)
{
	const Bool rt = getGrManagerImpl().getDeviceCapabilities().m_rayTracing;
	VkPipelineStageFlags stageMask = 0;

	if(!!(usage & BufferUsageBit::kAllIndirect))
	{
		stageMask |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}

	if(!!(usage & (BufferUsageBit::kVertexOrIndex)))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	}

	if(!!(usage & BufferUsageBit::kAllGeometry))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

		if(getGrManagerImpl().getDeviceCapabilities().m_meshShaders)
		{
			stageMask |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT | VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
		}
	}

	if(!!(usage & BufferUsageBit::kAllPixel))
	{
		stageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::kAllCompute & ~BufferUsageBit::kIndirectCompute)))
	{
		stageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::kAccelerationStructureBuild | BufferUsageBit::kAccelerationStructureBuildScratch)) && rt)
	{
		stageMask |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	}

	if(!!(usage & (BufferUsageBit::kAllDispatchRays & ~BufferUsageBit::kIndirectDispatchRays)) && rt)
	{
		stageMask |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
	}

	if(!!(usage & BufferUsageBit::kAllCopy))
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

	constexpr BufferUsageBit kShaderRead = BufferUsageBit::kAllShaderResource & BufferUsageBit::kAllRead;
	constexpr BufferUsageBit kShaderWrite = BufferUsageBit::kAllShaderResource & BufferUsageBit::kAllWrite;

	if(!!(usage & BufferUsageBit::kAllConstant))
	{
		mask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(!!(usage & kShaderRead))
	{
		mask |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(usage & kShaderWrite))
	{
		mask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(usage & BufferUsageBit::kVertexOrIndex))
	{
		mask |= VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::kAllIndirect))
	{
		mask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::kCopyDestination))
	{
		mask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(!!(usage & BufferUsageBit::kCopySource))
	{
		mask |= VK_ACCESS_TRANSFER_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::kAccelerationStructureBuild))
	{
		mask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(!!(usage & BufferUsageBit::kAccelerationStructureBuildScratch))
	{
		mask |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	return mask;
}

VkBufferMemoryBarrier BufferImpl::computeBarrierInfo(BufferUsageBit before, BufferUsageBit after, VkPipelineStageFlags& srcStages,
													 VkPipelineStageFlags& dstStages) const
{
	ANKI_ASSERT(usageValid(before) && usageValid(after));
	ANKI_ASSERT(!!after);

	VkBufferMemoryBarrier barrier = {};
	barrier.buffer = m_handle;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.srcAccessMask = computeAccessMask(before);
	barrier.dstAccessMask = computeAccessMask(after);
	barrier.offset = 0;
	barrier.size = VK_WHOLE_SIZE; // All size because none cares really

	srcStages |= computePplineStage(before);
	dstStages |= computePplineStage(after);

	return barrier;
}

VkBufferView BufferImpl::getOrCreateBufferView(Format fmt, PtrSize offset, PtrSize range) const
{
	if(range == kMaxPtrSize)
	{
		ANKI_ASSERT(m_size >= offset);
		range = m_size - offset;
		range = getAlignedRoundDown(getFormatInfo(fmt).m_texelSize, range);
	}

	// Checks
	ANKI_ASSERT(offset + range <= m_size);

	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_texelBufferBindOffsetAlignment, offset) && "Offset not aligned");

	ANKI_ASSERT((range % getFormatInfo(fmt).m_texelSize) == 0 && "Range doesn't align with the number of texel elements");

	[[maybe_unused]] const PtrSize elementCount = range / getFormatInfo(fmt).m_texelSize;
	ANKI_ASSERT(elementCount <= getGrManagerImpl().getVulkanCapabilities().m_maxTexelBufferElements);

	// Hash
	ANKI_BEGIN_PACKED_STRUCT
	class HashData
	{
	public:
		PtrSize m_offset;
		PtrSize m_range;
		Format m_fmt;
	} toHash;
	ANKI_END_PACKED_STRUCT

	toHash.m_fmt = fmt;
	toHash.m_offset = offset;
	toHash.m_range = range;

	const U64 hash = computeHash(&toHash, sizeof(toHash));

	// Check if exists
	{
		RLockGuard lock(m_viewsMtx);

		auto it = m_views.find(hash);
		if(it != m_views.getEnd())
		{
			return *it;
		}
	}

	WLockGuard lock(m_viewsMtx);

	// Check again
	auto it = m_views.find(hash);
	if(it != m_views.getEnd())
	{
		return *it;
	}

	// Doesn't exist, need to create it
	VkBufferViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	viewCreateInfo.buffer = m_handle;
	viewCreateInfo.format = convertFormat(fmt);
	viewCreateInfo.offset = offset;
	viewCreateInfo.range = range;

	VkBufferView view;
	ANKI_VK_CHECKF(vkCreateBufferView(getVkDevice(), &viewCreateInfo, nullptr, &view));

	m_views.emplace(hash, view);

	return view;
}

} // end namespace anki
