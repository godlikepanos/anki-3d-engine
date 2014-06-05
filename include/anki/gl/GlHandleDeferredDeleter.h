// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_HANDLE_DEFERRED_DELETER_H
#define ANKI_GL_GL_HANDLE_DEFERRED_DELETER_H

#include "anki/gl/GlManager.h"
#include "anki/gl/GlJobChainHandle.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Common deleter for objects that require deferred deletion. Some handles
/// might get releaced in various threads but their underlying GL object should 
/// get deleted in the server thread (where the context is). This deleter will
/// fire a server job with the deletion if the handle gets realeased thread 
/// other than the server thread.
template<typename T, typename TAlloc, typename TDeleteJob>
class GlHandleDeferredDeleter
{
public:
	void operator()(T* obj, TAlloc alloc, GlManager* manager)
	{
		ANKI_ASSERT(obj);
		ANKI_ASSERT(manager);
		
		/// If not the server thread then create a job for the server thread
		if(!manager->_getJobManager().isServerThread())
		{
			GlJobChainHandle jobs(manager);
			
			jobs.template _pushBackNewJob<TDeleteJob>(obj, alloc);
			jobs.flush();
		}
		else
		{
			alloc.deleteInstance(obj);
		}
	}
};

/// @}

} // end namespace anki

#endif

