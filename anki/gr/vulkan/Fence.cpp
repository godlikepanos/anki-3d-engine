// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Fence.h>
#include <anki/gr/vulkan/FenceImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Fence* Fence::newInstance(GrManager* manager)
{
	return manager->getAllocator().newInstance<FenceImpl>(manager, "N/A");
}

Bool Fence::clientWait(Second seconds)
{
	return static_cast<FenceImpl*>(this)->m_fence->clientWait(seconds);
}

} // end namespace anki
