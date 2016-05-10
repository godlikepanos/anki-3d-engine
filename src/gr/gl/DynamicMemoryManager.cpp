// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/DynamicMemoryManager.h>
#include <anki/gr/gl/Error.h>
#include <anki/core/Config.h>
#include <anki/core/Trace.h>

namespace anki
{

//==============================================================================
DynamicMemoryManager::~DynamicMemoryManager()
{
	for(DynamicBuffer& buff : m_buffers)
	{
		ANKI_ASSERT(buff.m_name == 0);
	}
}

//==============================================================================
void DynamicMemoryManager::destroyRenderThread()
{
	for(DynamicBuffer& buff : m_buffers)
	{
		if(buff.m_name != 0)
		{
			glDeleteBuffers(1, &buff.m_name);
			buff.m_name = 0;
		}

		buff.m_cpuBuff.destroy(m_alloc);
	}
}

//==============================================================================
void DynamicMemoryManager::initMainThread(
	GenericMemoryPoolAllocator<U8> alloc, const ConfigSet& cfg)
{
	m_alloc = alloc;

	m_buffers[BufferUsage::UNIFORM].m_size =
		cfg.getNumber("gr.uniformPerFrameMemorySize");

	m_buffers[BufferUsage::STORAGE].m_size =
		cfg.getNumber("gr.storagePerFrameMemorySize");

	m_buffers[BufferUsage::VERTEX].m_size =
		cfg.getNumber("gr.vertexPerFrameMemorySize");

	m_buffers[BufferUsage::TRANSFER].m_size =
		cfg.getNumber("gr.transferPerFrameMemorySize");
}

//==============================================================================
void DynamicMemoryManager::initRenderThread()
{
	const U BUFF_FLAGS = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

	// Uniform
	{
		// Create buffer
		DynamicBuffer& buff = m_buffers[BufferUsage::UNIFORM];
		PtrSize size = buff.m_size;
		glGenBuffers(1, &buff.m_name);
		glBindBuffer(GL_UNIFORM_BUFFER, buff.m_name);

		// Map it
		glBufferStorage(GL_UNIFORM_BUFFER, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(
			glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		GLint64 blockAlignment;
		glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		buff.m_frameAlloc.init(size, blockAlignment, MAX_UNIFORM_BLOCK_SIZE);
	}

	// Storage
	{
		// Create buffer
		DynamicBuffer& buff = m_buffers[BufferUsage::STORAGE];
		PtrSize size = buff.m_size;
		glGenBuffers(1, &buff.m_name);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buff.m_name);

		// Map it
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(
			glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		GLint64 blockAlignment;
		glGetInteger64v(
			GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		buff.m_frameAlloc.init(size, blockAlignment, MAX_STORAGE_BLOCK_SIZE);
	}

	// Vertex
	{
		// Create buffer
		DynamicBuffer& buff = m_buffers[BufferUsage::VERTEX];
		PtrSize size = buff.m_size;
		glGenBuffers(1, &buff.m_name);
		glBindBuffer(GL_ARRAY_BUFFER, buff.m_name);

		// Map it
		glBufferStorage(GL_ARRAY_BUFFER, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(
			glMapBufferRange(GL_ARRAY_BUFFER, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		buff.m_frameAlloc.init(size, 16, MAX_U32);
	}

	// Transfer
	{
		DynamicBuffer& buff = m_buffers[BufferUsage::TRANSFER];
		PtrSize size = buff.m_size;
		buff.m_cpuBuff.create(m_alloc, size);

		buff.m_mappedMem = reinterpret_cast<U8*>(&buff.m_cpuBuff[0]);
		buff.m_frameAlloc.init(size, 16, MAX_U32);
	}
}

//==============================================================================
void* DynamicMemoryManager::allocatePerFrame(
	BufferUsage usage, PtrSize size, DynamicBufferToken& handle)
{
	DynamicBuffer& buff = m_buffers[usage];
	Error err = buff.m_frameAlloc.allocate(size, handle, false);
	if(!err)
	{
		return buff.m_mappedMem + handle.m_offset;
	}
	else
	{
		ANKI_LOGW(
			"Out of per-frame GPU memory. Someone will have to handle this");
		return nullptr;
	}
}

//==============================================================================
void DynamicMemoryManager::endFrame()
{
	for(BufferUsage usage = BufferUsage::FIRST; usage < BufferUsage::COUNT;
		++usage)
	{
		DynamicBuffer& buff = m_buffers[usage];

		if(buff.m_mappedMem)
		{
			// Increase the counters
			switch(usage)
			{
			case BufferUsage::UNIFORM:
				ANKI_TRACE_INC_COUNTER(GR_DYNAMIC_UNIFORMS_SIZE,
					buff.m_frameAlloc.getUnallocatedMemorySize());
				break;
			case BufferUsage::STORAGE:
				ANKI_TRACE_INC_COUNTER(GR_DYNAMIC_STORAGE_SIZE,
					buff.m_frameAlloc.getUnallocatedMemorySize());
				break;
			default:
				break;
			}

			buff.m_frameAlloc.endFrame();
		}
	}
}

} // end namespace anki