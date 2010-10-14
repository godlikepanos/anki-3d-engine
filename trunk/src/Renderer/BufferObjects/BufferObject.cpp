#include "BufferObject.h"


//======================================================================================================================
// create                                                                                                              =
//======================================================================================================================
void BufferObject::create(GLenum target_, uint sizeInBytes, const void* dataPtr, GLenum usage_)
{
	RASSERT_THROW_EXCEPTION(isCreated()); // BO already initialized
	// unacceptable usage_
	RASSERT_THROW_EXCEPTION(usage_ != GL_STREAM_DRAW && usage_ != GL_STATIC_DRAW && usage_ != GL_DYNAMIC_DRAW);
	RASSERT_THROW_EXCEPTION(sizeInBytes < 1); // unacceptable sizeInBytes

	usage = usage_;
	target = target_;

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
}
