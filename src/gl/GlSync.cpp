// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlSync.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
void GlClientSync::wait()
{
	Bool timeout = m_barrier.wait();
	if(timeout)
	{
		ANKI_LOGW("Sync timed out. Probably because of exception");
	}
}

} // end namespace anki
