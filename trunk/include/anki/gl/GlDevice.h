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
	GlDevice(
		GlCallback makeCurrentCallback, void* context,
		GlCallback swapBuffersCallback, void* swapBuffersCbData,
		Bool registerDebugMessages,
		AllocAlignedCallback alloc, void* allocUserData);

	~GlDevice()
	{
		destroy();
	}

	/// Synchronize client and server
	void syncClientServer();

	/// Swap buffers
	void swapBuffers();

	/// Return the alignment of a buffer target
	PtrSize getBufferOffsetAlignment(GLenum target);

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
	/// @}

private:
	GlQueue* m_queue;
	HeapAllocator<U8> m_alloc; ///< Keep it last to be deleted last

	void destroy();
};

/// Singleton for common GL stuff
typedef SingletonInit<GlDevice> GlDeviceSingleton;

/// @}

} // end namespace anki

#endif
