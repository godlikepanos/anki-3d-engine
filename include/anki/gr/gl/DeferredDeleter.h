// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_DEFERRED_DELETER_H
#define ANKI_GR_GL_DEFERRED_DELETER_H

#include "anki/gr/GrManager.h"
#include "anki/gr/CommandBufferHandle.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/gl/RenderingThread.h"
#include "anki/gr/gl/GrManagerImpl.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Common deleter for objects that require deferred deletion. Some handles
/// might get releaced in various threads but their underlying GL object should 
/// get deleted in the server thread (where the context is). This deleter will
/// fire a server command with the deletion if the handle gets realeased thread 
/// other than the server thread.
template<typename T, typename TDeleteCommand>
class DeferredDeleter
{
public:
	void operator()(T* obj)
	{
		ANKI_ASSERT(obj);
		GrManager& manager = obj->getManager();
		const RenderingThread& thread = 
			manager.getImplementation().getRenderingThread();
		
		/// If not the server thread then create a command for the server thread
		if(!thread.isServerThread())
		{
			CommandBufferHandle commands;
			
			Error err = commands.create(&manager);
			if(!err)
			{
				commands.get().template pushBackNewCommand<TDeleteCommand>(obj);
				commands.flush();
			}
			else
			{
				ANKI_LOGE("Failed to create command");
			}
		}
		else
		{
			auto alloc = obj->getAllocator();
			alloc.deleteInstance(obj);
		}
	}
};
/// @}

} // end namespace anki

#endif

