// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/TimestampQuery.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/QueryFactory.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Occlusion query.
class TimestampQueryImpl final : public TimestampQuery, public VulkanObject<TimestampQuery, TimestampQueryImpl>
{
public:
	MicroQuery m_handle = {};

	TimestampQueryImpl(GrManager* manager, CString name)
		: TimestampQuery(manager, name)
	{
	}

	~TimestampQueryImpl();

	/// Create the query.
	ANKI_USE_RESULT Error init();

	/// Get query result.
	TimestampQueryResult getResultInternal(Second& timestamp) const;

private:
	U64 m_timestampPeriod = 0;
};
/// @}

} // end namespace anki
