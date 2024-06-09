// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/PipelineQuery.h>
#include <AnKi/Gr/D3D/D3DQueryFactory.h>

namespace anki {

/// @addtogroup d3d
/// @{

/// Pipeline query.
class PipelineQueryImpl final : public PipelineQuery
{
public:
	QueryHandle m_handle;

	PipelineQueryImpl(CString name)
		: PipelineQuery(name)
	{
	}

	~PipelineQueryImpl();

	/// Create the query.
	Error init(PipelineQueryType type);
};
/// @}

} // end namespace anki
