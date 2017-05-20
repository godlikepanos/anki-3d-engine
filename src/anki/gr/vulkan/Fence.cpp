// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Fence.h>
#include <anki/gr/vulkan/FenceImpl.h>

namespace anki
{

Fence::Fence(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Fence::~Fence()
{
}

Bool Fence::clientWait(F64 seconds)
{
	return m_impl->m_fence->clientWait(seconds);
}

} // end namespace anki