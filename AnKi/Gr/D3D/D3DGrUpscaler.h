// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrUpscaler.h>

namespace anki {

/// @addtogroup directx
/// @{

class GrUpscalerImpl final : public GrUpscaler
{
public:
	GrUpscalerImpl(CString name)
		: GrUpscaler(name)
	{
	}

	~GrUpscalerImpl();

	Error initInternal(const GrUpscalerInitInfo& initInfo);
};
/// @}

} // end namespace anki
