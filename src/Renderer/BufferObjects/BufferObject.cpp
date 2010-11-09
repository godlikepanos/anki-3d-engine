#include <cstring>
#include "BufferObject.h"
#include "GlException.h"


//======================================================================================================================
// create                                                                                                              =
//======================================================================================================================
void BufferObject::create(GLenum target_, uint sizeInBytes_, const void* dataPtr, GLenum usage_)
{
	// unacceptable usage_
	RASSERT_THROW_EXCEPTION(usage_ != GL_STREAM_DRAW && usage_ != GL_STATIC_DRAW && usage_ != GL_DYNAMIC_DRAW);
	RASSERT_THROW_EXCEPTION(sizeInBytes < 1); // unacceptable sizeInBytes

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


//======================================================================================================================
// write                                                                                                               =
//======================================================================================================================
void BufferObject::write(void* buff, size_t size)
{
	if(usage == GL_STATIC_DRAW)
	{
		throw EXCEPTION("Its not recomended to map GL_STATIC_DRAW BOs");
	}

	bind();

	int bufferSize = 0;
	glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
	if(size != uint(bufferSize))
	{
		throw EXCEPTION("Data size mismatch");
	}

	void* mapped = glMapBuffer(target, GL_WRITE_ONLY);
	memcpy(mapped, buff, size);
	glUnmapBuffer(target);
	unbind();
}
