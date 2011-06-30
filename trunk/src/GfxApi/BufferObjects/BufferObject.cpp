#include <cstring>
#include "BufferObject.h"
#include "GfxApi/GlException.h"


//==============================================================================
// create                                                                      =
//==============================================================================
void BufferObject::create(GLenum target_, uint sizeInBytes_, const void* dataPtr, GLenum usage_)
{
	ASSERT(!isCreated());
	// unacceptable usage_
	ASSERT(usage_ == GL_STREAM_DRAW || usage_ == GL_STATIC_DRAW || usage_ == GL_DYNAMIC_DRAW);
	ASSERT(sizeInBytes_ > 0); // unacceptable sizeInBytes

	usage = usage_;
	target = target_;
	sizeInBytes = sizeInBytes_;

	glGenBuffers(1, &glId);
	bind();
	glBufferData(target, sizeInBytes, dataPtr, usage);

	// make a check
	int bufferSize = 0;
	glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
	if(sizeInBytes != (uint)bufferSize)
	{
		deleteBuff();
		throw EXCEPTION("Data size mismatch");
	}

	unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}


//==============================================================================
// write                                                                       =
//==============================================================================
void BufferObject::write(void* buff)
{
	ASSERT(isCreated());
	ASSERT(usage != GL_STATIC_DRAW);
	bind();
	void* mapped = glMapBuffer(target, GL_WRITE_ONLY);
	memcpy(mapped, buff, sizeInBytes);
	glUnmapBuffer(target);
	unbind();
}


//==============================================================================
// write                                                                       =
//==============================================================================
void BufferObject::write(void* buff, size_t offset, size_t size)
{
	ASSERT(isCreated());
	ASSERT(usage != GL_STATIC_DRAW);
	ASSERT(offset + size <= sizeInBytes);
	bind();
	void* mapped = glMapBufferRange(target, offset, size, GL_MAP_WRITE_BIT);
	memcpy(mapped, buff, size);
	glUnmapBuffer(target);
	unbind();
}
