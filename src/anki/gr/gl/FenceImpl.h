// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Fence.h>
#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Fence implementation.
class FenceImpl final : public Fence, public GlObject
{
public:
	GLsync m_fence = nullptr;
	Atomic<Bool> m_signaled = {false};

	FenceImpl(GrManager* gr)
		: Fence(gr)
	{
	}

	~FenceImpl();
};
/// @}

} // end namespace anki
