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

void TransientMemoryManager::initBuffer(TransientBufferType type, U32 alignment, PtrSize maxAllocSize, GLenum target)
{
	const U BUFF_FLAGS = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

	// Create buffer
	PerFrameBuffer& buff = m_perFrameBuffers[type];
	glGenBuffers(1, &buff.m_name);
	glBindBuffer(target, buff.m_name);

	// Map it
	glBufferStorage(target, buff.m_size, nullptr, BUFF_FLAGS);
	buff.m_mappedMem = static_cast<U8*>(glMapBufferRange(target, 0, buff.m_size, BUFF_FLAGS));
	ANKI_ASSERT(buff.m_mappedMem);

	// Create the allocator
	buff.m_alloc.init(buff.m_size, alignment, maxAllocSize);

	glBindBuffer(target, 0);
}

void TransientMemoryManager::initRenderThread()
{
	// Uniform
	{
		GLint64 blockAlignment;
		glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		initBuffer(TransientBufferType::UNIFORM, blockAlignment, MAX_UNIFORM_BLOCK_SIZE, GL_UNIFORM_BUFFER);
	}

	// Storage
	{
		GLint64 blockAlignment;
		glGetInteger64v(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
		initBuffer(TransientBufferType::STORAGE, blockAlignment, MAX_STORAGE_BLOCK_SIZE, GL_SHADER_STORAGE_BUFFER);
	}

	// Vertex
	initBuffer(TransientBufferType::VERTEX, 16, MAX_U32, GL_ARRAY_BUFFER);

	// Transfer
	initBuffer(TransientBufferType::TRANSFER, 16, MAX_U32, GL_PIXEL_UNPACK_BUFFER);
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
