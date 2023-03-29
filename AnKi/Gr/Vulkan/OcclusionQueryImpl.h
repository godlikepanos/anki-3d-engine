// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/OcclusionQuery.h>
#include <AnKi/Gr/Vulkan/QueryFactory.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Occlusion query.
class OcclusionQueryImpl final : public OcclusionQuery
{
public:
	MicroQuery m_handle = {};

	OcclusionQueryImpl(CString name)
		: OcclusionQuery(name)
	{
	}

	~OcclusionQueryImpl();

	/// Create the query.
	Error init();

	/// Get query result.
	OcclusionQueryResult getResultInternal() const;
};
/// @}

} // end namespace anki
