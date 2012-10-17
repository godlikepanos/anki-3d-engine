#include <cstring>
#include "anki/gl/BufferObject.h"
#include "anki/gl/GlException.h"
#include "anki/util/Exception.h"

namespace anki {

//==============================================================================
BufferObject::~BufferObject()
{
	if(isCreated())
	{
		destroy();
	}
}

//==============================================================================
void BufferObject::create(GLenum target_, U32 sizeInBytes_,
	const void* dataPtr, GLenum usage_)
{
	ANKI_ASSERT(!isCreated());

	ANKI_ASSERT(usage_ == GL_STREAM_DRAW
		|| usage_ == GL_STATIC_DRAW
		|| usage_ == GL_DYNAMIC_DRAW);

	ANKI_ASSERT(sizeInBytes_ > 0 && "Unacceptable sizeInBytes");

	usage = usage_;
	target = target_;
	sizeInBytes = sizeInBytes_;

	glGenBuffers(1, &glId);
	ANKI_ASSERT(glId != 0);
	bind();
	glBufferData(target, sizeInBytes, dataPtr, usage);

	// make a check
	GLint bufferSize = 0;
	glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
	if(sizeInBytes != (U32)bufferSize)
	{
		destroy();
		throw ANKI_EXCEPTION("Data size mismatch");
	}

	unbind();
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void* BufferObject::map(U32 offset, U32 length, GLuint flags)
{
	ANKI_ASSERT(isCreated());
#if ANKI_DEBUG
	ANKI_ASSERT(mapped == false);
#endif
	bind();
	ANKI_ASSERT(offset + length <= sizeInBytes);
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

	void* mapped = map(offset, size, 
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	
	memcpy(mapped, buff, size);
	unmap();
}

} // end namespace anki
