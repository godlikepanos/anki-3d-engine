// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_SYNC_HANDLES_H
#define ANKI_GL_GL_SYNC_HANDLES_H

#include "anki/gr/GlHandle.h"

namespace anki {

// Forward
class GlClientSync;

/// @addtogroup opengl_other
/// @{

/// Client sync handle
class GlClientSyncHandle: public GlHandle<GlClientSync>
{
public:
	using Base = GlHandle<GlClientSync>;

	GlClientSyncHandle();

	~GlClientSyncHandle();

	ANKI_USE_RESULT Error create(CommandBufferHandle& commands);

	/// Fire a command that adds a waits for the client. The client should call 
	/// wait some time after this call or the server will keep waiting forever
	void sync(CommandBufferHandle& commands);

	/// Wait for the server. You need to call sync first or the client will
	/// keep waiting forever
	void wait();
};

/// @}

} // end namespace anki

#endif

