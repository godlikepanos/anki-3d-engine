// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DFence.h>

namespace anki {

Fence* Fence::newInstance()
{
	return anki::newInstance<FenceImpl>(GrMemoryPool::getSingleton(), "N/A");
}

Bool Fence::clientWait(Second seconds)
{
	ANKI_ASSERT(!"TODO");
	return false;
}

} // end namespace anki
