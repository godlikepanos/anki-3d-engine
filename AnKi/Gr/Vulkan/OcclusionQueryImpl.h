// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/OcclusionQuery.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/QueryFactory.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Occlusion query.
class OcclusionQueryImpl final : public OcclusionQuery, public VulkanObject<OcclusionQuery, OcclusionQueryImpl>
{
public:
	MicroQuery m_handle = {};

	OcclusionQueryImpl(GrManager* manager, CString name)
		: OcclusionQuery(manager, name)
	{
	}

	~OcclusionQueryImpl();

	/// Create the query.
	ANKI_USE_RESULT Error init();

	/// Get query result.
	OcclusionQueryResult getResultInternal() const;
};
/// @}

} // end namespace anki
