// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Sampler.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/SamplerFactory.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Vulkan implementation of Sampler.
class SamplerImpl final : public Sampler, public VulkanObject<Sampler, SamplerImpl>
{
public:
	MicroSamplerPtr m_sampler;

	SamplerImpl(GrManager* manager, CString name)
		: Sampler(manager, name)
	{
	}

	~SamplerImpl()
	{
	}

	ANKI_USE_RESULT Error init(const SamplerInitInfo& init);
};
/// @}

} // end namespace anki
