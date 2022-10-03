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

	BufferGarbage* garbage = anki::newInstance<BufferGarbage>(getMemoryPool());
	garbage->m_bufferHandle = m_handle;
	garbage->m_memoryHandle = m_memHandle;

	if(m_views.getSize())
	{
		garbage->m_viewHandles.create(getMemoryPool(), U32(m_views.getSize()));

		U32 count = 0;
		for(auto it : m_views)
		{
			const VkBufferView view = it;
			garbage->m_viewHandles[count++] = view;
		}
	}

	getGrManagerImpl().getFrameGarbageCollector().newBufferGarbage(garbage);

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

	m_views.destroy(getMemoryPool());
}

Error BufferImpl::init(const BufferInitInfo& inf)
{
	ANKI_ASSERT(!isCreated());
	const Bool exposeGpuAddress = !!(getGrManagerImpl().getExtensions() & VulkanExtensions::kKHR_buffer_device_address)
								  && !!(inf.m_usage & ~BufferUsageBit::kAllTransfer);

	PtrSize size = inf.m_size;
	BufferMapAccessBit access = inf.m_mapAccess;
	BufferUsageBit usage = inf.m_usage;

	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(usage != BufferUsageBit::kNone);

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
	getGrManagerImpl().trySetVulkanHandleName(inf.getName(), VK_OBJECT_TYPE_BUFFER, m_handle);

	// Get mem requirements
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(getDevice(), m_handle, &req);
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
			if((usage & (~BufferUsageBit::kAllTransfer)) != BufferUsageBit::kNone)
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
		if(memIdx == kMaxU32)
		{
			prefer = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			avoid = 0;

			if(isDiscreteGpu)
			{
				ANKI_VK_LOGW("Using a fallback mode for write-only buffer");

				if((usage & (~BufferUsageBit::kAllTransfer)) == BufferUsageBit::kNone)
				{
					// Will be used only for transfers, don't want it in the device
					avoid |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				}
			}

			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits, prefer, avoid);
		}
	}
	else if(!!(access & BufferMapAccessBit::kRead))
	{
		// Read or read/write

		// Cached & coherent
		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits,
																		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
																			 | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
																			 | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
																		 0);

		// Fallback: Just cached
		if(memIdx == kMaxU32)
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

		ANKI_ASSERT(access == BufferMapAccessBit::kNone);

		// Device only
		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
			req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Fallback: Device with anything else
		if(memIdx == kMaxU32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryTypeBits,
																			 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}
	}

	if(memIdx == kMaxU32)
	{
		ANKI_VK_LOGE("Failed to find appropriate memory type for buffer: %s", getName().cstr());
		return Error::kFunctionFailed;
	}

	const VkPhysicalDeviceMemoryProperties& props = getGrManagerImpl().getMemoryProperties();
	m_memoryFlags = props.memoryTypes[memIdx].propertyFlags;

	if(!!(access & BufferMapAccessBit::kRead) && !(m_memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		m_needsInvalidate = true;
	}

	if(!!(access & BufferMapAccessBit::kWrite) && !(m_memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		m_needsFlush = true;
	}

	// Allocate
	const U32 alignment = U32(max(m_mappedMemoryRangeAlignment, req.alignment));
	getGrManagerImpl().getGpuMemoryManager().allocateMemory(memIdx, req.size, alignment, m_memHandle);

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
			return Error::kFunctionFailed;
		}
	}

	m_access = access;
	m_size = inf.m_size;
	m_actualSize = size;
	m_usage = usage;
	return Error::kNone;
}

void* BufferImpl::map(PtrSize offset, PtrSize range, [[maybe_unused]] BufferMapAccessBit access)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(access != BufferMapAccessBit::kNone);
	ANKI_ASSERT((access & m_access) != BufferMapAccessBit::kNone);
	ANKI_ASSERT(!m_mapped);
	ANKI_ASSERT(offset < m_size);
	if(range == kMaxPtrSize)
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

	if(!!(usage & BufferUsageBit::kAllIndirect))
	{
		stageMask |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}

	if(!!(usage & (BufferUsageBit::kIndex | BufferUsageBit::kVertex)))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	}

	if(!!(usage & BufferUsageBit::kAllGeometry))
	{
		stageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}

	if(!!(usage & BufferUsageBit::kAllFragment))
	{
		stageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if(!!(usage & (BufferUsageBit::kAllCompute & ~BufferUsageBit::kIndirectCompute)))
	{
		stageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	if(!!(usage & BufferUsageBit::kAccelerationStructureBuild))
	{
		stageMask |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	}

	if(!!(usage & (BufferUsageBit::kAllTraceRays & ~BufferUsageBit::kIndirectTraceRays)))
	{
		stageMask |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
	}

	if(!!(usage & BufferUsageBit::kAllTransfer))
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

	constexpr BufferUsageBit kShaderRead = BufferUsageBit::kStorageGeometryRead | BufferUsageBit::kStorageFragmentRead
										   | BufferUsageBit::kStorageComputeRead | BufferUsageBit::kStorageTraceRaysRead
										   | BufferUsageBit::kTextureGeometryRead | BufferUsageBit::kTextureFragmentRead
										   | BufferUsageBit::kTextureComputeRead
										   | BufferUsageBit::kTextureTraceRaysRead;

	constexpr BufferUsageBit kShaderWrite =
		BufferUsageBit::kStorageGeometryWrite | BufferUsageBit::kStorageFragmentWrite
		| BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kStorageTraceRaysWrite
		| BufferUsageBit::kTextureGeometryWrite | BufferUsageBit::kTextureFragmentWrite
		| BufferUsageBit::kTextureComputeWrite | BufferUsageBit::kTextureTraceRaysWrite;

	if(!!(usage & BufferUsageBit::kAllUniform))
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

	if(!!(usage & BufferUsageBit::kIndex))
	{
		mask |= VK_ACCESS_INDEX_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::kVertex))
	{
		mask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::kAllIndirect))
	{
		mask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::kTransferDestination))
	{
		mask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(!!(usage & BufferUsageBit::kTransferSource))
	{
		mask |= VK_ACCESS_TRANSFER_READ_BIT;
	}

	if(!!(usage & BufferUsageBit::kAccelerationStructureBuild))
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

VkBufferView BufferImpl::getOrCreateBufferView(Format fmt, PtrSize offset, PtrSize range) const
{
	if(range == kMaxPtrSize)
	{
		ANKI_ASSERT(m_size >= offset);
		range = m_size - offset;
	}

	// Checks
	ANKI_ASSERT(!!(m_usage & BufferUsageBit::kAllTexture));
	ANKI_ASSERT(offset + range <= m_size);

	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_textureBufferBindOffsetAlignment,
						  m_memHandle.m_offset + offset)
				&& "Offset not aligned");

	ANKI_ASSERT((range % getFormatInfo(fmt).m_texelSize) == 0
				&& "Range doesn't align with the number of texel elements");

	[[maybe_unused]] const PtrSize elementCount = range / getFormatInfo(fmt).m_texelSize;
	ANKI_ASSERT(elementCount <= getGrManagerImpl().getPhysicalDeviceProperties().limits.maxTexelBufferElements);

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
		RLockGuard<RWMutex> lock(m_viewsMtx);

		auto it = m_views.find(hash);
		if(it != m_views.getEnd())
		{
			return *it;
		}
	}

	WLockGuard<RWMutex> lock(m_viewsMtx);

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
	ANKI_VK_CHECKF(vkCreateBufferView(getDevice(), &viewCreateInfo, nullptr, &view));

	m_views.emplace(getMemoryPool(), hash, view);

	return view;
}

} // end namespace anki
