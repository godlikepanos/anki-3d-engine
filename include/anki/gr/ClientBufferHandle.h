// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_CLIENT_BUFFER_HANDLE_H
#define ANKI_GL_GL_CLIENT_BUFFER_HANDLE_H

#include "anki/gr/GlHandle.h"

namespace anki {

/// @addtogroup opengl_other
/// @{

/// Client buffer handle
class ClientBufferHandle: public GlHandle<ClientBufferImpl>
{
public:
	using Base = GlHandle<ClientBufferImpl>;

	ClientBufferHandle();

	~ClientBufferHandle();

	/// Create the buffer using preallocated memory or memory from the chain
	/// @param chain The command buffer this client memory belongs to
	/// @param size The size of the buffer
	/// @param preallocatedMem Preallocated memory. Can be nullptr
	/// @return The address of the new memory
	ANKI_USE_RESULT Error create(
		CommandBufferHandle& chain, PtrSize size, void* preallocatedMem);

	/// Get the base address
	void* getBaseAddress();

	/// Get size
	PtrSize getSize() const;
};

/// @}

} // end namespace anki

#endif
