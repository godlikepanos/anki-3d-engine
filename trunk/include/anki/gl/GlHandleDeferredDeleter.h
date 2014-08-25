// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_HANDLE_DEFERRED_DELETER_H
#define ANKI_GL_GL_HANDLE_DEFERRED_DELETER_H

#include "anki/gl/GlDevice.h"
#include "anki/gl/GlCommandBufferHandle.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Common deleter for objects that require deferred deletion. Some handles
/// might get releaced in various threads but their underlying GL object should 
/// get deleted in the server thread (where the context is). This deleter will
/// fire a server command with the deletion if the handle gets realeased thread 
/// other than the server thread.
template<typename T, typename TAlloc, typename TDeleteCommand>
class GlHandleDeferredDeleter
{
public:
	void operator()(T* obj, TAlloc alloc, GlDevice* manager)
	{
		ANKI_ASSERT(obj);
		ANKI_ASSERT(manager);
		
		/// If not the server thread then create a command for the server thread
		if(!manager->_getQueue().isServerThread())
		{
			GlCommandBufferHandle commands(manager);
			
			commands.template _pushBackNewCommand<TDeleteCommand>(obj, alloc);
			commands.flush();
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

