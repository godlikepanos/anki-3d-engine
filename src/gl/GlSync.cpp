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
	m_barrier.wait();
}

} // end namespace anki
