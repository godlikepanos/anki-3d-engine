// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_BUFFER_HANDLE_H
#define ANKI_GR_BUFFER_HANDLE_H

#include "anki/gr/GrHandle.h"

namespace anki {

/// @addtogroup opengl_containers
/// @{

/// GPU buffer handle
class BufferHandle: public GrHandle<BufferImpl>
{
public:
	using Base = GrHandle<BufferImpl>;

	BufferHandle();

	~BufferHandle();

	/// Create the buffer with data
	ANKI_USE_RESULT Error create(CommandBufferHandle& commands, GLenum target, 
		const void* data, PtrSize size, GLbitfield flags);

	/// Get buffer size. It may serialize 
	PtrSize getSize() const;

	/// Get buffer's current target. It may serialize 
	GLenum getTarget() const;

	/// Get persistent mapping address. It may serialize 
	void* getPersistentMappingAddress();

	/// Write data to the buffer
	void write(
		CommandBufferHandle& commands, 
		ClientBufferHandle& data, PtrSize readOffset,
		PtrSize writeOffset, PtrSize size);

	/// Bind to the state as uniform/shader storage buffer
	void bindShaderBuffer(CommandBufferHandle& commands, U32 bindingPoint)
	{
		bindShaderBufferInternal(commands, -1, -1, bindingPoint);
	}

	/// Bind to the state as uniform/shader storage buffer
	void bindShaderBuffer(CommandBufferHandle& commands,
		U32 offset, U32 size, U32 bindingPoint)
	{
		bindShaderBufferInternal(commands, offset, size, bindingPoint);
	}

	/// Bind to the state as vertex buffer
	void bindVertexBuffer(
		CommandBufferHandle& commands, 
		U32 elementSize,
		GLenum type,
		Bool normalized,
		PtrSize stride,
		PtrSize offset,
		U32 attribLocation);

	/// Bind to the state as index buffer
	void bindIndexBuffer(CommandBufferHandle& commands);

private:
	void bindShaderBufferInternal(CommandBufferHandle& commands,
		I32 offset, I32 size, U32 bindingPoint);
};

/// @}

} // end namespace anki

#endif

