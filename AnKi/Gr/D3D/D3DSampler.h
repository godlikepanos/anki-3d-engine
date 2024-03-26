// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Sampler.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Vulkan implementation of Sampler.
class SamplerImpl final : public Sampler
{
public:
	SamplerImpl(CString name)
		: Sampler(name)
	{
	}

	~SamplerImpl();

	Error init(const SamplerInitInfo& init);
};
/// @}

} // end namespace anki
