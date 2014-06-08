// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_MANAGER_H
#define ANKI_GL_GL_MANAGER_H

#include "anki/gl/GlCommon.h"
#include "anki/util/Singleton.h"
#include "anki/gl/GlJobManager.h"

namespace anki {

/// @addtogroup opengl_other
/// @{

/// Common stuff for all GL contexts
class GlManager
{
public:
	/// @see GlJobManager::start
	GlManager(
		GlCallback makeCurrentCallback, void* context,
		GlCallback swapBuffersCallback, void* swapBuffersCbData,
		Bool registerDebugMessages,
		AllocAlignedCallback alloc, void* allocUserData);

	~GlManager()
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

	GlJobManager& _getJobManager() 
	{
		return *m_jobManager;
	}

	const GlJobManager& _getJobManager() const
	{
		return *m_jobManager;
	}
	/// @}

private:
	GlJobManager* m_jobManager;
	HeapAllocator<U8> m_alloc; ///< Keep it last to be deleted last

	void destroy();
};

/// Singleton for common GL stuff
typedef SingletonInit<GlManager> GlManagerSingleton;

/// @}

} // end namespace anki

#endif
