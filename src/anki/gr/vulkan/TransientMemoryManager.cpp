// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TransientMemoryManager.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/core/Config.h>
#include <anki/core/Trace.h>

namespace anki
{

Error TransientMemoryManager::init(const ConfigSet& cfg)
{
	Array<const char*, U(TransientBufferType::COUNT)> configVars = {{"gr.uniformPerFrameMemorySize",
		"gr.storagePerFrameMemorySize",
		"gr.vertexPerFrameMemorySize",
		"gr.transferPerFrameMemorySize"}};

	// This alignment satisfies the spec's condition for buffer to image copies: "bufferOffset must be a multiple of the
	// calling command's VkImage parameter's texel size". This alignment works for all supported formats
	const U TRANSFER_ALIGNMENT = 96;

	const VkPhysicalDeviceLimits& limits = m_manager->getImplementation().getPhysicalDeviceProperties().limits;
	Array<U32, U(TransientBufferType::COUNT)> alignments = {{U32(limits.minUniformBufferOffsetAlignment),
		U32(limits.minStorageBufferOffsetAlignment),
		sizeof(F32) * 4,
		TRANSFER_ALIGNMENT}};

	Array<BufferUsageBit, U(TransientBufferType::COUNT)> usages = {{BufferUsageBit::UNIFORM_ALL,
		BufferUsageBit::STORAGE_ALL,
		BufferUsageBit::VERTEX,
		BufferUsageBit::BUFFER_UPLOAD_SOURCE | BufferUsageBit::BUFFER_UPLOAD_DESTINATION
			| BufferUsageBit::TEXTURE_UPLOAD_SOURCE}};

	auto alloc = m_manager->getAllocator();
	for(TransientBufferType i = TransientBufferType::UNIFORM; i < TransientBufferType::COUNT; ++i)
	{
		PerFrameBuffer& frame = m_perFrameBuffers[i];
		frame.m_size = cfg.getNumber(configVars[i]);
		ANKI_ASSERT(frame.m_size);

		// Create buffer
		frame.m_buff = alloc.newInstance<BufferImpl>(m_manager);
		ANKI_CHECK(frame.m_buff->init(frame.m_size, usages[i], BufferMapAccessBit::WRITE));

		frame.m_bufferHandle = frame.m_buff->getHandle();

		// Map once
		frame.m_mappedMem = static_cast<U8*>(frame.m_buff->map(0, frame.m_size, BufferMapAccessBit::WRITE));
		ANKI_ASSERT(frame.m_mappedMem);

		// Init the allocator
		frame.m_alloc.init(frame.m_size, alignments[i]);
	}

	return ErrorCode::NONE;
}

void TransientMemoryManager::destroy()
{
	for(PerFrameBuffer& frame : m_perFrameBuffers)
	{
		if(frame.m_buff)
		{
			frame.m_buff->unmap();
			m_manager->getAllocator().deleteInstance(frame.m_buff);
		}
	}
}

void TransientMemoryManager::allocate(
	PtrSize size, BufferUsageBit usage, TransientMemoryToken& token, void*& ptr, Error* outErr)
{
	Error err = ErrorCode::NONE;
	ptr = nullptr;

	PerFrameBuffer& buff = m_perFrameBuffers[bufferUsageToTransient(usage)];
	err = buff.m_alloc.allocate(size, token.m_offset);

	if(!err)
	{
		token.m_usage = usage;
		token.m_range = size;
		token.m_lifetime = TransientMemoryTokenLifetime::PER_FRAME;
		ptr = buff.m_mappedMem + token.m_offset;
	}
	else if(outErr)
	{
		*outErr = err;
	}
	else
	{
		ANKI_LOGF("Out of transient GPU memory");
	}
}

void TransientMemoryManager::endFrame()
{
	for(TransientBufferType usage = TransientBufferType::UNIFORM; usage < TransientBufferType::COUNT; ++usage)
	{
		PerFrameBuffer& buff = m_perFrameBuffers[usage];

		if(buff.m_mappedMem)
		{
			// Increase the counters
			switch(usage)
			{
			case TransientBufferType::UNIFORM:
				ANKI_TRACE_INC_COUNTER(GR_DYNAMIC_UNIFORMS_SIZE, buff.m_alloc.getUnallocatedMemorySize());
				break;
			case TransientBufferType::STORAGE:
				ANKI_TRACE_INC_COUNTER(GR_DYNAMIC_STORAGE_SIZE, buff.m_alloc.getUnallocatedMemorySize());
				break;
			default:
				break;
			}

			buff.m_alloc.endFrame();
		}
	}
}

} // end namespace anki
