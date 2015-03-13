// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_CONTAINER_HANDLE_H
#define ANKI_GL_GL_CONTAINER_HANDLE_H

#include "anki/gr/GlHandle.h"
#include "anki/gr/GlDevice.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Handle template for container objects
template<typename T>
class GlContainerHandle: public GlHandle<T>
{
public:
	using Base = GlHandle<T>;

protected:
	/// Check if the object has been created and if not serialize the server
	ANKI_USE_RESULT Error serializeOnGetter() const
	{
		Error err = ErrorCode::NONE;
		GlHandleState state = Base::_getState();
		ANKI_ASSERT(state != GlHandleState::NEW);
		
		if(state == GlHandleState::TO_BE_CREATED)
		{
			Base::_getDevice()._getQueue().syncClientServer();

			state = Base::_getState();

			if(state == GlHandleState::ERROR)
			{
				err = ErrorCode::UNKNOWN;
			}
		}

		ANKI_ASSERT(state > GlHandleState::TO_BE_CREATED);

		return err;
	}
};
/// @}


} // end namespace anki

#endif

