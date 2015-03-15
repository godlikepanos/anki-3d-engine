// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/GrObject.h"
#include "anki/gr/GrManager.h"

namespace anki {

//==============================================================================
GrAllocator<U8> GrObject::getAllocator() const
{
	m_manager->getAllocator();
}

} // end namespace anki
