// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/D3D/D3DCommon.h>

namespace anki {

/// @addtogroup directx
/// @{

/// DX implementation of GrManager.
class GrManagerImpl : public GrManager
{
public:
	GrManagerImpl()
	{
	}

	~GrManagerImpl()
	{
	}

	Error init(const GrManagerInitInfo& cfg)
	{
		ANKI_ASSERT(0);
		return Error::kNone;
	}

private:
};
/// @}

} // end namespace anki
