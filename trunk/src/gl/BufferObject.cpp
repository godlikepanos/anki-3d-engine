#include <cstring>
#include "anki/gl/BufferObject.h"
#include "anki/gl/GlException.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================

/// Instead of map/unmap use glBufferSubData() when writing to the whole buffer
#define USE_BUFFER_DATA_ON_WRITE 1

//==============================================================================
// BufferObject                                                                =
//==============================================================================

//==============================================================================
BufferObject& BufferObject::operator=(BufferObject&& b)
{
	destroy();
	Base::operator=(std::forward<Base>(b));
	target = b.target;
	usage = b.usage;
	sizeInBytes = b.sizeInBytes;
#if ANKI_DEBUG
	mapped = b.mapped;
#endif
	return *this;
}

//==============================================================================
void BufferObject::destroy()
{
	if(isCreated())
	{
		unbind();
		glDeleteBuffers(objectsCount, &glIds[0]);
		memset(&glIds[0], 0, sizeof(Base::glIds));
		objectsCount = 1;
	}
}

//==============================================================================
void BufferObject::create(GLenum target_, U32 sizeInBytes_,
	const void* dataPtr, GLenum usage_, U objectsCount_)
{
	ANKI_ASSERT(!isCreated());

	if(target_ == GL_UNIFORM_BUFFER)
	{
		GLint64 maxBufferSize;
		glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxBufferSize);
		if(sizeInBytes_ > (PtrSize)maxBufferSize)
		{
			throw ANKI_EXCEPTION("Buffer size exceeds GL implementation max");
		}
	}

	usage = usage_;
	target = target_;
	sizeInBytes = sizeInBytes_;
	objectsCount = objectsCount_;

	ANKI_ASSERT(sizeInBytes > 0 && "Unacceptable sizeInBytes");
	ANKI_ASSERT(!(target == GL_UNIFORM_BUFFER && usage != GL_DYNAMIC_DRAW)
		&& "Don't use UBOs like that");
	ANKI_ASSERT(objectsCount > 0 && objectsCount < MAX_OBJECTS);
	ANKI_ASSERT(!(objectsCount > 1 && dataPtr != nullptr)
		&& "Multibuffering with data is not making sence");

	// Create
	glGenBuffers(objectsCount, &glIds[0]);

	for(U i = 0; i < objectsCount; i++)
	{
		glBindBuffer(target, glIds[i]);
		glBufferData(target, sizeInBytes, dataPtr, usage);

		// make a check
		GLint bufferSize = 0;
		glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
		if(sizeInBytes != (U32)bufferSize)
		{
			destroy();
			throw ANKI_EXCEPTION("Data size mismatch");
		}
	}

	glBindBuffer(target, 0);
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void* BufferObject::map(U32 offset, U32 length, GLuint flags)
{
	// XXX Remove this workaround
#if ANKI_GL == ANKI_GL_ES
	flags &= ~GL_MAP_INVALIDATE_BUFFER_BIT;
#endif

	// Precoditions
	ANKI_ASSERT(isCreated());
#if ANKI_DEBUG
	ANKI_ASSERT(mapped == false);
#endif
	ANKI_ASSERT(length > 0);
	ANKI_ASSERT(offset + length <= sizeInBytes);

	// Do the mapping
	bind();
	void* mappedMem = glMapBufferRange(target, offset, length, flags);
	ANKI_ASSERT(mappedMem != nullptr);
#if ANKI_DEBUG
	mapped = true;
#endif
	return mappedMem;
}

//==============================================================================
void BufferObject::unmap()
{
	ANKI_ASSERT(isCreated());
#if ANKI_DEBUG
	ANKI_ASSERT(mapped == true);
#endif
	bind();
	glUnmapBuffer(target);
#if ANKI_DEBUG
	mapped = false;
#endif
}

//==============================================================================
void BufferObject::write(void* buff, U32 offset, U32 size)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(usage != GL_STATIC_DRAW);
	ANKI_ASSERT(offset + size <= sizeInBytes);
	bind();

#if USE_BUFFER_DATA_ON_WRITE
	glBufferSubData(target, offset, sizeInBytes, buff);
#else
	void* mapped = map(offset, size, 
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT 
		| GL_MAP_UNSYNCHRONIZED_BIT);
	
	memcpy(mapped, buff, size);
	unmap();
#endif
}

//==============================================================================
void BufferObject::read(void* outBuff, U32 offset, U32 size)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(usage != GL_STATIC_DRAW);
	ANKI_ASSERT(offset + size <= sizeInBytes);
	bind();

	void* mapped = map(offset, size, GL_MAP_READ_BIT);
	memcpy(outBuff, mapped, size);
	unmap();
}

//==============================================================================
void BufferObject::setBinding(GLuint binding) const
{
	ANKI_ASSERT(isCreated());
	glBindBufferBase(target, binding, getGlId());
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void BufferObject::setBindingRange(
	GLuint binding, PtrSize offset, PtrSize size) const
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(offset + size <= sizeInBytes);
	ANKI_ASSERT(size > 0);

	glBindBufferRange(target, binding, getGlId(), offset, size);
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
// Ubo                                                                         =
//==============================================================================

//==============================================================================
void Ubo::create(PtrSize size, const void* data, U objectCount)
{
	BufferObject::create(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW,
		objectCount);
}

} // end namespace anki
