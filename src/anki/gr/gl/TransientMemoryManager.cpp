// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/TransientMemoryManager.h>
#include <anki/gr/gl/Error.h>
#include <anki/core/Config.h>
#include <anki/core/Trace.h>

namespace anki
{

TransientMemoryManager::~TransientMemoryManager()
{
	for(PerFrameBuffer& buff : m_perFrameBuffers)
	{
		ANKI_ASSERT(buff.m_name == 0);
		(void)buff;
	}
}

void TransientMemoryManager::destroyRenderThread()
{
	for(PerFrameBuffer& buff : m_perFrameBuffers)
	{
		if(buff.m_name != 0)
		{
			glDeleteBuffers(1, &buff.m_name);
			buff.m_name = 0;
		}

		buff.m_cpuBuff.destroy(m_alloc);
	}
}

void TransientMemoryManager::initMainThread(GenericMemoryPoolAllocator<U8> alloc, const ConfigSet& cfg)
{
	m_alloc = alloc;

	m_perFrameBuffers[TransientBufferType::UNIFORM].m_size = cfg.getNumber("gr.uniformPerFrameMemorySize");

	m_perFrameBuffers[TransientBufferType::STORAGE].m_size = cfg.getNumber("gr.storagePerFrameMemorySize");

	m_perFrameBuffers[TransientBufferType::VERTEX].m_size = cfg.getNumber("gr.vertexPerFrameMemorySize");

	m_perFrameBuffers[TransientBufferType::TRANSFER].m_size = cfg.getNumber("gr.transferPerFrameMemorySize");
}

void TransientMemoryManager::initRenderThread()
{
	const U BUFF_FLAGS = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

	// Uniform
	{
		// Create buffer
		PerFrameBuffer& buff = m_perFrameBuffers[TransientBufferType::UNIFORM];
		PtrSize size = buff.m_size;
		glGenBuffers(1, &buff.m_name);
		glBindBuffer(GL_UNIFORM_BUFFER, buff.m_name);

		// Map it
		glBufferStorage(GL_UNIFORM_BUFFER, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		GLint64 blockAlignment;
		glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		buff.m_alloc.init(size, blockAlignment, MAX_UNIFORM_BLOCK_SIZE);
	}

	// Storage
	{
		// Create buffer
		PerFrameBuffer& buff = m_perFrameBuffers[TransientBufferType::STORAGE];
		PtrSize size = buff.m_size;
		glGenBuffers(1, &buff.m_name);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buff.m_name);

		// Map it
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		GLint64 blockAlignment;
		glGetInteger64v(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		buff.m_alloc.init(size, blockAlignment, MAX_STORAGE_BLOCK_SIZE);
	}

	// Vertex
	{
		// Create buffer
		PerFrameBuffer& buff = m_perFrameBuffers[TransientBufferType::VERTEX];
		PtrSize size = buff.m_size;
		glGenBuffers(1, &buff.m_name);
		glBindBuffer(GL_ARRAY_BUFFER, buff.m_name);

		// Map it
		glBufferStorage(GL_ARRAY_BUFFER, size, nullptr, BUFF_FLAGS);
		buff.m_mappedMem = static_cast<U8*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, size, BUFF_FLAGS));
		ANKI_ASSERT(buff.m_mappedMem);

		// Create the allocator
		buff.m_alloc.init(size, 16, MAX_U32);
	}

	// Transfer
	{
		PerFrameBuffer& buff = m_perFrameBuffers[TransientBufferType::TRANSFER];
		PtrSize size = buff.m_size;
		buff.m_cpuBuff.create(m_alloc, size);

		buff.m_mappedMem = reinterpret_cast<U8*>(&buff.m_cpuBuff[0]);
		buff.m_alloc.init(size, 16, MAX_U32);
	}
}

void TransientMemoryManager::allocate(PtrSize size,
	BufferUsageBit usage,
	TransientMemoryTokenLifetime lifespan,
	TransientMemoryToken& token,
	void*& ptr,
	Error* outErr)
{
	Error err = ErrorCode::NONE;
	ptr = nullptr;
	U8* mappedMemBase;

	if(lifespan == TransientMemoryTokenLifetime::PER_FRAME)
	{
		PerFrameBuffer& buff = m_perFrameBuffers[bufferUsageToTransient(usage)];
		err = buff.m_alloc.allocate(size, token.m_offset);
		mappedMemBase = buff.m_mappedMem;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	if(!err)
	{
		token.m_usage = usage;
		token.m_range = size;
		token.m_lifetime = lifespan;
		ptr = mappedMemBase + token.m_offset;
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
