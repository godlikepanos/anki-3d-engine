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
	void init(
		GlCallback makeCurrentCallback, void* context,
		GlCallback swapBuffersCallback, void* swapBuffersCbData,
		Bool registerDebugMessages);

	void destroy();

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
	std::unique_ptr<GlJobManager> m_jobManager;
	HeapAllocator<U8> m_alloc; ///< Keep it last to be deleted last
};

/// Singleton for common GL stuff
typedef Singleton<GlManager> GlManagerSingleton;

/// @}

} // end namespace anki

#endif
