// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"

namespace anki {

//==============================================================================
Error GlObject::serializeOnGetter() const
{
	Error err = ErrorCode::NONE;
	State state = State(m_state.load());
	ANKI_ASSERT(state != State::NEW);
	
	if(state == State::TO_BE_CREATED)
	{
		RenderingThread& thread = const_cast<RenderingThread&>(
			getManager().getImplementation().getRenderingThread());
		thread.syncClientServer();

		state = State(m_state.load());

		if(state == State::ERROR)
		{
			err = ErrorCode::UNKNOWN;
		}
	}

	ANKI_ASSERT(state > State::TO_BE_CREATED);

	return err;
}

} // end namespace anki


