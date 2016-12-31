// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/QueryExtra.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Occlusion query.
class OcclusionQueryImpl : public VulkanObject
{
public:
	QueryAllocationHandle m_handle;

	OcclusionQueryImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~OcclusionQueryImpl();

	/// Create the query.
	ANKI_USE_RESULT Error init();

	/// Get query result.
	OcclusionQueryResult getResult() const;
};
/// @}

} // end namespace anki
