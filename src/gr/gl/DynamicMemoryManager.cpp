// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/DynamicMemoryManager.h>
#include <anki/core/Config.h>

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
void DynamicMemoryManager::destroy()
{
	for(DynamicBuffer& buff : m_buffers)
	{
		if(buff.m_name != 0)
		{
			glDeleteBuffers(1, &buff.m_name);
		}

		buff.m_cpuBuff.destroy(m_alloc);
	}
}

//==============================================================================
void DynamicMemoryManager::init(
	GenericMemoryPoolAllocator<U8> alloc, const ConfigSet& cfg)
{
	const U BUFF_FLAGS = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

	m_alloc = alloc;

	// Uniform
	{
		// Create buffer
		PtrSize size = cfg.getNumber("gr.uniformPerFrameMemorySize");
		DynamicBuffer& buff = m_buffers[BufferUsage::UNIFORM];
		glGenBuffers(1, &buff.m_name);

		// Map it
		glNamedBufferStorage(buff.m_name, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(
			glMapNamedBufferRange(buff.m_name, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		GLint64 blockAlignment;
		glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		buff.m_frameAlloc.init(size, blockAlignment, MAX_UNIFORM_BLOCK_SIZE);
	}

	// Storage
	{
		// Create buffer
		PtrSize size = cfg.getNumber("gr.storagePerFrameMemorySize");
		DynamicBuffer& buff = m_buffers[BufferUsage::STORAGE];
		glGenBuffers(1, &buff.m_name);

		// Map it
		glNamedBufferStorage(buff.m_name, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(
			glMapNamedBufferRange(buff.m_name, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		GLint64 blockAlignment;
		glGetInteger64v(
			GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		buff.m_frameAlloc.init(size, blockAlignment, MAX_STORAGE_BLOCK_SIZE);
	}

	// Transfer
	{
		// Big enough block to hold a texture surface
		const PtrSize BLOCK_SIZE = (4096 * 4096) / 4 * 16 + 512;

		PtrSize size = cfg.getNumber("gr.transferPersistentMemorySize");
		DynamicBuffer& buff = m_buffers[BufferUsage::TRANSFER];
		buff.m_cpuBuff.create(m_alloc, size);

		buff.m_mappedMem = reinterpret_cast<U8*>(&buff.m_cpuBuff[0]);
		buff.m_persistentAlloc.init(m_alloc, size, BLOCK_SIZE);
	}
}

//==============================================================================
void* DynamicMemoryManager::allocatePerFrame(
	BufferUsage usage, PtrSize size, DynamicBufferToken& handle)
{
	DynamicBuffer& buff = m_buffers[usage];
	Error err = buff.m_frameAlloc.allocate(size, handle, true);
	(void)err;
	return buff.m_mappedMem + handle.m_offset;
}

//==============================================================================
void* DynamicMemoryManager::allocatePersistent(
	BufferUsage usage, PtrSize size, DynamicBufferToken& handle)
{
	DynamicBuffer& buff = m_buffers[usage];
	Error err = buff.m_persistentAlloc.allocate(size, 16, handle, false);
	if(!err)
	{
		return buff.m_mappedMem + handle.m_offset;
	}
	else
	{
		ANKI_LOGW("Out of persistent dynamic memory. Someone should serialize");
		return nullptr;
	}
}

//==============================================================================
void DynamicMemoryManager::freePersistent(
	BufferUsage usage, const DynamicBufferToken& handle)
{
	DynamicBuffer& buff = m_buffers[usage];
	buff.m_persistentAlloc.free(handle);
}

} // end namespace anki