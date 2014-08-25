// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_CONTAINER_HANDLE_H
#define ANKI_GL_GL_CONTAINER_HANDLE_H

#include "anki/gl/GlHandle.h"
#include "anki/gl/GlDevice.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Handle template for container objects
template<typename T>
class GlContainerHandle: public GlHandle<T>
{
public:
	typedef GlHandle<T> Base;

protected:
	/// Check if the object has been created and if not serialize the server
	void serializeOnGetter() const
	{
		GlHandleState state = Base::_getState();
		ANKI_ASSERT(state != GlHandleState::NEW);

		if(state == GlHandleState::TO_BE_CREATED)
		{
			Base::_getManager()._getQueue().syncClientServer();
			ANKI_ASSERT(Base::_getState() > GlHandleState::TO_BE_CREATED);
		}
	}
};

/// @}


} // end namespace anki

#endif

