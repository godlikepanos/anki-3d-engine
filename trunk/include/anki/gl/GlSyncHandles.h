// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_SYNC_HANDLES_H
#define ANKI_GL_GL_SYNC_HANDLES_H

#include "anki/gl/GlHandle.h"

namespace anki {

// Forward
class GlClientSync;
class GlJobChainHandle;

/// @addtogroup opengl_other
/// @{

/// Client sync handle
class GlClientSyncHandle: public GlHandle<GlClientSync>
{
public:
	typedef GlHandle<GlClientSync> Base;

	GlClientSyncHandle();

	GlClientSyncHandle(GlJobChainHandle& jobs);

	~GlClientSyncHandle();

	/// Fire a job that adds a waits for the client. The client should call 
	/// wait some time after this call or the server will keep waiting forever
	void sync(GlJobChainHandle& jobs);

	/// Wait for the server. You need to call sync first or the client will
	/// keep waiting forever
	void wait();
};

/// @}

} // end namespace anki

#endif

