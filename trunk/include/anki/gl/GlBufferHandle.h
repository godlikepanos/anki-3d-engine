// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_BUFFER_HANDLE_H
#define ANKI_GL_GL_BUFFER_HANDLE_H

#include "anki/gl/GlContainerHandle.h"

namespace anki {

// Forward
class GlBuffer;
class GlClientBufferHandle;
class GlCommandBufferHandle;

/// @addtogroup opengl_containers
/// @{

/// GPU buffer handle
class GlBufferHandle: public GlContainerHandle<GlBuffer>
{
public:
	using Base = GlContainerHandle<GlBuffer>;

	GlBufferHandle();

	/// Create the buffer with data
	explicit GlBufferHandle(GlCommandBufferHandle& commands, GLenum target, 
		GlClientBufferHandle& data, GLbitfield flags);

	/// Create the buffer without data
	explicit GlBufferHandle(GlCommandBufferHandle& commands, GLenum target, 
		PtrSize size, GLbitfield flags);

	~GlBufferHandle();

	/// Get buffer size. It may serialize 
	PtrSize getSize() const;

	/// Get buffer's current target. It may serialize 
	GLenum getTarget() const;

	/// Get persistent mapping address. It may serialize 
	void* getPersistentMappingAddress();

	/// Write data to the buffer
	void write(
		GlCommandBufferHandle& commands, 
		GlClientBufferHandle& data, PtrSize readOffset,
		PtrSize writeOffset, PtrSize size);

	/// Bind to the state as uniform/shader storage buffer
	void bindShaderBuffer(GlCommandBufferHandle& commands, U32 bindingPoint)
	{
		bindShaderBufferInternal(commands, -1, -1, bindingPoint);
	}

	/// Bind to the state as uniform/shader storage buffer
	void bindShaderBuffer(GlCommandBufferHandle& commands,
		U32 offset, U32 size, U32 bindingPoint)
	{
		bindShaderBufferInternal(commands, offset, size, bindingPoint);
	}

	/// Bind to the state as vertex buffer
	void bindVertexBuffer(
		GlCommandBufferHandle& commands, 
		U32 elementSize,
		GLenum type,
		Bool normalized,
		PtrSize stride,
		PtrSize offset,
		U32 attribLocation);

	/// Bind to the state as index buffer
	void bindIndexBuffer(GlCommandBufferHandle& commands);

private:
	void bindShaderBufferInternal(GlCommandBufferHandle& commands,
		I32 offset, I32 size, U32 bindingPoint);
};

/// @}

} // end namespace anki

#endif

