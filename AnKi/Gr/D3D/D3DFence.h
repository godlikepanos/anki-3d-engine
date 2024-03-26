// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Fence.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Buffer implementation
class FenceImpl final : public Fence
{
public:
	FenceImpl(CString name)
		: Fence(name)
	{
	}

	~FenceImpl()
	{
	}
};
/// @}

} // end namespace anki
