// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_DEVICE_H
#define ANKI_GL_GL_DEVICE_H

#include "anki/gl/GlCommon.h"
#include "anki/util/Singleton.h"
#include "anki/gl/GlQueue.h"

namespace anki {

/// @addtogroup opengl_other
/// @{

/// Common stuff for all GL contexts
class GlDevice
{
public:
	/// @see GlQueue::start
	GlDevice(AllocAlignedCallback alloc, void* allocUserData, 
		const CString& cacheDir);

	~GlDevice()
	{
		destroy();
	}

	/// Start the queue thread. @see GlQueue::start
	void startServer(
		GlMakeCurrentCallback makeCurrentCb, void* makeCurrentCbData, void* ctx,
		GlCallback swapBuffersCallback, void* swapBuffersCbData,
		Bool registerDebugMessages);

	/// Synchronize client and server
	void syncClientServer();

	/// Swap buffers
	void swapBuffers();

	/// Return the alignment of a buffer target
	PtrSize getBufferOffsetAlignment(GLenum target) const;

	/// @privatesection
	/// @{
	HeapAllocator<U8> _getAllocator() const
	{
		return m_alloc;
	}

	GlQueue& _getQueue() 
	{
		return *m_queue;
	}

	const GlQueue& _getQueue() const
	{
		return *m_queue;
	}

	CString _getCacheDirectory() const
	{
		ANKI_ASSERT(m_cacheDir != nullptr);
		return m_cacheDir;
	}
	/// @}

private:
	GlQueue* m_queue;
	HeapAllocator<U8> m_alloc; ///< Keep it last to be deleted last
	char* m_cacheDir = nullptr;
	Bool8 m_queueStarted = false;

	void destroy();
};

/// @}

} // end namespace anki

#endif
